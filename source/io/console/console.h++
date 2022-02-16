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

        void setWaitOnText ( bool const & ) noexcept;

        std::uint32_t getCols ( ) const noexcept;
        void          setCols ( std::uint32_t const &value ) noexcept;
        std::uint32_t getRows ( ) const noexcept;
        void          setRows ( std::uint32_t const &value ) noexcept;

        std::uint64_t getTxtRate ( ) const noexcept;
        void          setTxtRate ( std::uint64_t const &value ) noexcept;
        std::uint64_t getCmdRate ( ) const noexcept;
        void          setCmdRate ( std::uint64_t const &value ) noexcept;

        std::shared_ptr< io::console::colors::IColor >
                getScreenColor ( std::uint8_t const &index );

        std::shared_ptr< io::console::colors::IColor > getCalculationColor (
                std::size_t const &,
                colors::IColor const &deflt =
                        colors::RGBAColor ( 0, 0, 0, 0 ) );

        void setScreenColor ( std::uint8_t const                      &index,
                              std::shared_ptr< colors::IColor > const &color );
        void setCalculationColor (
                std::size_t const                       &index,
                std::shared_ptr< colors::IColor > const &color );

        template < class T >
        // clang-format off
        Console &operator<< ( T const &t ) requires (
                !std::is_same_v < T, std::u8string > 
             && !std::is_same_v < T, std::u16string >
             && !std::is_same_v < T, std::u32string > 
             && !std::is_same_v < T, char8_t > 
             && !std::is_same_v < T, char16_t > 
             && !std::is_same_v < T, char32_t > 
             && !std::is_invocable_r_v< Console &, T, Console & > )
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
                // check here since a direct call to widen would appear too
                // hacky even though validUTF08 just checks if the call to widen
                // throws.
                if ( !manip::validUTF08 ( cp ) )
                {
                    RUNTIME_ERROR ( "Invalid UTF-8 Sequence!" )
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
                // no try / catch here since we would just rethrow. No call to
                // validUTF32 since we need temp and we would just throw a
                // runtime_error if the sequence weren't valid anyways
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
                std::is_invocable_r_v< Console &, T, Console & > )
        {
            return t ( *this );
        }
    };
} // namespace io::console