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

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <variant>

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
    };

    enum class InputModes
    {
        NONE, // wait for the user to press enter.
        _MAX,
    };

    using InputResult = std::variant< void * >;

    struct Input
    {
        InputModes  mode       = InputModes::_MAX;
        bool        inputReady = false;
        InputResult result     = { };
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
                output ( serialization::ExternalizedStrings const &strings,
                         defines::IString                          locale,
                         serialization::TransliterationLevel       level )
        {
            using namespace io::console;
            using namespace serialization;
            return [ =, *this ] ( Console &console ) -> Console & {
                auto getID = [ & ] ( defines::IString id ) -> ExternalID {
                    ExternalID       result = { "" };
                    defines::IString temp   = "";
                    temp = defines::rtToString< TransliterationLevel > (
                            level );
                    result.key = locale + "." + id + "." + temp;
                    return result;
                };

                auto outputLine = [ & ] ( Line const &line ) {
                    console << resetSGR;
                    console << textDelay ( line.txtRate );
                    console << commandDelay ( line.cmdRate );
                    [[likely]] if ( line.centered ) { console << doTextCenter; }
                    else
                    {
                        console << noTextCenter;
                        [[likely]] if ( line.wrapped )
                        {
                            console << doTextWrapping;
                        }
                        else { console << noTextWrapping; }
                    }
                    // iterate through the possble choices for the SGR
                    // attributes
                    if ( line.bold )
                    {
                        console << doSGR ( SGRCommand::BOLD );
                    }
                    if ( line.faint )
                    {
                        console << doSGR ( SGRCommand::FAINT );
                    }
                    if ( line.italic )
                    {
                        console << doSGR ( SGRCommand::ITALIC );
                    }
                    if ( line.underline )
                    {
                        console << doSGR ( SGRCommand::UNDERLINE );
                    }
                    if ( line.slowBlink )
                    {
                        console << doSGR ( SGRCommand::SLOW_BLINK );
                    }
                    if ( line.fastBlink )
                    {
                        console << doSGR ( SGRCommand::FAST_BLINK );
                    }
                    if ( line.invert )
                    {
                        console << doSGR ( SGRCommand::INVERT );
                    }
                    if ( line.hide )
                    {
                        console << doSGR ( SGRCommand::HIDE );
                    }
                    if ( line.strike )
                    {
                        console << doSGR ( SGRCommand::STRIKE );
                    }
                    if ( line.fraktur )
                    {
                        console << doSGR ( SGRCommand::FRAKTUR );
                    }
                    if ( line.doubleUnderline )
                    {
                        console << doSGR ( SGRCommand::DOUBLE_UNDERLINE );
                    }
                    // font
                    switch ( line.font )
                    {
                        case 9:
                            console << doSGR ( SGRCommand::ALT_FONT_9 );
                            break;
                        case 8:
                            console << doSGR ( SGRCommand::ALT_FONT_8 );
                            break;
                        case 7:
                            console << doSGR ( SGRCommand::ALT_FONT_7 );
                            break;
                        case 6:
                            console << doSGR ( SGRCommand::ALT_FONT_6 );
                            break;
                        case 5:
                            console << doSGR ( SGRCommand::ALT_FONT_5 );
                            break;
                        case 4:
                            console << doSGR ( SGRCommand::ALT_FONT_4 );
                            break;
                        case 3:
                            console << doSGR ( SGRCommand::ALT_FONT_3 );
                            break;
                        case 2:
                            console << doSGR ( SGRCommand::ALT_FONT_2 );
                            break;
                        case 1:
                            console << doSGR ( SGRCommand::ALT_FONT_1 );
                            break;
                        case 0:
                            console << doSGR ( SGRCommand::PRIMARY_FONT );
                            break;
                        default:
                            console << doSGR ( SGRCommand::PRIMARY_FONT );
                            break;
                    }

                    // foreground / background
                    if ( ( line.foreground & 0xff ) < 8 )
                    {
                        switch ( line.foreground & 0x7 )
                        {
                            case 0:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_0 );
                                break;
                            case 1:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_1 );
                                break;
                            case 2:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_2 );
                                break;
                            case 3:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_3 );
                                break;
                            case 4:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_4 );
                                break;
                            case 5:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_5 );
                                break;
                            case 6:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_6 );
                                break;
                            case 7:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_7 );
                                break;
                            default:
                                console << doSGR (
                                        SGRCommand::CGA_FOREGROUND_7 );
                                break;
                        }
                    } else if ( ( line.foreground & 0xff ) == 8 )
                    {
                        console << doSGR ( SGRCommand::FOREGROUND_DEFAULT );
                    } else if ( ( line.foreground & 0xff ) == 9 )
                    {
                        console << setForeground256 ( line.foreground >> 8 );
                    } else if ( ( line.foreground & 0xff ) == 10 )
                    {
                        console << setForegroundTrue (
                                0xff & ( line.foreground >> 24 ),
                                0xff & ( line.foreground >> 16 ),
                                0xff & ( line.foreground >> 8 ) );
                    } else // currently, alpha is not implemented, so we just
                           // set the true color
                    {
                        console << setForegroundTrue (
                                0xff & ( line.foreground >> 24 ),
                                0xff & ( line.foreground >> 16 ),
                                0xff & ( line.foreground >> 8 ) );
                    }

                    if ( ( line.background & 0xff ) < 8 )
                    {
                        switch ( line.background & 0x7 )
                        {
                            case 0:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_0 );
                                break;
                            case 1:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_1 );
                                break;
                            case 2:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_2 );
                                break;
                            case 3:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_3 );
                                break;
                            case 4:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_4 );
                                break;
                            case 5:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_5 );
                                break;
                            case 6:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_6 );
                                break;
                            case 7:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_7 );
                                break;
                            default:
                                console << doSGR (
                                        SGRCommand::CGA_BACKGROUND_7 );
                                break;
                        }
                    } else if ( ( line.background & 0xff ) == 8 )
                    {
                        console << doSGR ( SGRCommand::BACKGROUND_DEFAULT );
                    } else if ( ( line.background & 0xff ) == 9 )
                    {
                        console << setBackground256 ( line.background >> 8 );
                    } else if ( ( line.background & 0xff ) == 10 )
                    {
                        console << setBackgroundTrue (
                                0xff & ( line.background >> 24 ),
                                0xff & ( line.background >> 16 ),
                                0xff & ( line.background >> 8 ) );
                    } else // currently, alpha is not implemented, so we just
                           // set the true color.
                    {
                        console << setBackgroundTrue (
                                0xff & ( line.background >> 24 ),
                                0xff & ( line.background >> 16 ),
                                0xff & ( line.background >> 8 ) );
                    }

                    auto string = strings.get ( std::shared_ptr< ExternalID > (
                            new ExternalID ( getID ( line.textID ) ) ) );
                    assert ( !string.empty ( ) );
                    console << doWaitForText;
                    console << string + "\n";
                };
                // set our palette
                for ( auto &color : palette )
                {
                    if ( color.first > 7 )
                    {
                        console.setCalculationColor ( color.first,
                                                      color.second );
                    } else
                    {
                        console.setScreenColor ( color.first, color.second );
                    }
                }

                for ( auto const &line : lines ) { outputLine ( line ); }
                // get input
                bool matches = false;
                do {
                    std::cin.clear ( );
                    defines::ChrString input = "";
                    std::getline ( std::cin, input );

                    // check against input
                    switch ( inputPrompt.mode )
                    {
                        case InputModes::NONE: matches = true; break;
                        default: matches = true; break;
                    }

                    if ( !matches )
                    {
                        outputLine ( wrongAnswer );
                    }
                } while ( !matches );

                return console;
            };
        }
    };
} // namespace ux::console