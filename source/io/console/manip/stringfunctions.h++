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
#include <defines/manip.h++>
#include <defines/types.h++>

#include <sstream>
#include <string>
#include <vector>

namespace io::console::manip
{
    /**
     * @brief Converts one character from one encoding to another.
     * @warning This function only operates on one code point in the argument
     * and does not remove that code point from that string.
     * @tparam L the resulting encoding
     * @tparam R the input encoding
     * @param str the value to convert
     * @return std::basic_string< L > const the converted value
     */
    template < defines::VideoCharacter L, defines::VideoCharacter R >
    std::basic_string< L > const convert ( std::basic_string< R > const str );

    // trivial conversion between strings of same or almost same type
    template < defines::VideoCharacter L, defines::VideoCharacter R >
    inline std::basic_string< L > const
            convert ( std::basic_string< R > const str ) requires (
                    sizeof ( L ) == sizeof ( R )
                    || defines::VideoUTF8< L > && defines::VideoUTF8< R > )
    {
        std::basic_string< L > res { 0 };
        for ( auto const &c : str ) { res += ( L ) c; }
        return res;
    }

    // conversions which are really convenience functions
    template <>
    defines::ChrString const convert ( defines::U32String const str );
    template <>
    defines::U32String const convert ( defines::ChrString const str );
    // definitely that convenience function
    template <>
    inline defines::U08String const convert ( defines::U32String const str )
    {
        return convert< defines::U08Char, defines::ChrChar > (
                convert< defines::ChrChar, defines::U32Char > ( str ) );
    }
    template <>
    inline defines::U32String const convert ( defines::U08String const str )
    {
        return convert< defines::U32Char, defines::ChrChar > (
                convert< defines::ChrChar, defines::U08Char > ( str ) );
    }

    /**
     *
     * @brief Splits the string into a vector of its code-points.
     * @param std::string the string to split
     * @throw Throws std::runtime_error if there is an invalid / unknown code
     * point.
     * @return std::vector < std::string >
     */
    std::vector< defines::ChrString > splitByCodePoint ( defines::ChrString );

    std::vector< defines::ChrString > splitByCodePoint ( defines::U08String );

    /**
     * @brief Generates a vector of the parts of text which are not allowed to
     * be split across lines. Line breaks are allowed between elements. The
     * character allowing the line break is at the end of the string.
     *
     * @return std::vector< std::string >
     */
    std::vector< defines::ChrString >
            generateTextInseperables ( defines::ChrString );

    /**
     * @brief (Attempts to) Center text on a line of the specified size.
     * Attempts to make the parity of columns occupied by the string identical
     * to the parity of columns specified.
     * @param string the string to center
     * @param columns the amount of columns on the line
     * @return std::string
     */
    defines::ChrString centerTextOn ( defines::ChrString, std::uint32_t const );

    /**
     * @brief How many columns wide is this string?
     *
     * @return std::uint32_t
     */
    std::uint32_t columnsLong ( defines::ChrString const & );

    /**
     * @brief Takes a C-string and widens it to a UTF-32 character sequence.
     * @note Useful for querying the properties of a sequence.
     *
     * @param cstr
     * @return char32_t
     */
    defines::U32Char widen ( defines::ChrPString const );

    /**
     * @brief Narrows a UTF-32 sequence into the equivalent UTF-8 sequence.
     * Always allocates 5-bytes, unless it throws (it deallocates before
     * throwing)
     *
     * @param c
     * @return char* const
     */
    defines::ChrPString const narrow ( defines::U32Char const &c );

    /**
     * @brief Checks if a string starts with a valid UTF-8 code point
     *
     * @param str the string to check.
     * @return true
     * @return false
     */
    bool validUTF08 ( defines::ChrPString const &str );

    /**
     * @brief Indicates whether a UTF-32 character is a valid character.
     *
     * @param c the character to check
     * @return true
     * @return false
     */
    bool validUTF32 ( defines::U32Char const &c );
} // namespace io::console::manip