/**
 * @file stringfunctions.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief String functions -- text wrapping, text justification
 * @version 1
 * @date 2022-01-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <sstream>
#include <string>
#include <vector>

namespace io::console::manip
{
    /**
     * @brief Splits the string into a vector of its code-points.
     * @param std::string the string to split
     * @throw Throws std::runtime_error if there is an invalid / unknown code
     * point.
     * @return std::vector < std::string >
     */
    std::vector< std::string > splitByCodePoint ( std::string );

    std::vector< std::string > splitByCodePoint ( std::u8string str );

    /**
     * @brief Generates a vector of the parts of text which are not allowed to
     * be split across lines. Line breaks are allowed between elements. The
     * character allowing the line break is at the end of the string.
     *
     * @return std::vector< std::string >
     */
    std::vector< std::string > generateTextInseperables ( std::string );

    /**
     * @brief (Attempts to) Center text on a line of the specified size.
     * Attempts to make the parity of columns occupied by the string identical
     * to the parity of columns specified.
     * @param string the string to center
     * @param columns the amount of columns on the line
     * @return std::string
     */
    std::string centerTextOn ( std::string, std::uint32_t const );

    /**
     * @brief How many columns wide is this string?
     *
     * @return std::uint32_t
     */
    std::uint32_t columnsLong ( std::string const & );

    /**
     * @brief Takes a C-string and widens it to a UTF-32 character sequence.
     * @note Useful for querying the properties of a sequence.
     *
     * @param cstr
     * @return char32_t
     */
    inline char32_t widen ( char const *const cstr )
    {
        char32_t translated = 0;
        auto     grabFollow = [ & ] ( std::size_t num ) {
            unsigned char at = cstr [ num ];
            if ( at < 0x80 || at > 0xBF )
            {
                std::stringstream hexGetter;
                hexGetter << std::hex << std::uint32_t ( at );
                RUNTIME_ERROR (
                        "Invalid Unicode Sequence: Expected Following Byte "
                            "after hex literal ",
                        hexGetter.str ( ) )
            }
            translated <<= 6;
            translated += at & ~0x80;
        };
        if ( cstr )
        {
            unsigned char first = cstr [ 0 ];
            if ( first < 0x80 )
            {
                translated += first;
            } else if ( first < 0xC0 )
            {
                RUNTIME_ERROR (
                        "Invalid Unicode Sequence: Begins with Following Byte" )
            } else if ( first < 0xE0 )
            {
                translated += first & ~0xC0;
                grabFollow ( 1 );

                if ( translated < 0x80 )
                {
                    RUNTIME_ERROR ( "Overlong Encoding" )
                }
            } else if ( first < 0xF0 )
            {
                translated += first & ~0xE0;
                grabFollow ( 1 );
                grabFollow ( 2 );
                if ( translated < 0x800 )
                {
                    RUNTIME_ERROR ( "Overlong Encoding" )
                }
                if ( translated >= 0xD800 && translated <= 0xDFFF )
                {
                    RUNTIME_ERROR ( "Illegal Encoding" )
                }
            } else if ( translated < 0xF8 )
            {
                translated += first & ~0xF8;
                grabFollow ( 1 );
                grabFollow ( 2 );
                grabFollow ( 3 );
                if ( translated < 0xFFFF )
                {
                    RUNTIME_ERROR ( "Overlong Encoding" )
                }
                if ( translated > 0x10FFFD && translated < 0x100000 )
                {
                    RUNTIME_ERROR ( "Illegal Encoding" )
                }
                if ( translated >= 0x100000 )
                {
                    RUNTIME_ERROR ( "Character out of Bounds!" )
                }
            }
        }
        return translated;
    }
    /**
     * @brief Narrows a UTF-32 sequence into the equivalent UTF-8 sequence.
     * Always allocates 5-bytes, unless it throws (it deallocates before
     * throwing)
     *
     * @param c
     * @return char* const
     */
    inline char *const narrow ( char32_t const &c )
    {
        unsigned char *result = new unsigned char [ 5 ] { 0, 0, 0, 0, 0 };
        if ( c < 0x80 )
        {
            result [ 0 ] = ( char ) c;
        } else if ( c < 0x800 )
        {
            result [ 1 ] = 0x80;
            result [ 0 ] = 0xC0;

            result [ 1 ] += ( c >> 0x00 ) & 0b00111111;
            result [ 0 ] += ( c >> 0x06 ) & 0b00011111;
        } else if ( c >= 0xD800 && c <= 0xDFFF )
        {
            delete [] result;
            RUNTIME_ERROR ( "Illegal UTF-8 Sequence!" )
        } else if ( c < 0xFFFF )
        {
            result [ 2 ] = 0x80;
            result [ 1 ] = 0x80;
            result [ 0 ] = 0xE0;

            result [ 2 ] += ( c >> 0x00 ) & 0b00111111;
            result [ 1 ] += ( c >> 0x06 ) & 0b00111111;
            result [ 0 ] += ( c >> 0x0C ) & 0b00001111;
        } else if ( c < 0x10FFFF )
        {
            result [ 3 ] = 0x80;
            result [ 2 ] = 0x80;
            result [ 1 ] = 0x80;
            result [ 0 ] = 0xF0;

            result [ 3 ] = ( c >> 0x00 ) & 0b00111111;
            result [ 2 ] = ( c >> 0x06 ) & 0b00111111;
            result [ 1 ] = ( c >> 0x0C ) & 0b00111111;
            result [ 0 ] = ( c >> 0x12 ) & 0b00000111;
        } else
        {
            delete [] result;
            RUNTIME_ERROR ( "Out of bounds UTF-8 sequence!" );
        }
        return ( char * ) result;
    }

    inline bool validUTF08 ( char const *const &str )
    {
        try
        {
            ( void ) widen ( str );
            return true;
        } catch ( std::runtime_error &rt )
        {
            return false;
        }
    }

    inline bool validUTF32 ( char32_t const &c )
    {
        try
        {
            auto temp = narrow ( c );
            delete [] temp;
            return true;
        } catch ( std::runtime_error &rt )
        {
            return false;
        }
    }
} // namespace io::console::manip