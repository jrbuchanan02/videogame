/**
 * @file indirect.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief This file moves implementations out of indirect.h++
 * @version 1
 * @date 2022-02-27
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <io/console/colors/indirect.h++>

#include <io/console/colors/color.h++>
#include <io/console/colors/direct.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <numbers>
#include <sstream>
#include <stdexcept>

using namespace io::console::colors;
using namespace io::console::colors::blend_functions;

defines::UnboundColor const *const
        io::console::colors::IndirectColor::rgbaRaw ( ) const noexcept
{
    return RGBAColor ( this->color [ 0 ],
                       this->color [ 1 ],
                       this->color [ 2 ],
                       this->color [ 3 ] )
            .rgba ( );
}

defines::UnboundColor const *const
        io::console::colors::IndirectColor::cmyaRaw ( ) const noexcept
{
    return RGBAColor ( this->color [ 0 ],
                       this->color [ 1 ],
                       this->color [ 2 ],
                       this->color [ 3 ] )
            .cmya ( );
}

defines::UnboundColor const *const
        io::console::colors::IndirectColor::cmykRaw ( ) const noexcept
{
    return RGBAColor ( this->color [ 0 ],
                       this->color [ 1 ],
                       this->color [ 2 ],
                       this->color [ 3 ] )
            .cmyk ( );
}

io::console::colors::IndirectColor::IndirectColor (
        std::shared_ptr< IColor > const &amplitude,
        std::shared_ptr< IColor > const &frequency,
        std::shared_ptr< IColor > const &frequencyModulation,
        std::shared_ptr< IColor > const &amplitudeModulation ) noexcept :
        IColor ( ),
        delta ( amplitude ), fmMod ( frequencyModulation ),
        amMod ( amplitudeModulation ), freqs ( frequency )
{
    this->refresh ( );
}

io::console::colors::IndirectColor::IndirectColor (
        defines::UnboundColor const     &r,
        defines::UnboundColor const     &g,
        defines::UnboundColor const     &b,
        defines::UnboundColor const     &a,
        std::shared_ptr< IColor > const &amplitude,
        std::shared_ptr< IColor > const &frequency,
        std::shared_ptr< IColor > const &frequencyModulation,
        std::shared_ptr< IColor > const &amplitudeModulation ) noexcept :
        IndirectColor ( amplitude,
                        frequency,
                        frequencyModulation,
                        amplitudeModulation )
{
    this->basic [ 0 ] = r;
    this->basic [ 1 ] = g;
    this->basic [ 2 ] = b;
    this->basic [ 3 ] = a;
    this->refresh ( );
}

void io::console::colors::IndirectColor::refresh (
        double const &time ) const noexcept
{
    auto deltas = delta->rgba ( time );
    auto fmMods = fmMod->rgba ( time - std::numbers::pi / 2 );
    auto amMods = amMod->rgba ( time );
    auto cfreqs = freqs->rgba ( time );

    for ( std::size_t i = 0; i < 4; i++ )
    {
        // offsetting fmMod by pi / 2 allows the default blend function to
        // get closer to frequency modulation. Since we don't ever actually
        // take the integral (something really only reserved for software
        // defined radios!), this is about as close as I'm willing to get to
        // phase modulation for now.
        this->color [ i ] = blender ( time,
                                      this->basic [ i ],
                                      deltas [ i ],
                                      cfreqs [ i ],
                                      fmMods [ i ],
                                      amMods [ i ] );
    }

    // remove the arrays
    delete [] deltas;
    delete [] fmMods;
    delete [] amMods;
    delete [] cfreqs;
}

bool io::console::colors::IndirectColor::references (
        IColor const *const &color ) const noexcept
{
    auto check = [ & ] ( std::shared_ptr< IColor > const &c ) -> bool {
        return c->references ( color );
    };

    if ( color == this )
    {
        return true;
    } else if ( check ( delta ) || check ( fmMod ) || check ( amMod )
                || check ( freqs ) )
    {
        return true;
    } else
    {
        return false;
    }

    /*
     * OLD:
     *   if ( color == this )
     *       {
     *           return true;
     *       } else
     *       {
     *           if ( delta->references ( color ) )
     *           {
     *               return true;
     *           } else if ( fmMod->references ( color ) )
     *           {
     *               return true;
     *           } else if ( amMod->references ( color ) )
     *           {
     *               return true;
     *           } else if ( freqs->references ( color ) )
     *           {
     *               return true;
     *           } else
     *           {
     *               return false;
     *           }
     *       }
     *
     */
}

BlendFunction const &
        io::console::colors::IndirectColor::getBlendFunction ( ) const noexcept
{
    return blender;
}

void io::console::colors::IndirectColor::setBlendFunction (
        BlendFunction const &blender ) noexcept
{
    this->blender = blender;
}

void io::console::colors::IndirectColor::setParam (
        std::uint8_t const              &param,
        std::shared_ptr< IColor > const &to )
{
    if ( param >= 4 )
    {
        RUNTIME_ERROR ( "Parameter ",
                        ( std::uint32_t ) param,
                        " is out of range." )
    } else if ( !this->references ( to.get ( ) ) )
    {
        switch ( param & 3 )
        {
            case 0: delta = to; break;
            case 1: fmMod = to; break;
            case 2: amMod = to; break;
            case 3: freqs = to; break;
            // param can only take on 0, 1, 2, 3. so if we
            // managed to get here, it was by some weird hackery.
            default: assert ( false );
        }
    }
}