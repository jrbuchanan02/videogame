/**
 * @file direct.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Direct color implementations.
 * @version 1
 * @date 2022-02-27
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */

#include <io/console/colors/direct.h++>

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

void io::console::colors::DirectColor::baseRefresh ( ) const noexcept
{
    for ( std::size_t i = 0; i < 4; i++ )
    {
        this->color [ i ] = this->basic [ i ];
    }
}

io::console::colors::DirectColor::DirectColor (
        defines::UnboundColor const &_1,
        defines::UnboundColor const &_2,
        defines::UnboundColor const &_3,
        defines::UnboundColor const &_4 ) noexcept :
        DirectColor ( )
{
    this->color [ 0 ] = _1;
    this->color [ 1 ] = _2;
    this->color [ 2 ] = _3;
    this->color [ 3 ] = _4;
    baseRefresh ( );
}

void io::console::colors::DirectColor::refresh (
        double const &time ) const noexcept
{
    baseRefresh ( );
}

bool io::console::colors::DirectColor::references (
        IColor const *const &color ) const noexcept
{
    return color == this;
}

defines::UnboundColor const *const
        io::console::colors::RGBAColor::rgbaRaw ( ) const noexcept
{
    defines::UnboundColor *result = new defines::UnboundColor [ 4 ];
    for ( std::size_t i = 0; i < 4; i++ ) { result [ i ] = this->color [ i ]; }
    return result;
}

defines::UnboundColor const *const
        io::console::colors::RGBAColor::cmykRaw ( ) const noexcept
{
    defines::UnboundColor *result    = ( defines::UnboundColor    *) rgbaRaw ( );
    defines::UnboundColor  magnitude = DirectColor::normalizeColor ( result );
    if ( magnitude == 0 )
    {
        result [ 0 ] = result [ 1 ] = result [ 2 ] = 0;
        result [ 3 ]                               = 255;
    } else
    {
        result [ 3 ] = 1
                     - std::max ( {
                             result [ 0 ],
                             result [ 1 ],
                             result [ 2 ],
                     } );
        for ( std::size_t i = 0; i < 3; i++ )
        {
            result [ i ] =
                    ( 1 - result [ i ] - result [ 3 ] ) / ( 1 - result [ 3 ] );
            result [ i ] *= magnitude;
        }
        result [ 3 ] *= magnitude;
    }
    return result;
}

defines::UnboundColor const *const
        io::console::colors::RGBAColor::cmyaRaw ( ) const noexcept
{
    defines::UnboundColor *result    = ( defines::UnboundColor    *) rgbaRaw ( );
    defines::UnboundColor  magnitude = DirectColor::normalizeColor ( result );
    if ( magnitude == 0 )
    {
        result [ 0 ] = result [ 1 ] = result [ 2 ] = 255;
    } else
    {
        for ( std::size_t i = 0; i < 3; i++ )
        {
            result [ i ] = 1 - result [ i ];
            result [ i ] *= magnitude;
        }
    }
    return result;
}

defines::UnboundColor const *const
        io::console::colors::CMYAColor::rgbaRaw ( ) const noexcept
{
    defines::UnboundColor *result    = ( defines::UnboundColor    *) cmyaRaw ( );
    defines::UnboundColor  magnitude = DirectColor::normalizeColor ( result );
    if ( magnitude == 0 )
    {
        result [ 0 ] = result [ 1 ] = result [ 2 ] = 255;
    } else
    {
        for ( std::size_t i = 0; i < 3; i++ )
        {
            result [ i ] = 1 - result [ i ];
            result [ i ] *= magnitude;
        }
    }
    return result;
}

defines::UnboundColor const *const
        io::console::colors::CMYAColor::cmykRaw ( ) const noexcept
{
    defines::UnboundColor *result    = ( defines::UnboundColor    *) cmyaRaw ( );
    defines::UnboundColor  magnitude = DirectColor::normalizeColor ( result );
    if ( magnitude == 0 )
    {
        result [ 0 ] = result [ 1 ] = result [ 2 ] = 0;
        result [ 3 ]                               = 255;
    } else
    {
        for ( std::size_t i = 0; i < 3; i++ )
        {
            result [ i ] =
                    ( result [ i ] - result [ 3 ] ) / ( 1 - result [ 3 ] );
            result [ i ] *= magnitude;
        }
        result [ 3 ] *= magnitude;
    }
    return result;
}

defines::UnboundColor const *const io::console::colors::CMYAColor::cmyaRaw() const noexcept
{
    defines::UnboundColor *result = new defines::UnboundColor [ 4 ];
    for ( std::size_t i = 0; i < 4; i++ ) { result [ i ] = this->color [ i ]; }
    return result;
}

defines::UnboundColor const *const io::console::colors::CMYKColor::rgbaRaw() const noexcept
{
    defines::UnboundColor *result    = ( defines::UnboundColor    *) cmykRaw ( );
    // more unique: hypot 4. Because of how the colors work,
    // all valid cmyk colors have a magnitude != 0.
    defines::UnboundColor  magnitude = 0;
    for ( std::size_t i = 0; i < 4; i++ )
    {
        magnitude += result [ i ] * result [ i ];
    }
    magnitude = std::sqrt ( magnitude );
    for ( std::size_t i = 0; i < 4; i++ ) { result [ i ] /= magnitude; }

    // cmyk formaula for a color component n solved for rgba
    // a = 0
    // n_k = (1 - n_a - k) / (1 - k)
    // n_k * (1 - k ) = 1 - n_a - k
    // n_k * (1 - k ) + k - 1 = -n_a
    // 1 - k - n_k * (1 - k ) = n_a
    // (1 - k)(1 - n_k) = n_a

    for ( std::size_t i = 0; i < 3; i++ )
    {
        result [ i ] = ( 1 - result [ 3 ] ) * ( 1 - result [ i ] );
        result [ i ] *= magnitude;
    }
    result [ 3 ] = 0;
    return result;
}

defines::UnboundColor const *const io::console::colors::CMYKColor::cmykRaw() const noexcept
{
    defines::UnboundColor *result = new defines::UnboundColor [ 4 ];
    for ( std::size_t i = 0; i < 4; i++ ) { result [ i ] = this->color [ i ]; }
    return result;
}

defines::UnboundColor const *const io::console::colors::CMYKColor::cmyaRaw() const noexcept
{
    defines::UnboundColor *result    = ( defines::UnboundColor    *) cmykRaw ( );
    // more unique: hypot 4. Because of how the colors work,
    // all valid cmyk colors have a magnitude != 0.
    defines::UnboundColor  magnitude = 0;
    for ( std::size_t i = 0; i < 4; i++ )
    {
        magnitude += result [ i ] * result [ i ];
    }
    magnitude = std::sqrt ( magnitude );
    for ( std::size_t i = 0; i < 4; i++ ) { result [ i ] /= magnitude; }

    // cmyk formaula for a color component n solved for cmya
    // a = 0
    // n_k = (n_a - k) / (1 - k)
    // n_k(1 - k) + k = n_a

    for ( std::size_t i = 0; i < 3; i++ )
    {
        result [ i ] = result [ i ] * ( 1 - result [ 3 ] ) + result [ 3 ];
        result [ i ] *= magnitude;
    }
    result [ 3 ] = 0;
    return result;
}