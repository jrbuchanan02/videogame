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

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <iostream>
#include <sstream>
#include <string>

namespace io::base
{
    template < class CharT,
               class Traits    = std::char_traits< CharT >,
               class Allocator = std::allocator< CharT > >
    inline std::basic_string< CharT, Traits, Allocator >
            emptyString ( CharT const terminator = ( CharT ) 0 )
    {
        CharT temp [ 1 ] = { terminator };
        return std::basic_string< CharT, Traits, Allocator > ( temp );
    }

    template < class CharT >
    inline std::basic_ostream< CharT > &get_cout ( ) noexcept;

    template <> inline std::ostream &get_cout< char > ( ) noexcept
    {
        return std::cout;
    }

    template <> inline std::wostream &get_cout< wchar_t > ( ) noexcept
    {
        return std::wcout;
    }

    template < class ECharT,
               class ICharT,
               class ETraits    = std::char_traits< ECharT >,
               class ITraits    = std::char_traits< ICharT >,
               class IAllocator = std::allocator< ICharT > >
    inline void printIfPossible (
            std::basic_ostream< ECharT, ETraits > &os,
            std::basic_string< ICharT, ITraits, IAllocator > const
                    &str ) requires ( sizeof ( ECharT ) != sizeof ( ICharT ) )
    { }

    template < class ECharT,
               class ICharT,
               class ETraits    = std::char_traits< ECharT >,
               class ITraits    = std::char_traits< ICharT >,
               class IAllocator = std::allocator< ICharT > >
    inline void printIfPossible (
            std::basic_ostream< ECharT, ETraits > &os,
            std::basic_string< ICharT, ITraits, IAllocator > const
                    &str ) requires ( std::is_same_v< ECharT, ICharT > )
    {
        os << str;
    }

    /**
     * @brief Prints if possible when the two character types are of the same
     * size.
     * @warning Mojibake is possible with this function. Use with caution.
     *
     * @tparam ECharT
     * @tparam ICharT
     * @tparam ETraits
     * @tparam ITraits
     * @tparam IAllocator
     */
    template < class ECharT,
               class ICharT,
               class ETraits    = std::char_traits< ECharT >,
               class ITraits    = std::char_traits< ICharT >,
               class IAllocator = std::allocator< ICharT > >
    inline void printIfPossible (
            std::basic_ostream< ECharT, ETraits > &os,
            std::basic_string< ICharT, ITraits, IAllocator > const
                    &str ) requires ( sizeof ( ICharT ) == sizeof ( ECharT )
                                      && !std::is_same_v< ECharT, ICharT > )
    {
        std::basic_string< ECharT, ETraits > temp (
                emptyString< ECharT, ETraits > ( ) );
        FOREACH ( c, str )
        {
            temp += ( ECharT ) c;
        }
        os << temp;
    }
} // namespace io::base