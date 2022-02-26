/**
 * @file screens.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief The screen extraction thing.s
 * @version 1
 * @date 2022-02-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <ux/serialization/screens.h++>

#include <ux/serialization/externalized.h++>
#include <ux/serialization/strings.h++>

#include <ux/console/screen.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>

#include <io/console/colors/color.h++>
#include <io/console/conmanip.h++>
#include <io/console/console.h++>

#include <yaml-cpp/yaml.h>

using namespace ux::console;
using namespace io::console::colors;
using namespace ux::serialization;

std::shared_ptr< IColor > parseSingleColor ( YAML::Node const &node,
                                             std::size_t       i );

void ux::serialization::ExternalizedScreens::_parse (
        defines::ChrString const &string )
{
    YAML::Node node    = YAML::Load ( string );
    //
    // load in the screens.
    //
    YAML::Node screens = node [ "Screens" ];
    // iterate through each screen.
    for ( auto screen = screens.begin ( ); screen != screens.end ( ); screen++ )
    {
        Screen     parsed = { };
        ExternalID tag    = screen->first.Scalar ( );

        YAML::Node items   = screen->second;
        YAML::Node palette = items [ "Palette" ];
        YAML::Node lines   = items [ "Lines" ];
        YAML::Node input   = items [ "Input" ];
        YAML::Node next    = items [ "Next" ];

        auto parseLine = [ & ] ( YAML::Node const &line ) -> Line {
            Line parse      = { };
            parse.textID    = line [ "Id" ].as< defines::ChrString > ( );
            parse.txtRate   = line [ "TextRate" ].as< std::uint64_t > ( );
            parse.cmdRate   = line [ "CommandRate" ].as< std::uint64_t > ( );
            parse.centered  = line [ "Centered" ].as< bool > ( ) ? 1 : 0;
            parse.wrapped   = line [ "Wrapped" ].as< bool > ( ) ? 1 : 0;
            parse.bold      = line [ "Bold" ].as< bool > ( ) ? 1 : 0;
            parse.faint     = line [ "Faint" ].as< bool > ( ) ? 1 : 0;
            parse.italic    = line [ "Italic" ].as< bool > ( ) ? 1 : 0;
            parse.underline = line [ "Underline" ].as< bool > ( ) ? 1 : 0;
            parse.slowBlink = line [ "SlowBlink" ].as< bool > ( ) ? 1 : 0;
            parse.fastBlink = line [ "FastBlink" ].as< bool > ( ) ? 1 : 0;
            parse.invert    = line [ "Invert" ].as< bool > ( ) ? 1 : 0;
            parse.hide      = line [ "Hide" ].as< bool > ( ) ? 1 : 0;
            parse.strike    = line [ "Strike" ].as< bool > ( ) ? 1 : 0;
            parse.font      = line [ "Font" ].as< std::uint16_t > ( ) % 10;
            parse.fraktur   = line [ "Fraktur" ].as< bool > ( ) ? 1 : 0;
            parse.doubleUnderline =
                    line [ "DoubleUnderline" ].as< bool > ( ) ? 1 : 0;
            parse.foreground = line [ "Foreground" ].as< std::uint32_t > ( );
            parse.background = line [ "Background" ].as< std::uint32_t > ( );
            if ( !parse.centered )
                RUNTIME_ERROR ( "Not centered!" )
            return parse;
        };

        for ( auto const &line : lines )
        {
            parsed.lines.push_back ( { "EmptyString" } );
            parsed.lines.back ( ) = parseLine ( line );
        }

        for ( std::size_t i = 0; i < palette.size ( ); i++ )
        {
            parsed.palette.insert_or_assign (
                    palette [ i ][ "Number" ].as< std::size_t > ( ),
                    parseSingleColor ( palette, i ) );
        }

        parsed.inputPrompt.mode = defines::fromString< InputModes > (
                input [ "Expect" ][ "Mode" ].as< defines::ChrString > ( ) );
        parsed.wrongAnswer = input [ "Remind" ]
                                   ? parseLine ( input [ "Remind" ] )
                                   : Line { "EmptyString" };
        try
        {
            for ( auto const &option : next )
            {
                parsed.nextScreen.push_back (
                        option.as< defines::IString > ( ) );
            }
        } catch ( ... )
        {
            parsed.nextScreen.clear ( );
        }
        try
        {
            getMap ( ).try_emplace (
                    std::shared_ptr< ExternalID > ( new ExternalID ( tag ) ),
                    parsed );
        } catch ( std::logic_error &err )
        {
            RUNTIME_ERROR ( "IT's here officer!" )
        }
        // std::cin.get ( );
    }
}

std::shared_ptr< IColor > parseSingleColor ( YAML::Node const &node,
                                             std::size_t       i )
{
    // parse the color
    std::shared_ptr< IColor > parsedColor = nullptr;
    if ( node [ i ][ "Direct" ].as< bool > ( ) )
    {
        // parse direct color
        RGBAColor color;
        for ( std::size_t j = 0; i < 4; i++ )
        {
            color.setBasicComponent (
                    j,
                    node [ i ][ "Base" ][ j ].as< defines::UnboundColor > ( ) );
        }
        parsedColor = std::shared_ptr< RGBAColor > ( new RGBAColor ( color ) );
    } else
    {
        // parse indirect color
        // same basic principle applies in terms of parsing base
        // component
        IndirectColor color;
        for ( std::size_t j = 0; i < 4; i++ )
        {
            color.setBasicComponent (
                    j,
                    node [ i ][ "Base" ][ j ].as< defines::UnboundColor > ( ) );
        }
        // parse the waveform function
        blend_functions::IndirectColorBlendingFunctions blending =
                blend_functions::IndirectColorBlendingFunctions::WAVEFORM;
        blending = defines::fromString<
                blend_functions::IndirectColorBlendingFunctions > (
                node [ i ][ "Function" ].as< defines::ChrString > ( ) );
        switch ( blending )
        {
            case blend_functions::IndirectColorBlendingFunctions::AVERAGE4:
                color.setBlendFunction ( blend_functions::averageAdjust );
                break;
            case blend_functions::IndirectColorBlendingFunctions::AVERAGE5:
                color.setBlendFunction ( blend_functions::fullAverage );
                break;
            case blend_functions::IndirectColorBlendingFunctions::WAVEFORM:
                color.setBlendFunction ( blend_functions::defaultBlending );
                break;
            default:
                color.setBlendFunction ( blend_functions::defaultBlending );
        }
        // parse the numbers that make up the color's parameters.
        for ( std::size_t j = 0; j < 4; j++ )
        {
            color.setParam (
                    j,
                    parseSingleColor ( node,
                                       node [ i ][ "Params" ][ j ]
                                               .as< std::size_t > ( ) ) );
        }
        parsedColor = std::shared_ptr< IndirectColor > (
                new IndirectColor ( color ) );
    }
    return parsedColor;
}
