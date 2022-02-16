/**
 * @file console.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A console
 * @version 1
 * @date 2022-02-12
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <io/console/colors/color.h++>
#include <io/console/colors/direct.h++>
#include <io/console/colors/indirect.h++>
#include <io/console/internal/channel.h++>
#include <io/console/manip/stringfunctions.h++>

#include <functional>
#include <memory>
#include <sstream>
#include <string>

namespace io::console
{
    class Console; // forward-declare
    using ConsoleManipulator = std::function< Console &( Console & ) >;

    class Console
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;

        void send ( std::string const &str ) noexcept;
    public:
        Console ( );
        virtual ~Console ( );

        std::uint32_t getCols ( ) const noexcept;
        void          setCols ( std::uint32_t const &value ) noexcept;
        std::uint32_t getRows ( ) const noexcept;
        void          setRows ( std::uint32_t const &value ) noexcept;

        std::uint64_t getTxtRate ( ) const noexcept;
        void          setTxtRate ( std::uint64_t const &value ) noexcept;
        std::uint64_t getCmdRate ( ) const noexcept;
        void          setCmdRate ( std::uint64_t const &value ) noexcept;

        colors::IColor *&getScreenColor ( std::uint8_t const &index );

        colors::IColor *&getCalculationColor (
                std::size_t const &,
                colors::IColor const &deflt =
                        colors::RGBAColor ( 0, 0, 0, 0 ) );

        template < class T >
        // clang-format off
        Console &operator<< ( T const &t ) requires (
                !std::is_same_v < T, std::u8string > 
             && !std::is_same_v < T, std::u16string >
             && !std::is_same_v < T, std::u32string > 
             && !std::is_same_v < T, char8_t > 
             && !std::is_same_v < T, char16_t > 
             && !std::is_same_v < T, char32_t > 
             && !std::is_same_v < T, ConsoleManipulator > )
        {
            // clang-format on
            std::stringstream temp;
            temp << t;
            send ( temp.str ( ) );
            return *this;
        }

        template < class T >
        Console &operator<< ( T const &t ) requires (
                std::is_same_v< T, std::u8string > )
        {
            auto codePoints = manip::splitByCodePoint ( t );
            for ( auto &cp : codePoints )
            {
                try
                {
                    ( void ) widen ( cp.c_str ( ) );
                } catch ( std::runtime_error &rt )
                {
                    // runtime error here means invalid code point.
                    // which is an error here.
                    throw;
                }
                *this << cp;
            }
            return *this;
        }

        template < class T >
        Console &operator<< ( T const &t ) requires (
                std::is_same_v< T, char8_t > )
        {
            char8_t temp [ 2 ] = { t, 0 };
            return *this << std::u8string ( temp );
        }

        template < class T >
        Console &operator<< ( T const &t ) requires (
                std::is_same_v< T, std::u32string > )
        {
            std::string translated = u"";

            for ( auto &cp : t )
            {
                char *temp = narrow ( cp );
                translated += temp;
                // narrow allocates
                delete [] temp;
            }
            return *this << translated;
        }

        template < class T >
        Console &operator<< ( T const &t ) requires (
                std::is_same_v< T, char32_t > )
        {
            char32_t temp [ 2 ] = { t, 0 };
            return *this << std::u32string ( temp );
        }

        template < class T >
        Console &operator<< ( T const &t ) requires (
                std::is_same_v< T, ConsoleManipulator > )
        {
            return t ( *this );
        }
    };
} // namespace io::console