/**
 * @file screen.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A screen.
 * @version 1
 * @date 2022-02-25
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <io/console/colors/color.h++>
#include <io/console/conmanip.h++>
#include <io/console/console.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>

#include <ux/serialization/externalized.h++>
#include <ux/serialization/strings.h++>

#include <io/base/syncstream.h++>

#include <any>
#include <array>
#include <functional>
#include <iostream>
#include <list>
#include <map>

namespace ux::console
{
    // information on a screen:
    //
    // each screen ends with a prompt and then a following screen or screens.
    // our implementation of reading these screens needs to account for the
    // fact that screens will have multiple outputs in the future.
    //
    // Each screen is made of one or more lines that can control all attributes
    // available on the console. There are also default values for each field on
    // the line.
    // Each line can currently control:
    //  - The string id that it refers to, for example, Title for
    //  en-US.Title.NOT
    //  - The SGR attributes
    //  - The rate that text scrolls on the line.
    //  - The text justification and whether to wrap text.
    //
    // Each Screen then controls:
    //  - The color palette for the screen.
    //  - What the final input is and how to interpret it. Also, any message to
    //  give the user if they enter an invalid input.
    //  - The next screen(s) and what logic to use as a switch. This logic will
    // come later, but we definitely need the ability to specify multiple
    // screens as the "next" one.

    struct Line
    {
        defines::IString textID              = "";
        std::uint64_t    txtRate             = 17;
        std::uint64_t    cmdRate             = 100;
        defines::Flag    centered        : 1 = 0;
        defines::Flag    wrapped         : 1 = 0;
        defines::Flag    bold            : 1 = 0;
        defines::Flag    faint           : 1 = 0;
        defines::Flag    italic          : 1 = 0;
        defines::Flag    underline       : 1 = 0;
        defines::Flag    slowBlink       : 1 = 0;
        defines::Flag    fastBlink       : 1 = 0;
        defines::Flag    invert          : 1 = 0;
        defines::Flag    hide            : 1 = 0;
        defines::Flag    strike          : 1 = 0;
        defines::Flag    font            : 4 = 0;
        defines::Flag    fraktur         : 1 = 0;
        defines::Flag    doubleUnderline : 1 = 0;
        // 32-bit values for the color.
        // 0-7 mean this value is a CGA color
        // 8 means this is the default color
        // 9 (in the lowest byte) means the second highest byte
        // is the 256-color color
        // 10 means that the higher 3 bytes represent the
        // rrggbb color value.
        // Any value > 10 in the lowest byte is interpreted as
        // the alpha value. As a result, it is not possible to
        // have an alpha <= 10.
        std::uint32_t    foreground          = 7;
        std::uint32_t    background          = 0;

        bool operator== ( Line const & ) const noexcept = default;
    };

    enum class InputModes
    {
        NONE,      // wait for the user to press enter.
        FULL_NAME, // get two strings delimited by a space.
        _MAX,
    };

    using InputResult = std::any;

    struct Input
    {
        InputModes mode            = InputModes::_MAX;
        bool       inputReady      = false;
        InputResult mutable result = { };

        bool const operator== ( Input const &in ) const noexcept
        {
            return mode == in.mode;
        }
    };

    struct Screen
    {
        // the lines to draw
        std::list< Line > lines;
        // the input we ask for
        Input             inputPrompt;
        // the line shown on invalid input.
        Line              wrongAnswer;

        std::map< std::size_t, std::shared_ptr< io::console::colors::IColor > >
                palette;

        // the choices for the next screen. No screen means show the exit
        // screen and exit.
        std::list< serialization::ExternalID > nextScreen;

        io::console::ConsoleManipulator
                output ( serialization::ExternalizedStrings const  &strings,
                         defines::IString const                    &locale,
                         serialization::TransliterationLevel const &level );
        bool    operator== ( Screen const &screen ) const noexcept = default;
    };
} // namespace ux::console