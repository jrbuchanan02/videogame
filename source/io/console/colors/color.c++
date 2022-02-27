/**
 * @file color.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Moves implementation things out of color.h++
 * @version 1
 * @date 2022-02-27
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */

#include <io/console/colors/color.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace io::console::colors;

defines::BoundColor const
        io::console::colors::bind ( defines::UnboundColor const color )
{
    if ( color > 0xff )
    {
        return 0xff;
    } else if ( color < 0x00 )
    {
        return 0x00;
    } else if ( std::isnan ( color ) )
    {
        RUNTIME_ERROR ( "NAN" )
    } else
    {
        return defines::BoundColor ( color );
    }
}

defines::UnboundColor const io::console::colors::IColor::normalizeColor (
        defines::UnboundColor *const &color ) noexcept
{
    defines::UnboundColor magnitude =
            std::hypot ( color [ 0 ], color [ 1 ], color [ 2 ] );
    if ( magnitude == 0 )
    {
        return 0.0;
    }
    for ( std::size_t i = 0; i < 3; i++ ) { color [ i ] /= magnitude; }
    return magnitude;
}

io::console::colors::IColor::IColor ( ) noexcept :
        cache { 0, 0, 0, 0 }, color { 0, 0, 0, 0 }, basic { 0, 0, 0, 0 }
{ }

defines::UnboundColor const *const
        io::console::colors::IColor::rgba ( double const &time ) const noexcept
{
    refresh ( time );
    return rgbaRaw ( );
}

defines::UnboundColor const *const
        io::console::colors::IColor::cmyk ( double const &time ) const noexcept
{
    refresh ( time );
    return cmykRaw ( );
}

defines::UnboundColor const *const
        io::console::colors::IColor::cmya ( double const &time ) const noexcept
{
    refresh ( time );
    return cmyaRaw ( );
}

defines::UnboundColor const &io::console::colors::IColor::getBasicComponent (
        std::size_t const &i ) const
{
    if ( i > 3 )
    {
        RUNTIME_ERROR ( "Index out of bounds: ", i )
    } else
    {
        return basic [ i ];
    }
}

void io::console::colors::IColor::setBasicComponent (
        std::size_t const           &i,
        defines::UnboundColor const &color )
{
    if ( i > 3 )
    {
        RUNTIME_ERROR ( "Index out of bounds: ", i )
    } else
    {
        basic [ i ] = color;
    }
}