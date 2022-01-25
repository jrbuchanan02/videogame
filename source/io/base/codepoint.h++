/**
 * @file codepoint.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A special type of "character" that cannot be split into more codepoints
 * before turning into complete nonsense. Examples include ANSI Terminal sequences
 * and unicode characters.
 * @version 1
 * @date 2022-01-25
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace io::base
{
    class CodepointString;

    class Codepoint
    {
    private:
        friend class CodepointString; // needs to access constructor
        std::string bytes;
        std::uint32_t columns;

        /**
         * @brief Construct a new Codepoint object
         * @param std::string const & the string to grab the code-point from
         * @throw throws runtime_error if the string consists of more than one
         * code point.
         */
        Codepoint ( std::string const & );
    public:

        Codepoint ( ) : Codepoint ( "" ) {}
        std::uint32_t const &width ( ) const noexcept
        {
            return columns;
        }

        operator std::string const &( ) const noexcept
        {
            return bytes;
        }

        Codepoint &operator = ( std::string const &str ) noexcept
        {
            *this = Codepoint ( str );
            return *this;
        }

        template < class T >
        friend T &operator << ( T &os , Codepoint const &cp ) requires ( std::is_base_of_v < std::basic_ostream < char > , T > )
        {
            os << cp.bytes;
            return os;
        }
    };

    class CodepointString
    {
        std::vector < Codepoint > data;
    };
}