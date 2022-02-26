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

    enum class SGRCommand : std::uint8_t
    {
        BOLD = 1,
        FAINT,
        ITALIC,
        UNDERLINE,
        SLOW_BLINK,
        FAST_BLINK,
        INVERT,
        HIDE,
        STRIKE,
        PRIMARY_FONT,
        ALT_FONT_1,
        ALT_FONT_2,
        ALT_FONT_3,
        ALT_FONT_4,
        ALT_FONT_5,
        ALT_FONT_6,
        ALT_FONT_7,
        ALT_FONT_8,
        ALT_FONT_9,
        FRAKTUR,
        DOUBLE_UNDERLINE,
        NORMAL_INTENSITY,
        NOT_ITALIC,
        NOT_UNDERLINED,
        NOT_BLINKING,
        PROPORTIONAL_SPACING,
        NOT_REVERSED,
        REVEAL,
        NOT_CROSSED_OUT,
        CGA_FOREGROUND_BLACK,
        CGA_FOREGROUND_RED,
        CGA_FOREGROUND_GREEN,
        CGA_FOREGROUND_YELLOW,
        CGA_FOREGROUND_BLUE,
        CGA_FOREGROUND_MAGENTA,
        CGA_FOREGROUND_CYAN,
        CGA_FOREGROUND_WHITE,
        FOREGROUND_DEFAULT = 39,
        CGA_BACKGROUND_BLACK,
        CGA_BACKGROUND_RED,
        CGA_BACKGROUND_GREEN,
        CGA_BACKGROUND_YELLOW,
        CGA_BACKGROUND_BLUE,
        CGA_BACKGROUND_MAGENTA,
        CGA_BACKGROUND_CYAN,
        CGA_BACKGROUND_WHITE,
        BACKGROUND_DEFAULT = 49,
        DISABLE_PROP_SPACING,
        FRAMED,
        ENCIRCLED,
        OVERLINED,
        NOT_FRAMED_OR_CIRCLED,
        NOT_OVERLINED,
        IDEO_UNDERLINE = 60,
        IDEO_DOUBLE_UNDERLINE,
        IDEO_OVERLINE,
        IDEO_DOUBLE_OVERLINE,
        IDEO_STRESS_MARK,
        NO_IDEOGRAM_ATTR,
        _MAX,

        CGA_FOREGROUND_0 = CGA_FOREGROUND_BLACK,
        CGA_FOREGROUND_1 = CGA_FOREGROUND_RED,
        CGA_FOREGROUND_2 = CGA_FOREGROUND_GREEN,
        CGA_FOREGROUND_3 = CGA_FOREGROUND_YELLOW,
        CGA_FOREGROUND_4 = CGA_FOREGROUND_BLUE,
        CGA_FOREGROUND_5 = CGA_FOREGROUND_MAGENTA,
        CGA_FOREGROUND_6 = CGA_FOREGROUND_CYAN,
        CGA_FOREGROUND_7 = CGA_FOREGROUND_WHITE,

        CGA_BACKGROUND_0 = CGA_BACKGROUND_BLACK,
        CGA_BACKGROUND_1 = CGA_BACKGROUND_RED,
        CGA_BACKGROUND_2 = CGA_BACKGROUND_GREEN,
        CGA_BACKGROUND_3 = CGA_BACKGROUND_YELLOW,
        CGA_BACKGROUND_4 = CGA_BACKGROUND_BLUE,
        CGA_BACKGROUND_5 = CGA_BACKGROUND_MAGENTA,
        CGA_BACKGROUND_6 = CGA_BACKGROUND_CYAN,
        CGA_BACKGROUND_7 = CGA_BACKGROUND_WHITE,

    };

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

        void setWrapping ( bool const & ) noexcept;
        void setCentering ( bool const & ) noexcept;

        void sgrCommand ( SGRCommand const &, bool const = true ) noexcept;

        // different from adjusting the palette, this color allows setting a
        // direct color for the foreground. This color is interpreted similarly
        // to how it is interpreted in a screen YAML, but this value, if it
        // indicates a direct color (256-colors, true-color), overrides the CGA
        // value.
        void setForeground ( std::uint32_t const & ) noexcept;
        void setBackground ( std::uint32_t const & ) noexcept;

        template < class T >
        // clang-format off
        Console &operator<< ( T const &t ) requires (
                !std::is_same_v < T, std::u8string > 
             && !std::is_same_v < T, std::u16string >
             && !std::is_same_v < T, std::u32string > 
             && !std::is_same_v < T, char8_t > 
             && !std::is_same_v < T, char16_t > 
             && !std::is_same_v < T, char32_t > 
             && !std::is_invocable_r_v< Console &, T, Console & > 
             && !std::is_convertible_v< T, ConsoleManipulator>)
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

        template < class T >
        Console &operator<< ( T const &t ) requires (
                // clang-format off
                std::is_convertible_v< T, ConsoleManipulator > 
                && !std::is_invocable_r_v< Console &, T, Console & > )
        // clang-format on
        {
            return *this << t.operator io::console::ConsoleManipulator ( );
        }
    };
} // namespace io::console