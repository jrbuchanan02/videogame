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

#include <functional>
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
} // namespace ux::console