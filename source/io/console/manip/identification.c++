/**
 * @file codePoint.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief CodePoint processing implementation for stringfunctions
 * @version 1
 * @date 2022-01-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <io/console/manip/stringfunctions.h++>
#include <stdexcept>
#include <string>
#include <vector>
using namespace io::console::manip;

constexpr char bit80 = '\x80';
constexpr char bit40 = '\x40';
constexpr char bit20 = '\x20';
constexpr char bit10 = '\x10';
constexpr char bit08 = '\x08';

constexpr char maxControlCharacter = '\x1f';

constexpr char del = '\x7f';

#ifdef SUPER_DEBUG
constexpr char esc [] = "\\u001b";
constexpr char bsl [] = "\\\\";
#else
constexpr char esc [] = "\u001b";
constexpr char bsl [] = "\\";
#endif

enum CodePointType
{
    TERMINAL, // ansi escape sequence or control character, [0x00 - 0x20]
    UTF1BYTE, // one byte utf-8 starts with [0x20 - 0x7f]
    UTF2BYTE, // two byte utf-8 starts with [0xC0 - 0xDF]
    UTF3BYTE, // three byte utf-8 starts with [0xE0 - 0xEf]
    UTF4BYTE, // four byte utf-8 starts with [0xF0 - 0xF8]
    UTFNBYTE, // unknown, but it's probably unicode? perhaps 5-byte?
    INVALID_, // we know we errored out and found a character we know to be
              // invalid
};

/**
 * @brief Identifies what CodePointType the first character of a sequence is
 * @note This function might need to be removed? Lots of changes happened
 * recently in the IO system.
 * @param string the string
 * @return A CodePointType corresponding to the code point.
 */
CodePointType identifyFirst ( std::string const &string )
{
    auto front = string.front ( );
    if ( front & bit80 ) // is the highest bit set?
    {
        // multi-byte unicode
        if ( front & bit40 ) // highest + second highest
        {
            if ( front & bit20 ) // highest three
            {
                if ( front & bit10 ) // highest four
                {
                    if ( front & bit08 ) // highest five
                    {
                        // if the highest five are set, we don't know what
                        // the character is, but, since it's matching this
                        // unicode pattern, we'll assume it's some new
                        // version of unicode and indicate as such.
                        //
                        // this return value still indicates an error
                        return UTFNBYTE;
                    } else
                    {
                        // highest four and not fifth highest
                        return UTF4BYTE;
                    }
                } else
                {
                    // highest three and not fourth highest
                    return UTF3BYTE;
                }
            } else
            {
                // highest two and not third highest
                return UTF2BYTE;
            }
        } else
        {
            return INVALID_; // We found the following byte in a multi-byte
                             // unicode sequence. This is an error.
        }
    } else
    {
        // terminal / single byte
        if ( front <= maxControlCharacter ) // is this a control character?
        {
            return TERMINAL;
        } else
        {
            return UTF1BYTE;
        }
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
bool endsVariableLengthCode ( char const &character )
{
    // if the character is outside the range [0x09, 0x7E]
    // NOTE: check on the lower bounds here!
    if ( character < bit08 || character > del )
    {
        return true;
        // if the character is a control character and greater than 0x0D
    } else if ( character < bit20 && character > '\x0d' )
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
bool endsCSI ( char const &character )
{
    // if our character is an alphabetic character
    // or one if @, [, ], _, /, etc.
    return character >= '@' && character < del;
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
std::string grabCodePoint ( std::string &string )
{
    std::string result       = "";
    std::string errorMessage = "Unknown (UTF-8?) Sequence!";
    switch ( identifyFirst ( string ) )
    {
        case INVALID_:
            errorMessage = "Invalid Character Sequence!";
        case UTFNBYTE:
            throw std::runtime_error ( errorMessage );
        case TERMINAL:
            result += string.front ( );
            string = string.substr ( 1 );
            // check for an escape sequence.
            if ( result.starts_with ( esc ) )
            {
                result += string.front ( );
                string = string.substr ( 1 );
                // what type of sequence do we have?
                // some are only two bytes long (incl. esc)
                // others can be infinitely long.
                switch ( result.back ( ) )
                {
                        // SS2 and SS3 require one more character
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
                        throw std::runtime_error ( "Private use Sequence!" );
                        // these four commands are terminated by the string
                        // terminator (or, on xterm and windows terminal, BEL),
                        // and can contain the character in the range [\x08,
                        // \x0D] U [\x20, \x7E]
                    case 'P': // DCS
                    case ']': // OSC
                    case '^': // PM
                    case '_': // APC
                        do
                        {
                            if ( string.empty ( ) ) break;
                            result += string.front ( );
                            string = string.substr ( 1 );
                        } while (
                                !result.ends_with ( std::string ( esc ) + bsl )
                                && !string.empty ( )
                                && !endsVariableLengthCode (
                                        result.back ( ) ) );
                        break;
                        // SOS is different from the four above in that it only
                        // ends with either SOS or ST
                    case 'X': // SOS
                        do
                        {
                            if ( string.empty ( ) ) break;
                            result += string.front ( );
                            string = string.substr ( 1 );
                        } while (
                                !result.ends_with ( std::string ( esc ) + "X" )
                                && !result.ends_with ( std::string ( esc )
                                                       + bsl )
                                && !string.empty ( ) );
                        break;
                    case '[': // CSI
                        // CSI can be terminated by a byte in the range [0x40,
                        // 0x7E] the sequences that use a control character in
                        // this range have undefined behavior, so we'll assume
                        // that we know what we're doing and not terminate the
                        // sequence.
                        do
                        {
                            if ( string.empty ( ) ) break;
                            result += string.front ( );
                            string = string.substr ( 1 );
                        } while ( !endsCSI ( result.back ( ) )
                                  && !string.empty ( ) );
                        break;
                    default:
                        break;
                }
            }
            break;
            // use implicit fallthroughs to get the right amount of characters.
        case UTF4BYTE:
            result += string.front ( );
            string = string.substr ( 1 );
        case UTF3BYTE:
            result += string.front ( );
            string = string.substr ( 1 );
        case UTF2BYTE:
            result += string.front ( );
            string = string.substr ( 1 );
        case UTF1BYTE:
            result += string.front ( );
            string = string.substr ( 1 );
        default:
            break;
    }
    return result;
}

std::vector<std::string>
        io::console::manip::splitByCodePoint ( std::string string )
{
    std::vector<std::string> result = { };
    while ( !string.empty ( ) )
    {
        result.push_back ( grabCodePoint ( string ) );
    }
    return result;
}