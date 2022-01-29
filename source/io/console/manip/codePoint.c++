/**
 * @file codePoint.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief CodePoint processing implementation for stringfunctions
 * @version 1
 * @date 2022-01-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
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
constexpr char esc [ ] = "\\u001b";
constexpr char bsl [ ] = "\\\\";
#else
constexpr char esc [ ] = "\u001b";
constexpr char bsl [ ] = "\\";
#endif

enum CodePointType
{
    TERMINAL , // ansi escape sequence or control character, [0x00 - 0x20]
    UTF1BYTE , // one byte utf-8 starts with [0x20 - 0x7f]
    UTF2BYTE , // two byte utf-8 starts with [0xC0 - 0xDF]
    UTF3BYTE , // three byte utf-8 starts with [0xE0 - 0xEf]
    UTF4BYTE , // four byte utf-8 starts with [0xF0 - 0xF8]
    UTFNBYTE , // unknown, but it's probably unicode? perhaps 5-byte?
    INVALID_ , // we know we errored out and found a character we know to be invalid
};

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
        } else return UTF1BYTE;
    }
}

bool endsVariableLengthCode ( char const &character )
{
    if ( character < bit08 || character > del )
    {
        return true;
    } else if ( character < bit20 && character > '\x0d' )
    {
        return true;
    } else
    {
        return false;
    }
}

bool endsCSI ( char const &character )
{
    return character >= '@' && character < del;
}


std::string grabCodePoint ( std::string &string )
{
    std::string result = "";
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
                // PU1 and PU2 are subject to prior agreement on meaning between
                // us and the terminal. We can't know the length here without
                // more documentation on the windows / linux terminals.
            case 'Q':
            case 'R':
                throw std::runtime_error ( "Private use Sequence!" );
                // these four commands are terminated by the string terminator 
                // (or, on xterm and windows terminal, BEL), and can contain the 
                // character in the range [\x08, \x0D] U [\x20, \x7E]
            case 'P': // DCS
            case ']': // OSC
            case '^': // PM
            case '_': // APC
                do
                {
                    if ( string.empty ( ) ) break;
                    result += string.front ( );
                    string = string.substr ( 1 );
                } while ( !result.ends_with ( std::string ( esc ) + bsl ) && !string.empty ( ) && !endsVariableLengthCode ( result.back ( ) ) );
                break;
                // SOS is different from the four above in that it only ends with
                // either SOS or ST
            case 'X': // SOS
                do
                {
                    if ( string.empty ( ) ) break;
                    result += string.front ( );
                    string = string.substr ( 1 );
                } while ( !result.ends_with ( std::string ( esc ) + "X" ) && !result.ends_with ( std::string ( esc ) + bsl ) && !string.empty ( ) );
                break;
            case '[': // CSI
                // CSI can be terminated by a byte in the range [0x40, 0x7E]
                // the sequences that use a control character in this 
                // range have undefined behavior, so we'll assume that we 
                // know what we're doing and not terminate the sequence.
                do
                {
                    if ( string.empty ( ) ) break;
                    result += string.front ( );
                    string = string.substr ( 1 );
                } while ( !endsCSI ( result.back ( ) ) && !string.empty ( ) );
                break;
            default:
                break;
            }
        }
        break;
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

std::vector < std::string > io::console::manip::splitByCodePoint ( std::string string )
{
    std::vector < std::string > result = {};
    while ( !string.empty ( ) )
    {
        result.push_back ( grabCodePoint ( string ) );
    }
    return result;
}