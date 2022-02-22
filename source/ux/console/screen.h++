/**
 * @file screen.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A screen on the console.
 * @version 1
 * @date 2022-02-22
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
#include <io/console/conmanip.h++>
#include <io/console/console.h++>

#include <io/console/manip/stringfunctions.h++>

#include <ux/serialization/strings.h++>

#include <sstream>
#include <stdexcept>
#include <string>

#include <memory>

#include <concepts>
#include <list>

namespace ux::console
{
    template < class T >
    concept Arithmetic = std::floating_point< T > || std::integral< T >;

    class Screen
    {
    public:
        /**
         * @brief A block of colors that allows a screen to specify a unique
         * palette per line of text.
         *
         */
        class ColorBlock
        {
            struct impl_s;
            std::unique_ptr< impl_s > pimpl;
        public:
            ColorBlock ( ) noexcept;
            ColorBlock ( ColorBlock const & ) noexcept;
            ColorBlock ( ColorBlock && ) noexcept;
            ColorBlock ( std::shared_ptr< io::console::colors::IColor > const
                                 colors [ 8 ] ) noexcept;
            ~ColorBlock ( );

            ColorBlock &operator= ( ColorBlock const & ) noexcept;
            ColorBlock &operator= ( ColorBlock && ) noexcept;
            // effectively a friend insertion operator for Console << ColorBlock

            operator io::console::ConsoleManipulator ( ) const noexcept;
        };

        class Line
        {
            struct impl_s;
            std::unique_ptr< impl_s > pimpl;
        public:
            class IInputFormat
            {
            public:
                virtual bool
                        matches ( defines::ChrString const & ) noexcept = 0;
            };

            class WordInputFormat;
            class NameInputFormat;
            template < Arithmetic A > class NumberInputFormat;

            class WordInputFormat : public IInputFormat
            {
            public:
                bool matches ( defines::ChrString const &string ) noexcept
                        override final
                {
                    defines::ChrStringStream test { string };
                    defines::ChrString       temp;
                    test >> temp;
                    return !test.bad ( );
                }
            };
            class NameInputFormat : public IInputFormat
            {
            public:
                // expects a first & last name
                bool matches ( defines::ChrString const &string ) noexcept
                        override final
                {
                    defines::ChrStringStream test { string };
                    defines::ChrString       first, last;
                    test >> first >> last;
                    return !test.bad ( );
                }
            };
            template < Arithmetic A >
            class NumberInputFormat : public IInputFormat
            {
                A    min;
                A    max;
                bool applyMin = true;
                bool applyMax = true;
            public:
                constexpr inline NumberInputFormat ( ) noexcept = default;
                inline NumberInputFormat (
                        NumberInputFormat const & ) noexcept = default;
                inline NumberInputFormat ( NumberInputFormat && ) noexcept =
                        default;
                inline ~NumberInputFormat ( ) = default;

                // clang-format off
                inline NumberInputFormat &operator= ( 
                        NumberInputFormat const & ) noexcept = default;
                // clang-format on
                inline NumberInputFormat &
                        operator= ( NumberInputFormat && ) noexcept = default;
                constexpr inline NumberInputFormat (
                        A const    &min,
                        A const    &max,
                        bool const &applyMin = true,
                        bool const &applyMax = true ) noexcept :
                        min ( min ),
                        max ( max ), applyMin ( applyMin ),
                        applyMax ( applyMax )
                { }

                bool matches ( defines::ChrString const &string ) noexcept
                        override final
                {
                    defines::ChrStringStream test { string };
                    A                        temp;
                    test >> temp;
                    if ( temp < min && applyMin )
                    {
                        return false;
                    } else if ( temp > max && applyMax )
                    {
                        return false;
                    } else if ( test.bad ( ) )
                    {
                        return false;
                    } else
                    {
                        return true;
                    }
                }
            };

            Line ( ) noexcept;
            Line ( Line const & ) noexcept;
            Line ( Line && ) noexcept;
            ~Line ( );
            Line &operator= ( Line const & ) noexcept;
            Line &operator= ( Line && ) noexcept;

            /**
             * @brief Construct a Line
             *
             * @param parent the screen owning the line. If colors aren't
             * specified, the line will end up using this screen's colors.
             * @param key the key that this line uses to grab text.
             * @param colors the colors this line uses uniquely.
             * @param wrap whether to do screen wrapping.
             * @param center whether to center this line. Beware that setting
             * this flag forces wrapping, even if wrap is set to false.
             * @param tickRate how many milliseconds to wait between characters
             * @param block whether to block the current thread until the line
             * has been completely sent.
             * @param dest if this variable is not nullptr, the line will
             * wait for input matching fmt. When the line successfully gets
             * input matching that format, it will store the string it received
             * into dest.
             * @param fmt the format to match if the line is getting input.
             * @param correction the line to output if the user has input that
             * does not match the format. For example, if the line expects an
             * integer between 1 and 7, this line may say something along the
             * lines of "please input a number between one and seven"
             */
            Line ( std::shared_ptr< Screen > const           &parent,
                   defines::IString const                    &key,
                   std::shared_ptr< ColorBlock > const       &colors = nullptr,
                   bool const                                &wrap   = false,
                   bool const                                &center = false,
                   std::uint64_t const                       &tickRate = 100,
                   bool const                                &block    = false,
                   std::shared_ptr< defines::EString > const &dest = nullptr,
                   std::shared_ptr< IInputFormat > const     &fmt =
                           std::shared_ptr< WordInputFormat > (
                                   new WordInputFormat ( ) ),
                   std::shared_ptr< Line > const &correction =
                           nullptr ) noexcept;

            operator io::console::ConsoleManipulator ( ) const noexcept;
        };
    private:
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        static inline serialization::ExternalizedStrings source;

        Screen ( ) noexcept;
        Screen ( Screen const & ) noexcept;
        Screen ( Screen && ) noexcept;
        ~Screen ( );

        Screen &operator= ( Screen const & ) noexcept;
        Screen &operator= ( Screen && ) noexcept;
        /**
         * @brief insert a line at the back of the screen. Optionally indicating
         * whether this line is a correction message.
         *
         * @param line the line to insert.
         * @param correction if set to true, this line will only be drawn by
         * another line's correction message.
         */
        void    pushLine ( Line const &line,
                           bool const &correction = false ) noexcept;
        void    pullLine ( bool const &correction = false ) noexcept;

        std::list< Line >::iterator begin ( );
        std::list< Line >::iterator end ( );

        operator io::console::ConsoleManipulator ( ) const noexcept;
    };

} // namespace ux::console