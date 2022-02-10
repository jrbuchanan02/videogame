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
#include <test/unittester.h++>
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
              // invalid. includes characters outside the range of unicode.
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
    std::uint32_t total  = 0;
    unsigned      length = 1;
    unsigned char front  = string.front ( );
    if ( front < 0x80 )
    {
        total += front;
    } else
    {
        if ( front < 0xBF )
        {
            return INVALID_;
        } else if ( front < 0xC2 )
        {
            return INVALID_;
        } else if ( front < 0xE0 )
        {
            length = 2;
            if ( string.size ( ) < 2 )
            {
                return INVALID_;
            } else
            {
                total += front & ~0xC0;
                front = string.at ( 1 );
                if ( front > 0xBF || front < 0x80 )
                {
                    return INVALID_;
                } else
                {
                    total <<= 6;
                    total += front & ~0x80;
                }
            }
        } else if ( front < 0xF0 )
        {
            length = 3;
            if ( string.size ( ) < length )
            {
                return INVALID_;
            } else
            {
                total += front & ~0xE0;
                for ( int i = 1; i < 3; i++ )
                {
                    front = string.at ( i );
                    if ( front > 0xBF || front < 0x80 )
                    {
                        return INVALID_;
                    } else
                    {
                        total <<= 6;
                        total += front & ~0x80;
                    }
                }
            }
        } else
        {
            length = 4;
            if ( string.size ( ) < length )
            {
                return INVALID_;
            } else
            {
                total += front & ~0xF0;
                for ( int i = 1; i < 4; i++ )
                {
                    front = string.at ( i );
                    if ( front > 0xBF || front < 0x80 )
                    {
                        return INVALID_;
                    } else
                    {
                        total <<= 6;
                        total += front & ~0x80;
                    }
                }
            }
        }
    }

    switch ( length )
    {
        case 1:
            if ( total < 0x20 )
            {
                return TERMINAL;
            } else
            {
                return UTF1BYTE;
            }
        case 2:
            if ( total < 0x80 )
            {
                return INVALID_;
            } else
            {
                return UTF2BYTE;
            }
        case 3:
            if ( total < 0x800 )
            {
                return INVALID_;
            } else if ( total >= 0xD800 && total <= 0xDFFF )
            {
                return INVALID_;
            } else
            {
                return UTF3BYTE;
            }
        case 4:
            if ( total < 0x10000 )
            {
                return INVALID_;
            } else if ( total >= 0x10FFFE )
            {
                return INVALID_;
            } else
            {
                return UTF4BYTE;
            }
        default:
            return INVALID_;
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

bool testIdentification ( std::ostream &stream )
{
    stream << "Beginning identification unittest.\n";
    stream << "Ensuring that all characters can be identified properly...\n";

    stream << "One byte characters:\n";

    for ( char i = 0; i <= maxControlCharacter; i++ )
    {
        defines::ChrString test ( { i, 0 } );
        if ( identifyFirst ( test ) != TERMINAL )
        {
            stream << "Invalid result: U+" << std::hex << std::uint32_t ( i )
                   << std::dec << " was not marked as a control character!\n";
            return false;
        }
    }
    for ( char i = ' '; i < ( char ) 0x80; i++ )
    {
        defines::ChrString test ( { ( char ) i, 0 } );
        if ( identifyFirst ( test ) != UTF1BYTE )
        {
            stream << "Invalid result: U+" << std::hex << std::uint32_t ( i )
                   << std::dec
                   << " was not marked as a one-byte unicode character!\n";
            return false;
        }
    }

    stream << "Two-byte unicode characters:\n";
    for ( char i = ( char ) 0xC2; i < ( char ) 0xE0; i++ )
    {
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            defines::ChrString test ( { i, j, 0 } );
            if ( identifyFirst ( test ) != UTF2BYTE )
            {
                std::uint32_t translated = 0;
                translated += ( ( unsigned char ) i ) & ~0xC0;
                translated <<= 6;
                translated += ( ( unsigned char ) j ) & ~0x80;

                stream << "Invalid result: U+" << std::hex << translated
                       << std::dec
                       << " was not marked as a two-byte unicode character.\n";
                return false;
            }
        }
    }
    for ( char i = ( char ) 0xC0; i < ( char ) 0xC2; i++ )
    {
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            defines::ChrString test ( { i, j, 0 } );
            if ( identifyFirst ( test ) != INVALID_ )
            {
                std::uint32_t translated = 0;
                translated += ( ( unsigned char ) i ) & ~0xC0;
                translated <<= 6;
                translated += ( ( unsigned char ) j ) & ~0x80;

                stream << "Invalid result: U+" << std::hex << translated
                       << std::dec
                       << ", encoded overlong, was not marked as an invalid "
                          "character (the range "
                          "starting with bytes 0xC0 and 0xC1 are not valid "
                          "UTF-8.\n";
                return false;
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
                if ( identifyFirst ( test ) != UTF3BYTE )
                {
                    std::uint32_t translated = 0;
                    translated += ( ( unsigned char ) i ) & ~0xE0;
                    translated <<= 6;
                    translated += ( ( unsigned char ) j ) & ~0x80;
                    translated <<= 6;
                    translated += ( ( unsigned char ) k ) & ~0x80;
                    if ( translated >= 0xD800 && translated <= 0xDFFF )
                    {
                        if ( identifyFirst ( test ) != INVALID_ )
                        {
                            stream << "Invalid result: U+" << std::hex
                                   << translated << std::dec
                                   << " was not marked as invalid.\n";
                            return false;
                        } else
                        {
                            continue;
                        }
                    }
                    if ( translated < 0x800 )
                    {
                        if ( identifyFirst ( test ) != INVALID_ )
                        {
                            stream << "Invalid result: U+" << std::hex
                                   << translated << std::dec
                                   << " was written as an overlong encoding "
                                      "and treated as valid.\n";
                            return false;
                        } else
                        {
                            continue;
                        }
                    }

                    stream << "Invalid result: U+" << std::hex << translated
                           << std::dec
                           << " was not marked as a 3-byte character, but "
                              "instead type "
                           << ( int ) identifyFirst ( test ) << "\n";
                    return false;
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
            stream << "Following byte 1: 0x" << std::hex
                   << std::uint32_t ( ( unsigned char ) j) << std::dec
                   << std::endl;
            for ( char k = ( char ) 0x80; k < ( char ) 0xC0; k++ )
            {
                stream << "Following byte 2: 0x" << std::hex
                       << std::uint32_t ( ( unsigned char ) k ) << std::dec
                       << std::endl;
                for ( char m = ( char ) 0x80; m < ( char ) 0xC0; m++ )
                {
                    stream << "Following byte 3: 0x" << std::hex
                           << std::uint32_t ( ( unsigned char ) m ) << std::dec
                           << std::endl;
                    defines::ChrString test ( { i, j, k, m, 0 } );
                    if ( identifyFirst ( test ) != UTF4BYTE )
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
                            if ( identifyFirst ( test ) != INVALID_ )
                            {
                                stream << "Invalid result: U+" << std::hex
                                       << translated << std::dec
                                       << " was encoded overlong and treated "
                                          "as valid!\n";
                                return false;
                            } else
                            {
                                continue;
                            }
                        }
                        if ( translated == 0x10FFFE || translated == 0x10FFFF )
                        {
                            if ( identifyFirst ( test ) != INVALID_ )
                            {
                                stream << "Invalid result: U+" << std::hex
                                       << translated << std::dec
                                       << " (a noncharacter as of version "
                                          "14.0) was treated as valid!\n";
                                return false;
                            } else
                            {
                                continue;
                            }
                        }

                        stream << "Invalid result: U+" << std::hex << translated
                               << std::dec
                               << " was not marked as a 4-byte unicode "
                                  "character!\n";
                        return false;
                    }
                }
            }
        }
    }
    for ( char i = ( char ) 0xF5; i < ( char ) 0xF7; i++ )
    {
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            for ( char k = ( char ) 0x80; k < ( char ) 0xC0; k++ )
            {
                for ( char m = ( char ) 0x80; m < ( char ) 0xC0; m++ )
                {
                    defines::ChrString test ( { i, j, k, m, 0 } );
                    if ( identifyFirst ( test ) != INVALID_ )
                    {
                        stream << "Invalid result: 4-byte character sequence "
                                  "outside range of UTF-8 was treated as "
                                  "valid!\n";
                        return false;
                    }
                }
            }
        }
    }
    // five byte characters then six-byte characters.
    // all should be invalid
    for ( char i = ( char ) 0xF8; i < ( char ) 0xFE; i++ )
    {
        for ( char j = ( char ) 0x80; j < ( char ) 0xC0; j++ )
        {
            for ( char k = ( char ) 0x80; k < ( char ) 0xC0; k++ )
            {
                for ( char m = ( char ) 0x80; m < ( char ) 0xC0; m++ )
                {
                    for ( char n = ( char ) 0x80; n < ( char ) 0xC0; n++ )
                    {
                        for ( char p = ( char ) 0x80; p < ( char ) 0xC0; p++ )
                        {
                            defines::ChrString test ( { i, j, k, m, n, p, 0 } );
                            if ( identifyFirst ( test ) != INVALID_
                                 && identifyFirst ( test ) != UTFNBYTE )
                            {
                                stream << "Invalid result: way out of bounds "
                                          "sequence";
                                stream << std::hex;
                                for ( auto &c : test )
                                {
                                    stream << " " << std::uint32_t ( c );
                                }
                                stream << std::dec
                                       << ", was interpreted as a valid "
                                          "sequence!\n";
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

test::Unittest identification ( testIdentification );