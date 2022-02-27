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

    template < class T >
    concept VideoCharacter = requires ( )
    {
        // clang-format off
        requires std::is_same_v<T, defines::ChrChar> 
              || std::is_same_v<T, defines::U08Char> 
              || std::is_same_v<T, defines::U16Char> 
              || std::is_same_v<T, defines::U32Char>;
        // clang-format on
    };

    template < class T >
    concept VideoUTF8 = requires ( )
    {
        requires VideoCharacter< T >;
        requires sizeof ( T ) == sizeof ( defines::U08Char );
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
        if ( memberName.find_last_of ( ' ' ) != defines::ChrString::npos )
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

    template < VideoEnumeration VE, VE val = VE::_MAX >
    inline VE fromString ( defines::ChrString string ) requires (
            ( std::size_t ) val == 0 )
    {
        if ( string == toString< VE, val > ( ) )
        {
            return val;
        } else
        {
            return VE::_MAX;
        }
    }

    template < VideoEnumeration VE, VE val = VE::_MAX >
    inline VE fromString ( defines::ChrString string ) requires (
            ( std::size_t ) val > 0 )
    {
        if ( string == toString< VE, val > ( ) )
        {
            return val;
        } else
        {
            return fromString< VE, ( VE ) ( std::size_t ( val ) - 1 ) > (
                    string );
        }
    }

    template < VideoEnumeration VE, VE End = ( VE ) 0 >
    constexpr void foreach ( void ( *function ) ( ) )
    {
        std::size_t i = ( std::size_t ) VE::_MAX;
        for ( ; ( VE ) i != End; i-- ) { function ( ); }
    }
} // namespace defines