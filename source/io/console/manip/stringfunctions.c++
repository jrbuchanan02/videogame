/**
 * @file stringfunctions.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief All string functions combined into a single file.
 * @version 1
 * @date 2022-02-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */

#include <io/console/manip/stringfunctions.h++>

#include <io/unicode/character.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <test/unittester.h++>

#include <io/base/syncstream.h++>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using io::unicode::BreakingProperties;
using io::unicode::CharacterProperties;
using io::unicode::characterProperties;

using namespace io::console::manip;

/**
 * @brief Type of a code point.
 *
 */
enum class CodePointType
{
    TERMINAL, // ansi escape sequence or control character, [0x00 - 0x20]
    UTF1BYTE, // one byte utf-8 starts with [0x20 - 0x7f]
    UTF2BYTE, // two byte utf-8 starts with [0xC0 - 0xDF]
    UTF3BYTE, // three byte utf-8 starts with [0xE0 - 0xEf]
    UTF4BYTE, // four byte utf-8 starts with [0xF0 - 0xF8]
    UTFNBYTE, // unknown, but it's probably unicode? perhaps 5-byte?
    INVALID_, // we know we errored out and found a character we know to be
              // invalid. includes characters outside the range of unicode.
    _MAX,     // maximum value
};

std::size_t const utf8SequenceLength ( defines::ChrPString const );

/**
 * @brief Identifies what CodePointType the first character of a sequence is
 * @note This function might need to be removed? Lots of changes happened
 * recently in the IO system.
 * @param string the string
 * @return A CodePointType corresponding to the code point.
 */
CodePointType identifyFirst ( defines::ChrString const &string )
{
    defines::U32Char total = 0;
    try
    {
        total = widen ( string.c_str ( ) );
    } catch ( std::runtime_error &notReallyAnError )
    {
        return CodePointType::INVALID_;
    }

    std::size_t length = 0;
    try
    {
        length = utf8SequenceLength ( string.c_str ( ) );
    } catch ( std::runtime_error &notReallyAnError )
    {
        return CodePointType::INVALID_;
    }

    switch ( length )
    {
        case 1:
            // characters before space are all control characters
            if ( total < defines::space )
            {
                return CodePointType::TERMINAL;
            } else
            { // if it's not a control character...
                return CodePointType::UTF1BYTE;
            }
        case 2:
            if ( total <= ( unsigned char ) defines::maximumASCII )
            {
                // overlong encodings (when it can be made shorter) are
                // officially an error condition
                return CodePointType::INVALID_;
            } else
            {
                // valid UTF-8 sequence (two bytes)
                return CodePointType::UTF2BYTE;
            }
        case 3:
            if ( total < defines::maximumTwoByteEncoded )
            {
                // overlong encoding (it can be made shorter). Which makes this
                // sequence illegal.
                return CodePointType::INVALID_;
            } else if ( total >= defines::ucs2Deadzone [ 0 ]
                        && total <= defines::ucs2Deadzone [ 1 ] )
            {
                // invalid UTF-8 encoding because we cannot express it in
                // UTF-16.
                return CodePointType::INVALID_;
            } else
            {
                // valid UTF-8 sequence (three bytes)
                return CodePointType::UTF3BYTE;
            }
        case 4:
            if ( total < defines::maximumThreeByteEncoded )
            {
                // overlong encoding -- we can make it shorter.
                return CodePointType::INVALID_;
            } else if ( total > defines::maxUnicode )
            {
                // out of bounds. Since UTF-16 has a maximum value of
                // defines::maxUnicode, we cannot support this character.
                return CodePointType::INVALID_;
            } else
            {
                // valid 4-byte UTF-8 sequence
                return CodePointType::UTF4BYTE;
            }
            // we fell through somehow because we got a length outside the
            // range [1, 4]. This is an error condition -- so we'll describe the
            // character as invalid
        default: return CodePointType::INVALID_;
    }
}
/**
 * @brief Whether a character ends a variable-length ESC sequence.
 * @note the variable length sequences are:
 *  - SOS (Start Of String)
 *  - OSC (Operating System Command)
 *  - APC (Application Program Command)
 *  - PM ( Privacy Message).
 * However, this function should only be dealing with OSC, as most of
 * our sequences are Control Sequences or an Operating System Command to change
 * the pallette
 * @param character the character to check against
 * @return true when the character ends the sequence
 * @return false when the character does not end the sequence
 */
bool endsVariableLengthCode ( defines::ChrChar const &character )
{
    // if the character is outside the range [0x09, 0x7E]
    if ( character < 0x08 || character >= defines::maximumASCII )
    {
        return true;
        // if the character is a control character and greater than 0x0D
    } else if ( character < defines::space && character > '\x0d' )
    {
        return true;
    } else
    {
        // if the character satisfies none of the requirements, it does not end
        // the sequence
        return false;
    }
}
/**
 * @brief If the character ends specifically a CSI sequence.
 *
 * @param character the character to check against
 * @return true ends a CSI sequence
 * @return false does not end a CSI sequence
 */
bool endsCSI ( defines::ChrChar const &character )
{
    // if our character is an alphabetic character
    // or one if @, [, ], _, /, etc.
    // the characters which end CSI continue from the @ sign to the ~, which
    // is right before the last character (tilde is 0x7E, ascii ends at 0x7F)
    return character >= '@' && character < defines::maximumASCII;
}

/**
 * @brief (Destructively) Grabs the first code point of a given string
 * @note Takes at least one character. Given the nature of terminal sequences,
 * there is no upper bound on the amount of characters  taken.
 * @throw std::runtime_error if the sequence seems to be a unicode sequence, but
 * has an indication of being longer than four bytes
 * @throw std::runtime_error if the character sequence is invalid.
 * @throw std::runtime_error if the sequence is a private-use sequence
 * @param string the string to grab from
 * @return the first code point
 */
defines::ChrString grabCodePoint ( defines::ChrString &string )
{
    // our return value
    defines::ChrString result       = CHR_STRINGIZE ( );
    // anticipated error message.
    defines::ChrString errorMessage = "Unknown (UTF-8?) Sequence!";
    switch ( identifyFirst ( string ) )
    {
        case CodePointType::INVALID_:
            // change the error message to reflect the actual error and fall
            // through to the next case, where we throw the runtime error
            errorMessage = "Invalid Character Sequence!";
        case CodePointType::UTFNBYTE:
            RUNTIME_ERROR ( errorMessage,
                            ": in decimal, U+",
                            widen ( string.c_str ( ) ),
                            ", \"",
                            string.substr ( 0, 4 ),
                            "\"" );
        case CodePointType::TERMINAL:
            // process the escape sequence.
            result += string.front ( );
            string = string.substr ( 1 );
            // check for an escape sequence.
            if ( result.starts_with ( '\u001b' ) )
            {
                result += string.front ( );
                string = string.substr ( 1 );
                // what type of sequence do we have?
                // some are only two bytes long (incl. esc)
                // others can be infinitely long.
                switch ( result.back ( ) )
                {
                        // SS2 and SS3 require one more character. Exactly one
                        // more character (as the name "single shift" implies)
                    case 'N': // ss2
                    case 'O': // ss3
                        result += string.front ( );
                        string = string.substr ( 1 );
                        break;
                        // PU1 and PU2 are subject to prior agreement on meaning
                        // between us and the terminal. We can't know the length
                        // here without more documentation on the windows /
                        // linux terminals.
                    case 'Q':
                    case 'R':
                        RUNTIME_ERROR (
                                "Encountered Private Use Sequence (don't do "
                                "that!)" )
                        // these four commands are terminated by the string
                        // terminator (or, on xterm and windows terminal, BEL),
                        // and can contain the character in the range [\x08,
                        // \x0D] U [\x20, \x7E]
                    case 'P': // DCS
                    case ']': // OSC
                    case '^': // PM
                    case '_': // APC
                        do {
                            if ( string.empty ( ) )
                                break;
                            result += string.front ( );
                            string = string.substr ( 1 );
                        } while (
                                // while the result does not end with the string
                                // terminator and while the character does not
                                // end a variable length code.
                                !result.ends_with ( defines::ChrString (
                                                            { '\u001b', '\0' } )
                                                    + defines::backwardSlash )
                                && !endsVariableLengthCode (
                                        result.back ( ) ) );
                        break;
                        // SOS is different from the four above in that it only
                        // ends with either SOS or ST
                    case 'X': // SOS
                        do {
                            if ( string.empty ( ) )
                                break;
                            result += string.front ( );
                            string = string.substr ( 1 );
                        } while (
                                !result.ends_with ( defines::ChrString (
                                                            { '\u001b', '\0' } )
                                                    + "X" )
                                && !result.ends_with (
                                        defines::ChrString (
                                                { '\u001b', '\0' } )
                                        + defines::backwardSlash ) );
                        break;
                    case '[': // CSI
                        // CSI can be terminated by a byte in the range [0x40,
                        // 0x7E] the sequences that use a control character in
                        // this range have undefined behavior, so we'll assume
                        // that we know what we're doing and not terminate the
                        // sequence.
                        do {
                            if ( string.empty ( ) )
                                break;
                            result += string.front ( );
                            string = string.substr ( 1 );
                        } while ( !endsCSI ( result.back ( ) ) );
                        break;
                    default: break;
                }
            }
            break;
            // use implicit fallthroughs to get the right amount of characters.
        case CodePointType::UTF4BYTE:
            result += string.front ( );
            string = string.substr ( 1 );
        case CodePointType::UTF3BYTE:
            result += string.front ( );
            string = string.substr ( 1 );
        case CodePointType::UTF2BYTE:
            result += string.front ( );
            string = string.substr ( 1 );
        case CodePointType::UTF1BYTE:
            result += string.front ( );
            string = string.substr ( 1 );
        default: break;
    }
    return result;
}

// split text by UTF-8 code point
std::vector< defines::ChrString >
        io::console::manip::splitByCodePoint ( defines::ChrString string )
{
    // our result. Holds an empty string by default to ensure that we don't
    // accidentally attempt to make a string with nullptr.
    std::vector< defines::ChrString > result = { CHR_STRINGIZE ( ) };
    while ( !string.empty ( ) )
    {
        result.push_back ( grabCodePoint ( string ) );
    }
    return result;
}

// convenience function so that you switch between UTF-8 and char less often?
std::vector< defines::ChrString >
        io::console::manip::splitByCodePoint ( defines::U08String str )
{
    return splitByCodePoint (
            convert< defines::ChrChar, defines::U08Char > ( str ) );
}

// definitions:
// 1. Line Break: where one line ends nad the next one starts.
// 2. Line Break Opportunity: where we are allowed to break lines. This is where
// we break our string.
// 3. Mandatory Break: we have to break.
// 4. Direct Break: we are allowed to break between characters of these classes
// 5. Indirect Break: line break opportunity exists between these characters if
// and only if they are separated by any whitespace.
// 6. Prohibited Break: no line break opportunity exists, even if there is
// whitespace.
// 7. Hyphenation: uses language specific rules.

BreakingProperties getBreakingPropertiesFrom ( defines::U32Char const & );

// bool ruleApplies(std::string const &line, std::string const &code)
bool rule3Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule4Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule5Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule6Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule7Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule8Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule9Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule10Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule11Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule12Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule13Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule14Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule15Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule16Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule17Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule18Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule19Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule20Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule21Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule22Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule23Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule24Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule25Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule26Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule27Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule28Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule29Applies ( defines::ChrString const &, defines::ChrString const & );
bool rule30Applies ( defines::ChrString const &, defines::ChrString const & );

bool isBreakingPropertyTailorable ( defines::U32Char const & );

void removeCodePoint ( std::vector< defines::ChrString > & );

std::vector< defines::ChrString >
        io::console::manip::generateTextInseperables ( defines::ChrString str )
{
    // output lines. Contains an empty string to ensure that we never try to
    // initialize a string with nullptr.
    std::vector< defines::ChrString > output     = { CHR_STRINGIZE ( ) };
    std::vector< defines::ChrString > codePoints = splitByCodePoint ( str );
    // rule 1: never break at start of text. Add a new element
    // which simply contains the first code point.
    output.push_back ( codePoints.front ( ) );
    removeCodePoint ( codePoints );
    while ( !codePoints.empty ( ) )
    {
        // our line breaking routine
        // rule 3: always break after hard line breaks
        if ( rule3Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            // line break. Add a new element.
            output.push_back ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 4: CRLF, CR, LF, and NL are also hard line breaks
        if ( rule4Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            // line break. Add a new element
            output.push_back ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 5: do not break before hard line breaks
        if ( rule5Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 6: Do not break before spaces or zero width spacce
        if ( rule6Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 7: break before any character following a zero-width space, even
        // if one or more spaces intervene unless the zero-width space is a ZWJ
        if ( rule7Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.push_back ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 8: do not break a character in a combining sequence, treat ZWJ
        // as CM
        if ( rule8Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 10: do not break before or after WJ
        if ( rule10Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 11: Do not break after GL characters
        if ( rule11Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // /\/\/\ non-tailorable rules / tailorable rules \/\/\/
        //~~rule 11a: Do not break before GL, except after SP, BA, and HY~~
        // we don't follow rule 11a

        // rule 12: Do not break before CL, CP, EX, IS, or SY. We have tailored
        // this rule to ignore spaces. So, [ this ] allows line breaks.
        if ( rule12Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 13: do not break after OP. We have tailored this rule to break
        // if there are spaces.
        if ( rule13Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 14: do not break between QU and OP, we have tailored this rule
        // to break if there are spaces.
        if ( rule14Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 15: do not break between CL | CP and NS.
        if ( rule15Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 16: do not break within B2...B2 even with intervening spaces.
        // However, due to our own limitations, we have to eliminate this rule.
        // So. it never applies
        if ( rule16Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 17: break after spaces
        if ( rule17Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.push_back ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 18: do not break before or after quotation marks
        if ( rule18Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 19: break before and after unresolved CB
        if ( rule19Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.push_back ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 20: do not break before BA, HY, NS, or BB
        // rule 20a: do not break after HL (HY | BA)
        // rule 20b: do not break between SY and HL
        if ( rule20Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 21: do not break before IN
        if ( rule21Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 22: do not break between digits and letters
        // rule 22a: do not break between numeric prefixes and letters or
        // between ideographs and numeric postfixes
        if ( rule22Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 23: do not break between numeric prefix / postfix and letters or
        //  between letters and numeric prefix / postfix
        if ( rule23Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 24: do not break between the following
        // pairs of classes: CL PO, CP PO, CL PR, CP PR, NU PO, NO PR, PO OP, PO
        // NU, PR OP, PR NU, HY NU, IS NU, NU NU, SY NU
        if ( rule24Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 25: do not break korean syllables: JL (JL | JV | H2 | H3 ) or
        // (JV | H2) (JV | JT) or (JT | H3) JT
        if ( rule25Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 9: Any and all remaining ZWJ / CM are AL
        // rule 27: do not break between alphabetics (AL|HL)(AL|HL)
        if ( rule27Applies ( output.back ( ), codePoints.front ( ) )
             || ( rule9Applies ( output.back ( ), codePoints.front ( ) ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 26: treat an entire korean syllable as an ID character.
        // rule 28: do not break between numeric punctuation and alphabetics
        // IS(AL|HL)
        if ( rule26Applies ( output.back ( ), codePoints.front ( ) )
             || rule28Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // rule 29: do not brek between letters, numbers, or ordinary symbols
        // and opening or closing parentheses unless the break is before an
        // east asian character or after an east asian character
        // rule 29b: do not break between emoji bases and emoji modifiers.
        if ( rule29Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.back ( ).append ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
        // we don't follow rule 29a
        // rule 29a: Allow a break between regional indicator symbols if and
        // only if there are an even number of regional indicators preceding the
        // position of the break.

        // rule 30: if no other rules apply, break
        if ( rule30Applies ( output.back ( ), codePoints.front ( ) ) )
        {
            output.push_back ( codePoints.front ( ) );
            removeCodePoint ( codePoints );
            continue;
        }
    }
    // rule 2: always break at end of text
    // this rule requires no action since doing anything would just
    // add an empty string to our text.
    output.shrink_to_fit ( );
    return output;
}

// #61 can also be addressed here by replacing adding spaces with the
// appropriate cursor-movement command.
defines::ChrString io::console::manip::centerTextOn ( defines::ChrString string,
                                                      std::uint32_t columns )
{
    // get the estimate of the amount of columns in the string
    std::uint32_t  estimate   = columnsLong ( string );
    // get the amount of columns we have to play with
    std::ptrdiff_t difference = columns - estimate;
    // whether or not the columns on the console and the estimated number of
    // columns that the screen takes up have the same parity. Will come in handy
    // later since it determines whether it is currently possible for the line
    // to be fully centered.
    bool           sameParity = ( ~( columns ^ estimate ) ) & 1;
    // four cases:
    // 1. difference == 0: trivially, this is already centered.
    // 2. difference < 0: replace characters with narrower variants until
    // we can no longer do that or the difference is 0.
    // 3. difference > 0 && sameParity: add difference / 2 spaces to the
    // front and back of the string.
    // 4. difference > 0 && !sameParity: attempt to widen the string by one
    // column or narrow it by one column (if we can't widen it). Then follow
    // case 3.
    if ( !difference ) // case 1
    {
        return string;
    } else
    {
        // all cases here are easier if we have another string as utf-32
        defines::ChrString result = CHR_STRINGIZE ( );
        defines::U32String asU32  = U32_STRINGIZE ( );
        // populate the utf-32 string.
        for ( auto const &cp : splitByCodePoint ( string ) )
        {
            asU32 += widen ( cp.c_str ( ) );
        }

        // convenient lambda function to convert back to UTF-8
        auto backToUTF8 = [ & ] ( ) {
            defines::ChrString output = CHR_STRINGIZE ( );
            for ( auto &cp : asU32 ) { output += narrow ( cp ); }
            return output;
        };

        // returns true if it succeeded.
        auto narrowOneUnit = [ & ] ( ) -> bool {
            for ( std::size_t i = 0; i < asU32.size ( ) - 1; i++ )
            {
                if ( asU32.at ( i ) == defines::space )
                {
                    if ( asU32.at ( i + 1 ) == defines::space )
                    {
                        // remove the character at i+1
                        asU32.erase ( i + 1, 1 );
                        return true;
                    }
                }
            }
            for ( std::size_t i = 0; i < asU32.size ( ) - 1; i++ )
            {
                if ( asU32.at ( i ) == '-' )
                {
                    if ( asU32.at ( i + 1 ) == '-' )
                    {
                        // remove the character at i+1
                        asU32.erase ( i + 1, 1 );
                        return true;
                    }
                }
            }
            defines::U32Char diff = U'！' - '!';
            for ( char32_t cp = U'！'; cp <= U'～'; cp++ )
            {
                for ( std::size_t i = 0; i < asU32.size ( ); i++ )
                {
                    if ( asU32.at ( i ) == cp )
                    {
                        asU32.at ( i ) = ( cp - diff );
                        return true;
                    }
                }
            }
            // TODO: implement looking for Katakana halfwidth <-> fullwidth
            // (requires lookup table)
            return false;
        };

        auto widenOneUnit = [ & ] ( ) -> bool {
            for ( std::size_t i = 0; i < asU32.size ( ); i++ )
            {
                if ( asU32.at ( i ) == defines::space )
                {
                    asU32.insert ( i, 1, defines::space );
                    return true;
                }
            }
            for ( std::size_t i = 0; i < asU32.size ( ); i++ )
            {
                if ( asU32.at ( i ) == '-' )
                {
                    asU32.insert ( i, 1, '-' );
                }
            }
            char32_t diff = U'！' - '!';
            for ( char32_t cp = '!'; cp <= '~'; cp++ )
            {
                for ( std::size_t i = 0; i < asU32.size ( ); i++ )
                {
                    if ( asU32.at ( i ) == cp )
                    {
                        asU32.at ( i ) = ( cp + diff );
                        return true;
                    }
                }
            }
            // TODO: implement looking for Katakana halfwidth <-> fullwidth
            // (requires lookup table)
            return false;
        };

        if ( difference < 0 )
        {
            while ( difference < 0 )
            {
                if ( narrowOneUnit ( ) )
                {
                    difference++;
                } else
                {
                    // give up, we couldn't narrow the string
                    return backToUTF8 ( );
                }
            }
            // difference == 0
            // exactly 0 columns to play with.
            return backToUTF8 ( );
        } else // difference > 0
        {
            if ( difference == 1 )
            {
                // attempt to narrow the string by one unit.
                // add a space on the left.
                narrowOneUnit ( );
                return " " + backToUTF8 ( );
            } else if ( !sameParity )
            {
                // attempt to narrow the string by one unit. If that
                // fails, attempt to widen the string by one unit.
                // add difference / 2 spaces to the left of the string.
                if ( !narrowOneUnit ( ) )
                {
                    widenOneUnit ( );
                }
                std::string output = "";
                for ( std::ptrdiff_t i = 0; i < difference / 2; i++ )
                {
                    output += " ";
                }
                return output + backToUTF8 ( );

            } else
            {
                // add difference / 2 spaces to the left of the string.
                std::string output = "";
                for ( std::ptrdiff_t i = 0; i < difference / 2; i++ )
                {
                    output += " ";
                }
                return output + backToUTF8 ( );
            }
        }
    }
}

std::uint32_t io::console::manip::columnsLong ( std::string const &string )
{
    std::uint32_t  result = 0;
    std::u32string asU32  = U"";
    for ( auto &cp : splitByCodePoint ( string ) )
    {
        asU32 += widen ( cp.c_str ( ) );
    }
    for ( auto &cp : asU32 )
    {
        auto properties = characterProperties ( ).at ( cp );
        result += properties.control
                        ? 0
                        : ( 1 + ( properties.columns | properties.emoji ) );
    }
    return result;
}

BreakingProperties getBreakingPropertiesFrom ( char32_t const &c )
{
    CharacterProperties properties = characterProperties ( ).at ( c );
    return ( BreakingProperties ) properties.lineBreaking;
}

bool isBreakingPropertyTailorable ( char32_t const &c )
{
    switch ( getBreakingPropertiesFrom ( c ) )
    {
        case BreakingProperties::BK:
        case BreakingProperties::CR:
        case BreakingProperties::LF:
        case BreakingProperties::CM:
        case BreakingProperties::NL:
        case BreakingProperties::SG:
        case BreakingProperties::WJ:
        case BreakingProperties::ZW:
        case BreakingProperties::GL:
        case BreakingProperties::SP:
        case BreakingProperties::ZWJ: return false;
        default: return true;
    }
}

void removeCodePoint ( std::vector< std::string > &points )
{
    points.erase ( points.begin ( ) );
}

bool rule3Applies ( std::string const &line, std::string const & )
{
    // rule 3 says that if the output currently ends in a hard line break,
    // then we have to perform a line break.
    std::string last     = splitByCodePoint ( line ).back ( );
    char32_t    point    = widen ( last.c_str ( ) );
    auto        breaking = getBreakingPropertiesFrom ( point );
    return breaking == BreakingProperties::BK;
}
bool rule4Applies ( std::string const &line, std::string const &code )
{
    // rule 4 says to treat CR, LF, CRLF, and NL as line breaks
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    switch ( lineBreaking )
    {
        case BreakingProperties::CR:
            return codeBreaking != BreakingProperties::LF;
        case BreakingProperties::LF:
        case BreakingProperties::NL: return true;
        default: return false;
    }
}
bool rule5Applies ( std::string const &, std::string const &code )
{
    // rule 5 says to prohibit line breaks before hard line breaks.
    // so we just check if code contains BK, CR, LF, or NL since all
    // of those lead to a line break
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );
    switch ( codeBreaking )
    {
        case BreakingProperties::BK:
        case BreakingProperties::CR:
        case BreakingProperties::LF:
        case BreakingProperties::NL: return true;
        default: return false;
    }
}
bool rule6Applies ( std::string const &, std::string const &code )
{
    // rule six says to not break before spaces or the zero width spaces
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );
    switch ( codeBreaking )
    {
        case BreakingProperties::SP:
        case BreakingProperties::ZW: return true;
        default: return false;
    }
}
bool rule7Applies ( std::string const &line, std::string const &code )
{
    // rule 7 says to break before any character following a zero-width space,
    // but after any spaces between the character and the 0 width character.
    // So, implicitly, not after the space.
    // first, we need to check if line either ends in a ZW or there are only
    // spaces to the next ZW

    std::vector< std::string > linePoints          = splitByCodePoint ( line );
    bool                       beforePartSatisfied = true;
    bool                       foundZeroWidth      = false;
    for ( auto i = linePoints.rbegin ( ); i != linePoints.rend ( ); i++ )
    {
        // get the properties of this item
        std::string linePoint         = *i;
        char32_t    widened           = widen ( linePoint.c_str ( ) );
        auto        linePointBreaking = getBreakingPropertiesFrom ( widened );
        if ( linePointBreaking == BreakingProperties::SP )
        {
            beforePartSatisfied &= true;
        } else if ( linePointBreaking == BreakingProperties::ZW )
        {
            beforePartSatisfied &= true;
            foundZeroWidth = true;
            break;
        } else
        {
            beforePartSatisfied = false;
            break;
        }
    }
    if ( beforePartSatisfied && foundZeroWidth )
    {
        // now check if we have a character **other** than the space, which,
        // as the rule is written, implicitly does not lead to a break
        std::string lastCode     = splitByCodePoint ( code ).front ( );
        char32_t    codePoint    = widen ( lastCode.c_str ( ) );
        auto        codeBreaking = getBreakingPropertiesFrom ( codePoint );
        return codeBreaking != BreakingProperties::SP;
    }
    return false;
}
bool rule8Applies ( std::string const &line, std::string const &code )
{
    // this rule applies if we have a character X and it is followed by
    // at least one CM | ZWJ. So, we have to check all the previously added
    // characters in the line and also check if the next character to add
    // falls under this rule.

    // step 1. check for our character X. X cannot be a space, but the other
    // checks are implicit within our currently defined splitting-rules
    std::vector< std::string > linePoints = splitByCodePoint ( line );
    bool                       foundX     = false;
    for ( auto i = linePoints.rbegin ( ); i != linePoints.rend ( ); i++ )
    {
        std::string linePoint         = *i;
        char32_t    widened           = widen ( linePoint.c_str ( ) );
        auto        linePointBreaking = getBreakingPropertiesFrom ( widened );
        if ( linePointBreaking == BreakingProperties::SP )
        {
            continue;
        } else
        {
            foundX = true;
            break;
        }
    }
    if ( foundX )
    {
        // check against the last character.
        std::string lastCode     = splitByCodePoint ( code ).front ( );
        char32_t    codePoint    = widen ( lastCode.c_str ( ) );
        auto        codeBreaking = getBreakingPropertiesFrom ( codePoint );
        return codeBreaking == BreakingProperties::CM
            || codeBreaking == BreakingProperties::ZWJ;
    }
    return false;
}
bool rule9Applies ( std::string const &, std::string const &code )
{
    // this rule states that any stray combining mark should be treated as an
    // alphebetical character
    std::string lastCode     = splitByCodePoint ( code ).front ( );
    char32_t    codePoint    = widen ( lastCode.c_str ( ) );
    auto        codeBreaking = getBreakingPropertiesFrom ( codePoint );
    return codeBreaking == BreakingProperties::CM
        || codeBreaking == BreakingProperties::ZWJ;
}
bool rule10Applies ( std::string const &line, std::string const &code )
{
    // this rule bans line breaks before / after word joiner characters
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    return lineBreaking == BreakingProperties::WJ
        || codeBreaking == BreakingProperties::WJ;
}
bool rule11Applies ( std::string const &line, std::string const & )
{
    // this rule bans breaking after glue characters
    std::string lastLine     = splitByCodePoint ( line ).back ( );
    char32_t    linePoint    = widen ( lastLine.c_str ( ) );
    auto        lineBreaking = getBreakingPropertiesFrom ( linePoint );
    return lineBreaking == BreakingProperties::GL;
}
bool rule12Applies ( std::string const &, std::string const &code )
{
    // this rule prohibits breaking before close punctuation, sentence
    // terminators, etc. Characters like ], ), ;, ., ,, and 」
    std::string lastCode     = splitByCodePoint ( code ).front ( );
    char32_t    codePoint    = widen ( lastCode.c_str ( ) );
    auto        codeBreaking = getBreakingPropertiesFrom ( codePoint );
    switch ( codeBreaking )
    {
        case BreakingProperties::CL:
        case BreakingProperties::CP:
        case BreakingProperties::EX:
        case BreakingProperties::IS:
        case BreakingProperties::SY: return true;
        default: return false;
    }
}
bool rule13Applies ( std::string const &line, std::string const & )
{
    // rule 13 is like rule 12, but specifies with opening punctuation only,
    // so characters like [, (, and {.
    std::string lastLine     = splitByCodePoint ( line ).back ( );
    char32_t    linePoint    = widen ( lastLine.c_str ( ) );
    auto        lineBreaking = getBreakingPropertiesFrom ( linePoint );
    return lineBreaking == BreakingProperties::OP;
}
bool rule14Applies ( std::string const &line, std::string const &code )
{
    // rule 14 is a special case that prevents "[ and other quotation mark-
    // characters followed by an open punctuation from allowing a line-break.
    std::string lastLine     = splitByCodePoint ( line ).back ( );
    char32_t    linePoint    = widen ( lastLine.c_str ( ) );
    auto        lineBreaking = getBreakingPropertiesFrom ( linePoint );

    std::string lastCode     = splitByCodePoint ( code ).front ( );
    char32_t    codePoint    = widen ( lastCode.c_str ( ) );
    auto        codeBreaking = getBreakingPropertiesFrom ( codePoint );

    return lineBreaking == BreakingProperties::QU
        && codeBreaking == BreakingProperties::OP;
}
bool rule15Applies ( std::string const &line, std::string const &code )
{
    // rule 15 prevents a break between a close punctuation and a nonstarter.
    // even with intervening spaces.
    std::string lastLine     = splitByCodePoint ( line ).back ( );
    char32_t    linePoint    = widen ( lastLine.c_str ( ) );
    auto        lineBreaking = getBreakingPropertiesFrom ( linePoint );

    std::string lastCode     = splitByCodePoint ( code ).front ( );
    char32_t    codePoint    = widen ( lastCode.c_str ( ) );
    auto        codeBreaking = getBreakingPropertiesFrom ( codePoint );

    return lineBreaking == BreakingProperties::CP
        && ( codeBreaking == BreakingProperties::NS
             // strict breaking
             || codeBreaking == BreakingProperties::CJ );
}
bool rule16Applies ( std::string const &line, std::string const &code )
{
    // rule 16 prevents a break between two "break before and after" characters
    // even if spaces intervene.

    // unfortunately, this involves looking more than one code point in the
    // future, and we can't do that with our current setup.
    return false;
}
bool rule17Applies ( std::string const &line, std::string const & )
{
    // rule 17 states that a break should occur after a space
    std::string lastLine     = splitByCodePoint ( line ).back ( );
    char32_t    linePoint    = widen ( lastLine.c_str ( ) );
    auto        lineBreaking = getBreakingPropertiesFrom ( linePoint );
    return lineBreaking == BreakingProperties::SP;
}
bool rule18Applies ( std::string const &line, std::string const &code )
{
    // rule 18 prevents line breaks immediately before or after quotation marks.
    // this rule bans line breaks before / after word joiner characters
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    return lineBreaking == BreakingProperties::QU
        || codeBreaking == BreakingProperties::QU;
}
bool rule19Applies ( std::string const &line, std::string const &code )
{
    // rule 19 states that if we see a CB character now, we should treat it as a
    // break opportunity for both before and after
    // rule 18 prevents line breaks immediately before or after quotation marks.
    // this rule bans line breaks before / after word joiner characters
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    return lineBreaking == BreakingProperties::CB
        || codeBreaking == BreakingProperties::CB;
}
bool rule20Applies ( std::string const &line, std::string const &code )
{
    // rule 20 states that we should:
    // 1. not break before a BA character
    // 2. not break before a hyphen
    // 3. not break before a small kana character, eg, my name in
    // Japanese katakana: ジョシュア, where the characters "ョ" and "ュ" are
    // smaller than the other three.
    // 4. not break after a BB character.
    // 5. Do not break after a Hebrew Letter followed by a hyphen or break after
    // character.
    // 6. do not break between an SY and a Hebrew letter

    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );
    switch ( codeBreaking )
    {
        case BreakingProperties::BA:
        case BreakingProperties::HY:
        case BreakingProperties::NS:
        case BreakingProperties::CJ: return true;
        case BreakingProperties::HL:
            return lineBreaking == BreakingProperties::SY;
        default:
            switch ( lineBreaking )
            {
                case BreakingProperties::HY:
                case BreakingProperties::BA:
                    // check if the code point immediately before ours
                    // is a hebrew letter
                    {
                        std::vector< std::string > linePoints =
                                splitByCodePoint ( line );
                        if ( linePoints.size ( ) > 1 )
                        {
                            // if there is a previous letter
                            std::string pointOfInterest =
                                    linePoints.at ( linePoints.size ( ) - 2 );
                            char32_t widened =
                                    widen ( pointOfInterest.c_str ( ) );
                            auto props = getBreakingPropertiesFrom ( widened );
                            // we apply if the letter is a hebrew letter
                            return props == BreakingProperties::HL;
                        } else
                        {
                            // if there is no previous letter, then we do
                            // not apply
                            return false;
                        }
                    }
                    // don't break after a break before.
                case BreakingProperties::BB: return true;
                default: return false;
            }
    }
}
bool rule21Applies ( std::string const &, std::string const &code )
{
    // rule 21 states that we should not break before an infix character, such
    // as ellipses.
    std::string lastCode     = splitByCodePoint ( code ).front ( );
    char32_t    codePoint    = widen ( lastCode.c_str ( ) );
    auto        codeBreaking = getBreakingPropertiesFrom ( codePoint );
    return codeBreaking == BreakingProperties::IN;
}
bool rule22Applies ( std::string const &line, std::string const &code )
{
    // rule 22 states that we should not break between digits and letters or
    // between ideographs and numeric prefixes / postfixes. Here, ideographs
    // include emoji.
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    switch ( lineBreaking )
    {
        case BreakingProperties::AL:
        case BreakingProperties::HL:
            return codeBreaking == BreakingProperties::NU;
        case BreakingProperties::NU:
            return codeBreaking == BreakingProperties::AL
                || codeBreaking == BreakingProperties::HL;
        case BreakingProperties::PR:
            return codeBreaking == BreakingProperties::ID
                || codeBreaking == BreakingProperties::EB
                || codeBreaking == BreakingProperties::EM;
        case BreakingProperties::ID:
        case BreakingProperties::EB:
        case BreakingProperties::EM:
            return codeBreaking == BreakingProperties::PO;
        default: return false;
    }
}
bool rule23Applies ( std::string const &line, std::string const &code )
{
    // rule 23 is like rule 22's case for ideographs, except its for letters.
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );
    switch ( lineBreaking )
    {
        case BreakingProperties::PR:
        case BreakingProperties::PO:
            return codeBreaking == BreakingProperties::AL
                || codeBreaking == BreakingProperties::HL;
        case BreakingProperties::AL:
        case BreakingProperties::HL:
            return codeBreaking == BreakingProperties::PR
                || codeBreaking == BreakingProperties::PO;
        default: return false;
    }
}
bool rule24Applies ( std::string const &line, std::string const &code )
{
    // rule 24 prohibits line breaks according to this pattern where 'x' is
    // the possible location of the line break:
    // CLxPO
    // CLxPR
    // CPxPO
    // CPxPR
    // HYxNU
    // ISxNU
    // NUxNU
    // NUxPO
    // NUxPR
    // POxOP // don't laugh, that's inappropriate ^_^
    // POxNU
    // PRxOP
    // PRxNU
    // SYxNU
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );
    switch ( lineBreaking )
    {
        case BreakingProperties::CL:
        case BreakingProperties::CP:
            return codeBreaking == BreakingProperties::PO
                || codeBreaking == BreakingProperties::PR;
        case BreakingProperties::HY:
        case BreakingProperties::IS:
        case BreakingProperties::SY:
            return codeBreaking == BreakingProperties::NU;
        case BreakingProperties::NU:
            return codeBreaking == BreakingProperties::NU
                || codeBreaking == BreakingProperties::PO
                || codeBreaking == BreakingProperties::PR;
        case BreakingProperties::PO:
        case BreakingProperties::PR:
            return codeBreaking == BreakingProperties::NU
                || codeBreaking == BreakingProperties::OP;
        default: return false;
    }
}
bool rule25Applies ( std::string const &line, std::string const &code )
{
    // rule 25 prohibits breaks within a korean syllable.
    // a korean syllable is defined by:
    // 1. JL followed by one of JL, JV, H2, or H3
    // 2. JV or H2 followed by one of JV or JT
    // 3. JT or H3 followed by JT.

    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    switch ( lineBreaking )
    {
        case BreakingProperties::JL:
            return codeBreaking == BreakingProperties::JL
                || codeBreaking == BreakingProperties::JV
                || codeBreaking == BreakingProperties::H2
                || codeBreaking == BreakingProperties::H3;
        case BreakingProperties::JV:
        case BreakingProperties::H2:
            return codeBreaking == BreakingProperties::JV
                || codeBreaking == BreakingProperties::JT;
        case BreakingProperties::JT:
        case BreakingProperties::H3:
            return codeBreaking == BreakingProperties::JT;
        default: return false;
    }
}
bool rule26Applies ( std::string const &line, std::string const &code )
{
    // rule 26 states that korean syllables are ideographs.
    // So, each character in 안녕하세요 (google translated "hello") allows a
    // line break. BUT ㅇ, ㅏ, and ㄴ (which combined form 안) prevent line
    // breaks between them.
    //
    // unicode writes this as preventing a break between hangul characters iff
    // followed by a PO character or preceeded by a PR character.
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    switch ( lineBreaking )
    {
        case BreakingProperties::JL:
        case BreakingProperties::JV:
        case BreakingProperties::JT:
        case BreakingProperties::H2:
        case BreakingProperties::H3:
            return codeBreaking == BreakingProperties::PO;
        case BreakingProperties::PR:
            return codeBreaking == BreakingProperties::JL
                || codeBreaking == BreakingProperties::JV
                || codeBreaking == BreakingProperties::JT
                || codeBreaking == BreakingProperties::H2
                || codeBreaking == BreakingProperties::H3;
        default: return false;
    }
}
bool rule27Applies ( std::string const &line, std::string const &code )
{
    // rule 27 prevents line breaks between alphabetic characters.
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    if ( lineBreaking == BreakingProperties::AL )
    {
        return codeBreaking == BreakingProperties::HL
            || codeBreaking == BreakingProperties::AL;
    } else if ( lineBreaking == BreakingProperties::HL )
    {
        return codeBreaking == BreakingProperties::HL
            || codeBreaking == BreakingProperties::AL;
    } else
    {
        return false;
    }
}
bool rule28Applies ( std::string const &line, std::string const &code )
{
    // rule 28 prevents line breaks between numeric invixes (eg, ".")
    // and alphabetic characters.
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    if ( lineBreaking == BreakingProperties::IS )
    {
        return codeBreaking == BreakingProperties::AL
            || codeBreaking == BreakingProperties::HL;
    }
    return false;
}
bool rule29Applies ( std::string const &line, std::string const &code )
{
    // rule 29 states that letters, numbers, or ordinary symbols followed by
    // an opening or closing parentheses should not break unless the opening or
    // closing parenthesis is a wide character.
    std::string lastLine = splitByCodePoint ( line ).back ( );
    // the front because line << code
    std::string lastCode = splitByCodePoint ( code ).front ( );

    char32_t linePoint = widen ( lastLine.c_str ( ) );
    char32_t codePoint = widen ( lastCode.c_str ( ) );

    auto lineBreaking = getBreakingPropertiesFrom ( linePoint );
    auto codeBreaking = getBreakingPropertiesFrom ( codePoint );

    switch ( lineBreaking )
    {
        case BreakingProperties::AL:
        case BreakingProperties::HL:
        case BreakingProperties::NU:
            if ( codeBreaking == BreakingProperties::OP )
            {
                return characterProperties ( ).at ( codePoint ).columns ? false
                                                                        : true;
            } else
            {
                return false;
            }
        case BreakingProperties::CP:
            switch ( codeBreaking )
            {
                case BreakingProperties::AL:
                case BreakingProperties::HL:
                case BreakingProperties::NU:
                    return characterProperties ( ).at ( linePoint ).columns
                                 ? false
                                 : true;
                default: return false;
            }
        default: return false;
    }
}
// rule 30, the "base case" always applies since we only check for it if none
// of the other rules apply.
bool rule30Applies ( std::string const &, std::string const & ) { return true; }

/**
 * @brief Takes a C-string and widens it to a UTF-32 character sequence.
 * @note Useful for querying the properties of a sequence.
 *
 * @param cstr
 * @return char32_t
 */
defines::U32Char io::console::manip::widen ( defines::ChrPString const cstr )
{
    if ( !cstr )
    {
        RUNTIME_ERROR ( "Invalid Sequence: \"sequence\" was nullptr" )
    }
    defines::ChrString temp { cstr };

    std::uint32_t total  = 0;
    unsigned      length = 1;
    unsigned      front  = ( unsigned char ) temp.front ( );

    auto addMultiByteSequence = [ & ] ( unsigned mask ) {
        // error out if we have to short a string.
        if ( temp.size ( ) < length )
        {
            RUNTIME_ERROR ( "Invalid Sequence: string too short" )
        }
        // if not, then add the first byte
        total = front & mask;
        // add the characters.
        for ( std::size_t i = 1; i < length; i++ )
        {
            front = ( unsigned char ) temp.at ( i );
            // if the character is not a valid following byte, indicate
            // that the sequence is invalid. Otherwise, continue with
            // the parsing.
            if ( front > ( unsigned char ) defines::maximumFollowing
                 || front <= ( unsigned char ) defines::maximumASCII )
            {
                RUNTIME_ERROR ( "Invalid Sequence: missing following byte" )
            } else
            {
                front &= ~( defines::minimumTwoByte - 2 );
                total <<= 6;
                total += front;
            }
        }
    };

    if ( front <= defines::maximumASCII )
    {
        total += front;
    } else
    {
        length = utf8SequenceLength ( temp.c_str ( ) );
        switch ( length )
        {
            case 2: addMultiByteSequence ( ~defines::minimumThreeByte ); break;
            case 3: addMultiByteSequence ( ~defines::minimumFourByte ); break;
            case 4: addMultiByteSequence ( ~defines::fourByteMask ); break;
            default:
                RUNTIME_ERROR ( "Unexpected Result from utf8SequenceLength: ",
                                length )
        }
    }
    return total;
}

defines::ChrPString const
        io::console::manip::narrow ( defines::U32Char const &c )
{
    // explicitly unsigned char to prevent subtraction from trying to
    // kill us.
    unsigned char *result =
            ( unsigned char * ) new defines::ChrChar [ 5 ] { 0, 0, 0, 0, 0 };

    if ( c <= ( unsigned char ) defines::maximumASCII )
    {
        result [ 0 ] = ( defines::ChrChar ) c;
    } else if ( c < defines::maximumTwoByteEncoded )
    {
        result [ 1 ] = ( unsigned char ) defines::minimumFollowing;
        result [ 0 ] = ( unsigned char ) defines::firstTwoByte;

        result [ 1 ] +=
                ( c >> 0x00 ) & ~( unsigned char ) defines::firstTwoByte;
        result [ 0 ] +=
                ( c >> 0x06 ) & ~( unsigned char ) defines::minimumThreeByte;
    } else if ( c >= 0xD800 && c <= 0xDFFF )
    {
        delete [] result;
        RUNTIME_ERROR ( "Illegal UTF-8 Sequence!" )
    } else if ( c < defines::maximumThreeByteEncoded )
    {
        result [ 2 ] = ( unsigned char ) defines::minimumFollowing;
        result [ 1 ] = ( unsigned char ) defines::minimumFollowing;
        result [ 0 ] = ( unsigned char ) defines::minimumThreeByte;

        result [ 2 ] +=
                ( c >> 0x00 ) & ~( unsigned char ) defines::firstTwoByte;
        result [ 1 ] +=
                ( c >> 0x06 ) & ~( unsigned char ) defines::firstTwoByte;
        result [ 0 ] +=
                ( c >> 0x0C ) & ~( unsigned char ) defines::minimumFourByte;
    } else if ( c < 0x10FFFF )
    {
        result [ 3 ] = ( unsigned char ) defines::minimumFollowing;
        result [ 2 ] = ( unsigned char ) defines::minimumFollowing;
        result [ 1 ] = ( unsigned char ) defines::minimumFollowing;
        result [ 0 ] = ( unsigned char ) defines::minimumFourByte;

        result [ 3 ] +=
                ( c >> 0x00 ) & ~( unsigned char ) defines::firstTwoByte;
        result [ 2 ] +=
                ( c >> 0x06 ) & ~( unsigned char ) defines::firstTwoByte;
        result [ 1 ] +=
                ( c >> 0x0C ) & ~( unsigned char ) defines::firstTwoByte;
        result [ 0 ] +=
                ( c >> 0x12 ) & ~( unsigned char ) defines::fourByteMask;
    } else
    {
        delete [] result;
        RUNTIME_ERROR ( "Out of bounds UTF-8 sequence!" );
    }
    return ( defines::ChrPString ) result;
}

bool io::console::manip::validUTF08 ( defines::ChrPString const &str )
{
    return identifyFirst ( defines::ChrString { str } )
        != CodePointType::INVALID_;
}

bool io::console::manip::validUTF32 ( defines::U32Char const &c )
{
    // UTF-32 characters are valid if they lie outside the UTF-16 deadzone
    // and are in the bounds of unicode.
    if ( c < defines::ucs2Deadzone [ 0 ] || c > defines::ucs2Deadzone [ 1 ] )
    {
        return c <= defines::maxUnicode;
    } else
    {
        return false;
    }
}

std::size_t const utf8SequenceLength ( defines::ChrPString const string )
{
    // unlike in widen, a nullptr here is not necessarily an error, it just
    // means that the string is of 0 length.
    if ( !string )
    {
        return 0;
    }
    unsigned front = ( unsigned char ) string [ 0 ];

    if ( front <= ( unsigned char ) defines::maximumASCII )
    {
        return 1;
    } else if ( front < ( unsigned char ) defines::minimumTwoByte )
    {
        RUNTIME_ERROR (
                "Invalid Sequence: unexpected following byte or overlong "
                "encoding" )
    } else if ( front < ( unsigned char ) defines::minimumThreeByte )
    {
        return 2;
    } else if ( front < ( unsigned char ) defines::minimumFourByte )
    {
        return 3;
    } else if ( front <= ( unsigned char ) defines::maximumFirstByte )
    {
        return 4;
    } else
    {
        RUNTIME_ERROR ( "Invalid Sequence: UTF out of range" )
    }
}

template <>
std::basic_string< defines::U16Char > const io::console::manip::convert (
        std::basic_string< defines::U32Char > const str )
{
    // the conversion process is trivial for when the UTF-16 character is
    // not a surrogate. However, we might need two characters.
    defines::U16Char temp [ 2 ];
    auto             convertSingleCharacter = [ & ] ( defines::U32Char in ) {
        if ( in < defines::ucs2Deadzone [ 0 ] )
        {
            temp [ 0 ] = ( defines::U16Char ) in;
            temp [ 1 ] = 0;
        } else if ( in < defines::maxUnicode ) // requirement.
        {
            // use a surrogate character.
            // the magic number used to encode both words happens to be
            // the first UTF-8 sequence which requires four bytes.
            defines::U32Char uPrime = in - defines::maximumThreeByteEncoded;
            defines::U16Char w1, w2;
            w1 = defines::ucs2Deadzone [ 0 ];
            w2 = defines::ucs2Deadzone [ 1 ];
            w1 += 0b1111111111 & ( uPrime >> 0xA );
            w2 += 0b1111111111 & ( uPrime >> 0x0 );
            temp [ 0 ] = w1;
            temp [ 2 ] = w2;
        } else
        {
            RUNTIME_ERROR ( "Unicode character out of bounds!" )
        }
    };

    std::basic_string< defines::U16Char > accumulated { 0 };
    for ( auto const &cp : str )
    {
        convertSingleCharacter ( cp );
        accumulated += temp [ 0 ];
        if ( temp [ 1 ] )
        {
            accumulated += temp [ 1 ];
        }
    }
    return accumulated;
}

template <>
defines::ChrString const
        io::console::manip::convert ( defines::U32String const str )
{
    defines::ChrString temp = "";
    for ( auto &cp : str ) { temp += narrow ( cp ); }
    return temp;
}
template <>
defines::U32String const
        io::console::manip::convert ( defines::ChrString const str )
{
    defines::U32String temp = U"";
    for ( std::size_t i = 0; i < str.size ( );
          i += utf8SequenceLength ( &str.c_str ( ) [ i ] ) )
    {
        temp += widen ( &str.c_str ( ) [ i ] );
    }
    return temp;
}

#define INCORRECT_SEQUENCE( TRANS, EXPECT, ... )                               \
    CHAR_UNITTEST_FAIL ( stream,                                               \
                         "Incorrect Labeling",                                 \
                         "U+",                                                 \
                         std::uint32_t ( TRANS ),                              \
                         "Was not marked as a(n) ",                            \
                         EXPECT,                                               \
                         " Sequence. Its byte representation is:" __VA_OPT__ ( \
                                 ,                                             \
                                 "0x",                                         \
                                 ( unsigned char ) ) __VA_ARGS__ )             \
    END_UNIT_FAIL ( stream )

bool testIdentification ( std::ostream &stream )
{
    stream << "Beginning identification unittest.\n";
    stream << "Ensuring that all characters can be identified properly...\n";

    stream << "One byte characters:\n";

    for ( char i = 0; i <= defines::maximumControlCharacter; i++ )
    {
        defines::ChrString test ( { i, 0 } );
        if ( identifyFirst ( test ) != CodePointType::TERMINAL )
        {
            INCORRECT_SEQUENCE ( ( unsigned char ) i, "Terminal", i )
        }
    }
    for ( char i = ' '; i < ( char ) 0x80; i++ )
    {
        defines::ChrString test ( { ( char ) i, 0 } );
        if ( identifyFirst ( test ) != CodePointType::UTF1BYTE )
        {
            INCORRECT_SEQUENCE ( ( unsigned char ) i, "1-byte UTF-8", i )
        }
    }

    stream << "Two-byte unicode characters:\n";
    for ( char i = ( char ) 0xC2; i < ( char ) 0xE0; i++ )
    {
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            defines::ChrString test ( { i, j, 0 } );
            if ( identifyFirst ( test ) != CodePointType::UTF2BYTE )
            {
                std::uint32_t translated = 0;
                translated += ( ( unsigned char ) i ) & ~0xC0;
                translated <<= 6;
                translated += ( ( unsigned char ) j ) & ~0x80;
                stream << "Translated to U+" << std::hex << translated
                       << std::dec << "\n";
                stream << "CodePointType is: "
                       << defines::rtToString< CodePointType > (
                                  identifyFirst ( test ) )
                       << "\n";
                stream << "Code Point Length is: "
                       << utf8SequenceLength ( test.c_str ( ) ) << "\n";
                INCORRECT_SEQUENCE ( translated, "2-byte UTF-8", i, j )
            }
        }
    }
    for ( char i = ( char ) 0xC0; i < ( char ) 0xC2; i++ )
    {
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            defines::ChrString test ( { i, j, 0 } );
            if ( identifyFirst ( test ) != CodePointType::INVALID_ )
            {
                std::uint32_t translated = 0;
                translated += ( ( unsigned char ) i ) & ~0xC0;
                translated <<= 6;
                translated += ( ( unsigned char ) j ) & ~0x80;

                INCORRECT_SEQUENCE ( translated, "Invalid", i, j )
            }
        }
    }

    stream << "Three-byte unicode characters:\n";
    for ( char i = ( char ) 0xE0; i < ( char ) 0xF0; i++ )
    {
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            for ( char k = ( char ) 0x80; k < ( char ) 0xC0; k++ )
            {
                defines::ChrString test ( { i, j, k, 0 } );
                if ( identifyFirst ( test ) != CodePointType::UTF3BYTE )
                {
                    std::uint32_t translated = 0;
                    translated += ( ( unsigned char ) i ) & ~0xE0;
                    translated <<= 6;
                    translated += ( ( unsigned char ) j ) & ~0x80;
                    translated <<= 6;
                    translated += ( ( unsigned char ) k ) & ~0x80;
                    if ( translated >= 0xD800 && translated <= 0xDFFF )
                    {
                        if ( identifyFirst ( test ) != CodePointType::INVALID_ )
                        {
                            INCORRECT_SEQUENCE ( translated,
                                                 "Invalid",
                                                 i,
                                                 j,
                                                 k )
                        } else
                        {
                            continue;
                        }
                    }
                    if ( translated < 0x800 )
                    {
                        if ( identifyFirst ( test ) != CodePointType::INVALID_ )
                        {
                            INCORRECT_SEQUENCE ( translated,
                                                 "Invalid",
                                                 i,
                                                 j,
                                                 k )
                        } else
                        {
                            continue;
                        }
                    }

                    INCORRECT_SEQUENCE ( translated, "3-byte UTF-8", i, j, k )
                }
            }
        }
    }
    stream << "Four-byte unicode characters:\n";
    for ( char i = ( char ) 0xF0; i < ( char ) 0xF4; i++ )
    {
        stream << "Leading byte 0x" << std::hex
               << std::uint32_t ( ( unsigned char ) i ) << std::dec
               << std::endl;
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            for ( char k = ( char ) 0x80; k < ( char ) 0xC0; k++ )
            {
                for ( char m = ( char ) 0x80; m < ( char ) 0xC0; m++ )
                {
                    defines::ChrString test ( { i, j, k, m, 0 } );
                    if ( identifyFirst ( test ) != CodePointType::UTF4BYTE )
                    {
                        std::uint32_t translated = 0;
                        translated += ( ( unsigned char ) i ) & ~0xF0;
                        translated <<= 6;
                        translated += ( ( unsigned char ) j ) & ~0x80;
                        translated <<= 6;
                        translated += ( ( unsigned char ) k ) & ~0x80;
                        translated <<= 6;
                        translated += ( ( unsigned char ) m ) & ~0x80;

                        // check for overlong encodings or the last two codes.
                        if ( translated < 0x10000 )
                        {
                            if ( identifyFirst ( test )
                                 != CodePointType::INVALID_ )
                            {
                                INCORRECT_SEQUENCE ( translated,
                                                     "Invalid",
                                                     i,
                                                     j,
                                                     k,
                                                     m )
                            } else
                            {
                                continue;
                            }
                        }
                        if ( translated == 0x10FFFE || translated == 0x10FFFF )
                        {
                            if ( identifyFirst ( test )
                                 != CodePointType::INVALID_ )
                            {
                                INCORRECT_SEQUENCE ( translated,
                                                     "Invalid",
                                                     i,
                                                     j,
                                                     k,
                                                     m )
                            } else
                            {
                                continue;
                            }
                        }

                        INCORRECT_SEQUENCE ( translated,
                                             "4-byte UTF-8",
                                             i,
                                             j,
                                             k,
                                             m )
                    }
                }
            }
        }
    }
    stream << "Four byte, out of range characters:\n";
    for ( char i = ( char ) 0xF5; i < ( char ) 0xF7; i++ )
    {
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            for ( char k = ( char ) 0x80; k < ( char ) 0xC0; k++ )
            {
                for ( char m = ( char ) 0x80; m < ( char ) 0xC0; m++ )
                {
                    defines::ChrString test ( { i, j, k, m, 0 } );
                    if ( identifyFirst ( test ) != CodePointType::INVALID_ )
                    {
                        std::uint32_t translated = 0;
                        translated += ( ( unsigned char ) i ) & ~0xF0;
                        translated <<= 6;
                        translated += ( ( unsigned char ) j ) & ~0x80;
                        translated <<= 6;
                        translated += ( ( unsigned char ) k ) & ~0x80;
                        translated <<= 6;
                        translated += ( ( unsigned char ) m ) & ~0x80;
                        INCORRECT_SEQUENCE ( translated, "Invalid", i, j, k, m )
                    }
                }
            }
        }
    }

    return true;
}

test::Unittest identification ( testIdentification );