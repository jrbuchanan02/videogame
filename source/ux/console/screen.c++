/**
 * @file screen.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief implementations for screen.h++
 * @version 1
 * @date 2022-02-28
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <ux/console/screen.h++>

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

using namespace io::console::manip;
using namespace io::console::colors;
using namespace io::console;
using namespace ux::serialization;
using namespace ux::console;

using InputModeGetter = std::function< bool ( InputResult & ) >;
using ConsoleSGRManip =
        std::function< ConsoleManipulator ( SGRCommand const & ) >;
using ConsoleColor256 =
        std::function< ConsoleManipulator ( std::uint8_t const & ) >;
using ConsoleColor24B =
        std::function< ConsoleManipulator ( std::uint8_t const &,
                                            std::uint8_t const &,
                                            std::uint8_t const & ) >;

InputModeGetter parseInputMode ( InputModes const & );

void parseColor ( Console &,
                  std::uint32_t const &,
                  ConsoleSGRManip const &,
                  ConsoleColor256 const &,
                  ConsoleColor24B const &,
                  std::uint8_t const & );

ConsoleManipulator
        ux::console::Screen::output ( ExternalizedStrings const  &strings,
                                      defines::IString const     &locale,
                                      TransliterationLevel const &level )
{
    return [ =, *this ] ( Console &console ) -> Console & {
        auto getID = [ & ] ( defines::IString id ) -> ExternalID {
            ExternalID       result = { CHR_STRINGIZE ( ) };
            defines::IString temp   = CHR_STRINGIZE ( );
            temp       = defines::rtToString< TransliterationLevel > ( level );
            result.key = locale + "." + id + "." + temp;
            return result;
        };

        auto outputLine = [ & ] ( Line const &line ) {
            console << resetSGR;
            console << textDelay ( line.txtRate );
            console << commandDelay ( line.cmdRate );
            if ( line.centered )
            {
                console << doTextCenter;
            } else
            {
                console << noTextCenter;
                if ( line.wrapped )
                {
                    console << doTextWrapping;
                } else
                {
                    console << noTextWrapping;
                }
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
                case 9: console << doSGR ( SGRCommand::ALT_FONT_9 ); break;
                case 8: console << doSGR ( SGRCommand::ALT_FONT_8 ); break;
                case 7: console << doSGR ( SGRCommand::ALT_FONT_7 ); break;
                case 6: console << doSGR ( SGRCommand::ALT_FONT_6 ); break;
                case 5: console << doSGR ( SGRCommand::ALT_FONT_5 ); break;
                case 4: console << doSGR ( SGRCommand::ALT_FONT_4 ); break;
                case 3: console << doSGR ( SGRCommand::ALT_FONT_3 ); break;
                case 2: console << doSGR ( SGRCommand::ALT_FONT_2 ); break;
                case 1: console << doSGR ( SGRCommand::ALT_FONT_1 ); break;
                case 0: console << doSGR ( SGRCommand::PRIMARY_FONT ); break;
                default: console << doSGR ( SGRCommand::PRIMARY_FONT ); break;
            }

            parseColor ( console,
                         line.foreground,
                         doSGR,
                         setForeground256,
                         setForegroundTrue,
                         0 );
            parseColor ( console,
                         line.background,
                         doSGR,
                         setBackground256,
                         setBackgroundTrue,
                         10 );

            auto string = strings.get ( std::shared_ptr< ExternalID > (
                    new ExternalID ( getID ( line.textID ) ) ) );
            assert ( !string.empty ( ) );
            // wait until we have finished outputting the line.
            console << doWaitForText;
            console << string + "\n";
        };
        // set our palette
        for ( auto &color : palette )
        {
            if ( color.first > 7 )
            {
                console.setCalculationColor ( color.first, color.second );
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
            // get the input
            matches =
                    parseInputMode ( inputPrompt.mode ) ( inputPrompt.result );

        } while ( !matches );

        return console;
    };
}

// the input modes.
bool inputModeNone ( InputResult & ) { return true; }

InputModeGetter parseInputMode ( InputModes const &mode )
{
    switch ( mode )
    {
        case InputModes::NONE: return &inputModeNone;
        default: return &inputModeNone;
    }
}

void parseColor ( Console               &console,
                  std::uint32_t const   &color,
                  ConsoleSGRManip const &cgaColor,
                  ConsoleColor256 const &vgaColor,
                  ConsoleColor24B const &bmpColor,
                  std::uint8_t const    &off )
{
    if ( ( color & 0xff ) < 8 )
    {
        std::uint8_t temp =
                ( off ? ( std::uint8_t ) SGRCommand::CGA_FOREGROUND_0
                      : ( std::uint8_t ) SGRCommand::CGA_BACKGROUND_0 );
        temp += ( color & 7 );
        console << doSGR ( ( SGRCommand ) temp );
    } else if ( ( color & 0xff ) == 8 )
    {
        // 256 color mode
        console << vgaColor ( ( color >> 8 ) & 0xff );
    } else if ( ( color & 0xff ) == 9 )
    {
        // true color mode
        std::uint8_t c [ 3 ] = { 0, 0, 0 };
        c [ 0 ]              = ( color >> 24 ) & 0xff; // red
        c [ 1 ]              = ( color >> 16 ) & 0xff; // green
        c [ 2 ]              = ( color >> 8 ) & 0xff;  // blue
        console << bmpColor ( c [ 0 ], c [ 1 ], c [ 2 ] );
    } else
    {
        // true color with alpha (alpha is not yet implemented as it woudl
        // require us to keep track of colors on the screen here.)
        // so we just do the **exact** same thing as when we color in
        // a 24-bit color
        std::uint8_t c [ 3 ] = { 0, 0, 0 };
        c [ 0 ]              = ( color >> 24 ) & 0xff; // red
        c [ 1 ]              = ( color >> 16 ) & 0xff; // green
        c [ 2 ]              = ( color >> 8 ) & 0xff;  // blue
        console << bmpColor ( c [ 0 ], c [ 1 ], c [ 2 ] );
    }
}