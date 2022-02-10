/**
 * @file unistring.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief File for string unification.
 * @version 1
 * @date 2022-01-28
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace io::base
{
    template <class CharT,
              class Traits    = std::char_traits<CharT>,
              class Allocator = std::allocator<CharT>>
    std::basic_string<CharT, Traits, Allocator>
            emptyString ( CharT const terminator = ( CharT ) 0 )
    {
        CharT temp [ 1 ] = { terminator };
        return std::basic_string<CharT, Traits, Allocator> ( temp );
    }

    template <class CharT>
    inline std::basic_ostream<CharT> &get_cout ( ) noexcept;

    template <> inline std::ostream &get_cout<char> ( ) noexcept
    {
        return std::cout;
    }

    template <> inline std::wostream &get_cout<wchar_t> ( ) noexcept
    {
        return std::wcout;
    }
} // namespace io::base