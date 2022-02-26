/**
 * @file splitting.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implements the line break functionality
 * @version 1
 * @date 2022-02-19
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <io/console/manip/stringfunctions.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <io/unicode/character.h++>

#include <sstream>
#include <vector>

#include <io/base/syncstream.h++>
#include <iostream>

using io::unicode::BreakingProperties;
using io::unicode::CharacterProperties;
using io::unicode::characterProperties;

using namespace io::console::manip;

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

BreakingProperties getBreakingPropertiesFrom ( char32_t const & );

// bool ruleApplies(std::string const &line, std::string const &code)
bool rule3Applies ( std::string const &, std::string const & );
bool rule4Applies ( std::string const &, std::string const & );
bool rule5Applies ( std::string const &, std::string const & );
bool rule6Applies ( std::string const &, std::string const & );
bool rule7Applies ( std::string const &, std::string const & );
bool rule8Applies ( std::string const &, std::string const & );
bool rule9Applies ( std::string const &, std::string const & );
bool rule10Applies ( std::string const &, std::string const & );
bool rule11Applies ( std::string const &, std::string const & );
bool rule12Applies ( std::string const &, std::string const & );
bool rule13Applies ( std::string const &, std::string const & );
bool rule14Applies ( std::string const &, std::string const & );
bool rule15Applies ( std::string const &, std::string const & );
bool rule16Applies ( std::string const &, std::string const & );
bool rule17Applies ( std::string const &, std::string const & );
bool rule18Applies ( std::string const &, std::string const & );
bool rule19Applies ( std::string const &, std::string const & );
bool rule20Applies ( std::string const &, std::string const & );
bool rule21Applies ( std::string const &, std::string const & );
bool rule22Applies ( std::string const &, std::string const & );
bool rule23Applies ( std::string const &, std::string const & );
bool rule24Applies ( std::string const &, std::string const & );
bool rule25Applies ( std::string const &, std::string const & );
bool rule26Applies ( std::string const &, std::string const & );
bool rule27Applies ( std::string const &, std::string const & );
bool rule28Applies ( std::string const &, std::string const & );
bool rule29Applies ( std::string const &, std::string const & );
bool rule30Applies ( std::string const &, std::string const & );

bool isBreakingPropertyTailorable ( char32_t const & );

void removeCodePoint ( std::vector< std::string > & );

std::vector< std::string >
        io::console::manip::generateTextInseperables ( std::string str )
{
    std::vector< std::string > output     = { "" };
    std::vector< std::string > codePoints = splitByCodePoint ( str );
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

std::string io::console::manip::centerTextOn ( std::string   string,
                                               std::uint32_t columns )
{
    // get the estimate of the amount of columns in the string
    std::uint32_t  estimate   = columnsLong ( string );
    // get the amount of columns we have to play with
    std::ptrdiff_t difference = columns - estimate;
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
    if ( !difference )
    {
        return string;
    } else
    {
        // all cases here are easier if we have another string as utf-32
        std::string    result = "";
        std::u32string asU32  = U"";
        for ( auto &cp : splitByCodePoint ( string ) )
        {
            asU32 += widen ( cp.c_str ( ) );
        }

        auto backToUTF8 = [ & ] ( ) {
            std::string output = "";
            for ( auto &cp : asU32 ) { output += narrow ( cp ); }
            return output;
        };

        // returns true if it succeeded.
        auto narrowOneUnit = [ & ] ( ) -> bool {
            for ( std::size_t i = 0; i < asU32.size ( ) - 1; i++ )
            {
                if ( asU32.at ( i ) == ' ' )
                {
                    if ( asU32.at ( i + 1 ) == ' ' )
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
            char32_t diff = U'！' - '!';
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
            diff = U'･' - U'・';
            for ( char32_t cp = U'･'; cp <= U'ﾟ'; cp++ )
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
            return false;
        };

        auto widenOneUnit = [ & ] ( ) -> bool {
            for ( std::size_t i = 0; i < asU32.size ( ); i++ )
            {
                if ( asU32.at ( i ) == ' ' )
                {
                    asU32.insert ( i, 1, ' ' );
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
            diff = U'･' - U'・';
            for ( char32_t cp = U'・'; cp <= U'゜'; cp++ )
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