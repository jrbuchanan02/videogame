/**
 * @file colors.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implements reading in the colors.
 * @version 1
 * @date 2022-02-23
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <ux/console/colors.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <io/console/colors/color.h++>
#include <io/console/console.h++>

#include <io/base/syncstream.h++>

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <map>
#include <memory>

using namespace ux::console;
using namespace io::console::colors;

struct ux::console::ColorBlock::impl_s
{
    std::map< std::size_t, std::shared_ptr< IColor > > colors;
    impl_s ( ) noexcept                = default;
    ~impl_s ( ) noexcept               = default;
    impl_s ( impl_s const & ) noexcept = default;
    impl_s ( impl_s && ) noexcept      = default;

    impl_s &operator= ( impl_s const & ) noexcept = default;
    impl_s &operator= ( impl_s && ) noexcept = default;
};

ux::console::ColorBlock::ColorBlock ( ) noexcept : pimpl ( new impl_s ( ) ) { }
ux::console::ColorBlock::ColorBlock ( ColorBlock const &that ) noexcept :
        pimpl ( new impl_s ( *that.pimpl ) )
{ }
ux::console::ColorBlock::ColorBlock ( ColorBlock &&that ) noexcept :
        pimpl ( new impl_s ( *std::move ( that.pimpl ) ) )
{ }
ux::console::ColorBlock::~ColorBlock ( ) = default;

ColorBlock &
        ux::console::ColorBlock::operator= ( ColorBlock const &that ) noexcept
{
    pimpl.reset ( that.pimpl.get ( ) );
    return *this;
}
ColorBlock &ux::console::ColorBlock::operator= ( ColorBlock &&that ) noexcept
{
    pimpl.reset ( std::move ( that.pimpl ).get ( ) );
    return *this;
}

std::shared_ptr< IColor > parseColor (
        YAML::Node const &,
        std::map< std::size_t, std::shared_ptr< IColor > > const & );

ux::console::ColorBlock::ColorBlock ( YAML::Node const &object )
{
    /**
     * The expected format of a single color:
     *
     * 1. the color is omitted or the color "n" is specified as "default"
     *  - the color is not added
     * 2. the color "n" is specified as:
     * "n" : {
     *      "mode" : "DIRECT",
     *      "base" : [r, g, b, a]
     * }
     *  where r, g, b, and a are all numerical values.
     *  - the color is added as a direct color.
     * 3. the color "n" is specified as:
     * "n" : {
     *      "mode" : "INDIRECT",
     *      "base" : [r, g, b, a],
     *      "args" : [a, b, c, d],
     *      "func" : "FUNCTION"
     * }
     *  where r, g, b, a and a, b, c, d, are all numerical values.
     *  where FUNCTION is one of WAVEFORM, AVERAGE4, AVERAGE5 or the "func"
     * field is omitted.
     *  - the color is added as an indirect color taking these arguments
     */

    // the colors can appear in either a "calculation" object or a
    // "screen" object.

    auto iterate = [ & ] ( defines::EPString place ) {
        // iterate through the colors.
        for ( auto const &item : object [ place ] )
        {
            // grab the keys.
            defines::EStringStream temp { item.Tag ( ) };

            // don't overwrite black with a poorly initialized color.
            std::size_t index = 8;
            temp >> index;

            pimpl->colors.insert_or_assign (
                    index,
                    parseColor ( item, pimpl->colors ) );
        }
    };

    if ( object [ "Calculation" ] )
    {
        iterate ( "Calculation" );
    }
    if ( object [ "Screen" ] )
    {
        iterate ( "Screen" );
    }
}

void ux::console::ColorBlock::addColor (
        std::size_t const               &index,
        std::shared_ptr< IColor > const &color ) noexcept
{
    pimpl->colors.insert_or_assign ( index, color );
}

void ux::console::ColorBlock::delColor ( std::size_t const &index ) noexcept
{
    if ( pimpl->colors.contains ( index ) )
    {
        pimpl->colors.at ( index ) = nullptr;
    }
}

std::shared_ptr< IColor > const
        ux::console::ColorBlock::getColor ( std::size_t const &index ) noexcept
{
    if ( pimpl->colors.contains ( index ) )
    {
        return pimpl->colors.at ( index );
    } else
    {
        return nullptr;
    }
}

void ux::console::ColorBlock::setColor (
        std::size_t const               &index,
        std::shared_ptr< IColor > const &color ) noexcept
{
    addColor ( index, color );
}

bool ux::console::ColorBlock::hasColor ( std::size_t const &index ) noexcept
{
    return pimpl->colors.contains ( index );
}

ux::console::ColorBlock::operator io::console::ConsoleManipulator ( )
        const noexcept
{
    using io::console::Console;
    return [ & ] ( Console &console ) -> Console & {
        for ( auto &pair : pimpl->colors )
        {
            if ( pair.first > 7 )
            {
                console.setCalculationColor ( pair.first, pair.second );
            } else
            {
                console.setScreenColor ( pair.first, pair.second );
            }
        }
        return console;
    };
}

std::shared_ptr< IColor > parseColor (
        YAML::Node const                                         &color,
        std::map< std::size_t, std::shared_ptr< IColor > > const &colors )
{
    // at this point, it has already been determined that the color is either
    // direct or indirect.
    defines::EString mode = color [ "Mode" ].as< defines::EString > ( );
    if ( mode == "DIRECT" )
    {
        std::shared_ptr< RGBAColor > output =
                std::shared_ptr< RGBAColor > ( new RGBAColor ( ) );
        // grab the base
        auto base = color [ "Base" ];

        for ( std::size_t i = 0; i < base.size ( ); i++ )
        {
            output->setBasicComponent (
                    i,
                    base [ i ].as< defines::UnboundColor > ( ) );
        }
        // zero-initialize any ommitted values
        for ( std::size_t i = base.size ( ); i < 4; i++ )
        {
            output->setBasicComponent ( i, 0 );
        }
        return output;
    } else if ( mode == "INDIRECT" )
    {
        std::shared_ptr< IndirectColor > output =
                std::shared_ptr< IndirectColor > ( new IndirectColor ( ) );
        // same basic component grabbing as before, but, this time the
        // entire field is optional.
        if ( color [ "Base" ] )
        {
            auto base = color [ "Base" ];
            // currently, defines::UnboundColor is a double
            for ( std::size_t i = 0; i < base.size ( ); i++ )
            {
                output->setBasicComponent (
                        i,
                        base [ i ].as< defines::UnboundColor > ( ) );
            }
            // zero-initialize any ommitted values
            for ( std::size_t i = base.size ( ); i < 4; i++ )
            {
                output->setBasicComponent ( i, 0 );
            }
        } else
        {
            for ( std::size_t i = 0; i < 4; i++ )
            {
                output->setBasicComponent ( i, 0 );
            }
        }
        // similar grabbing mechanism, but this time for integers, and they
        // must be >=0 and the field is required.
        auto args = color [ "Args" ];
        if ( !args )
        {
            RUNTIME_ERROR ( "The parameter \"Args\" is required." )
        }
        if ( args.size ( ) != 4 )
        {
            RUNTIME_ERROR (
                    "Must have four color reference arguments for indirect "
                    "colors. Got ",
                    args.size ( ),
                    " instead." )
        }
        for ( std::size_t i = 0; i < 4; i++ )
        {
            std::size_t index = args [ i ].as< std::size_t > ( );
            output->setParam ( std::uint8_t ( i ), colors.at ( index ) );
        }
        // set the function. This field is optional.
        if ( color [ "Function" ] )
        {
            defines::EStringStream temp {
                    color [ "Function" ].as< defines::EString > ( ) };
            SerializedColorFunction scf;
            temp >> scf;
            switch ( scf )
            {
                case SerializedColorFunction::_MAX: // an error condition
                case SerializedColorFunction::WAVEFORM:
                    output->setBlendFunction (
                            blend_functions::defaultBlending );
                    break;
                case SerializedColorFunction::AVERAGE4:
                    output->setBlendFunction ( blend_functions::averageAdjust );
                    break;
                case SerializedColorFunction::AVERAGE5:
                    output->setBlendFunction ( blend_functions::fullAverage );
            }
        }
        return output;
    } else
    {
        RUNTIME_ERROR ( "Mode was not DIRECT or INDIRECT but ", mode );
    }
}