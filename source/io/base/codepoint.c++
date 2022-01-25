/**
 * @file codepoint.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Literally just the constructor, LMAO
 * @version 1
 * @date 2022-01-25
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#include <io/base/codepoint.h++>

#include <string>

using namespace io::base;

enum ByteLength
{
    MANY = -1 ,
    BYTE = 0 ,
    PAIR = 2 ,
    TRIO = 3 ,
    FOUR = 4 ,
};
ByteLength getLength ( char const & );
char const grabByte ( std::string & );
std::string lastTwo ( std::string const & );
bool endsCSI ( char const & );
std::string const grabCodepoint ( std::string &from );

Codepoint::Codepoint ( std::string const &string )
{
    std::string temp = string;
    bytes = grabCodepoint ( temp );
    if ( !string.empty ( ) ) throw std::runtime_error ( "Invalid Codepoint: \"" + string + "\" contains multiple codepoints!" );
    columns = 1;
    if ( bytes.front() == '\u001b' )
    {
        // check for SS2, SS3, SGC, SCI
        columns = 0;
    }
}

std::string const grabCodepoint ( std::string &from )
{
    std::string result = "";
    result += grabByte ( from );
    switch ( getLength ( result.front ( ) ) )
    {
    case MANY:
        result += grabByte ( from );
        switch ( result.back ( ) )
        {
        case '[': // CSI
            while ( !endsCSI ( result.back ( ) ) && !from.empty ( ) )
            {
                result += grabByte ( from );
            }
            break;
        case ']': // OSC
        // TODO: case APC
        // TODO: case PM
            while ( !lastTwo ( result ).ends_with ( "\u001b\\" ) )
            {
                result += grabByte ( from );
            }
        default: // all other cases
            break;
        }
        break;
    case FOUR:
        result += grabByte ( from );
    case TRIO:
        result += grabByte ( from );
    case PAIR:
        result += grabByte ( from );
    case BYTE:
        break;
    default:
        throw std::runtime_error ( "Illegal result from getLength!" );
    }
    return result;
}
bool endsCSI ( char const &byte )
{
    return byte >= 0x40 && byte <= 0x7E;
}
std::string lastTwo ( std::string const &of )
{
    std::string result = "";
    result = of.substr ( of.size ( ) - 2 );
    return result;
}
char const grabByte ( std::string &from )
{
    char result = from.front ( );
    from = from.substr ( 1 );
    return result;
}
ByteLength getLength ( char const &from )
{
    if ( from == '\u001b' )
    {
        return MANY;
    } else if ( from & '\x80' )
    {
        if ( from & '\x60' )
        {
            if ( from & '\x40' )
            {
                if ( from & '\x20' )
                {
                    if ( from & '\x10' )
                    {
                        throw std::runtime_error ( "Illegal Character Combination!" );
                    } else return FOUR;
                } else return TRIO;
            } else return PAIR;
        } else throw std::runtime_error ( "Illegal Character Combination!" );
    } else
    {
        return BYTE;
    }
}