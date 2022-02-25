/**
 * @file manip.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Manipulation functions on types.
 * @version 1
 * @date 2022-02-24
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <concepts>

namespace defines
{
    template < class T >
    concept VideoEnumeration = requires ( )
    {
        requires std::is_enum_v< T >;
        T::_MAX;
    };

    template < VideoEnumeration VE, VE Val >
    inline defines::ChrString toString ( ) noexcept
    {
        defines::ChrString memberName { __PRETTY_FUNCTION__ };
        memberName.pop_back ( );
        if ( memberName.find_last_of ( ':' ) != defines::ChrString::npos )
        {
            memberName = memberName.substr ( memberName.find_last_of ( ':' ) );
        }
        // also eliminate up to the last space
        if ( memberName.find_last_of(' ') != defines::ChrString::npos )
        {
            memberName = memberName.substr ( memberName.find_last_of ( ' ' ) );
        }
        return memberName;
    }

    template < VideoEnumeration VE, VE checkVal = VE::_MAX >
    inline defines::ChrString rtToString ( VE const &ve ) noexcept
    {
        if ( checkVal == ve )
        {
            return toString< VE, checkVal > ( );
        } else
        {
            return rtToString< VE, ( VE ) ( ( std::size_t ) checkVal - 1 ) > (
                    ve );
        }
    }

    template < VideoEnumeration VE, VE val = ( VE ) 0 >
    inline VE fromString ( defines::ChrString string )
    {
        if ( string == toString< VE, val > ( ) )
        {
            return val;
        } else if ( val != VE::_MAX )
        {
            return fromString< VE, ( VE ) ( std::size_t ( val ) + 1 ) > (
                    string );
        } else
        {
            return VE::_MAX;
        }
    }

    template < VideoEnumeration VE, VE End = ( VE ) 0 >
    constexpr void foreach ( void ( *function ) ( ) )
    {
        std::size_t i = ( std::size_t ) VE::_MAX;
        for ( ; ( VE ) i != End; i-- ) { function ( ); }
    }
} // namespace defines