/**
 * @file color.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A color
 * @version 1
 * @date 2022-02-14
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace io::console::colors
{
    static inline constexpr defines::BoundColor const
            bind ( defines::UnboundColor const color ) noexcept
    {
        if ( color > 0xff )
        {
            return 0xff;
        } else if ( color < 0x00 )
        {
            return 0x00;
        } else if ( std::isnan ( color ) )
        {
            return 0x00;
        } else
        {
            return defines::BoundColor ( color );
        }
    }

    class IColor
    {
    protected:
        // cache for any use by some subclass
        defines::UnboundColor mutable cache [ 4 ];
        // the color for current use
        defines::UnboundColor mutable color [ 4 ];
        // blend arguments we receive, these are what's written to the
        // color
        defines::UnboundColor mutable basic [ 4 ];
        // the raw-getter functions are expected to allocate a new buffer.
        // a call to refresh always preceeds these functions.
        virtual defines::UnboundColor const *const
                rgbaRaw ( ) const noexcept = 0;
        virtual defines::UnboundColor const *const
                cmykRaw ( ) const noexcept = 0;
        virtual defines::UnboundColor const *const
                cmyaRaw ( ) const noexcept = 0;

        static inline defines::UnboundColor const
                normalizeColor ( defines::UnboundColor *const &color ) noexcept
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
    public:
        POLYMORPHIC_IDENTIFIER ( IColor )

        IColor ( ) noexcept = default;
        virtual ~IColor ( ) = default;
        IColor ( IColor const & ) noexcept = default;
        IColor ( IColor && ) noexcept      = default;
        IColor &operator= ( IColor const & ) noexcept = default;
        IColor &operator= ( IColor && ) noexcept = default;

        defines::UnboundColor const *const
                rgba ( double const &time = 0 ) const noexcept
        {
            refresh ( time );
            return rgbaRaw ( );
        }
        defines::UnboundColor const *const
                cmyk ( double const &time = 0 ) const noexcept
        {
            refresh ( time );
            return cmykRaw ( );
        }
        defines::UnboundColor const *const
                cmya ( double const &time = 0 ) const noexcept
        {
            refresh ( time );
            return cmyaRaw ( );
        }

        virtual void refresh ( double const &time = 0 ) const noexcept   = 0;
        virtual bool references ( IColor const *const & ) const noexcept = 0;

        defines::UnboundColor const &
                getBasicComponent ( std::size_t const &i ) const
        {
            if ( i > 3 )
            {
                RUNTIME_ERROR ( "Index out of bounds: ", i )
            } else
            {
                return basic [ i ];
            }
        }

        void setBasicComponent ( std::size_t const           &i,
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
    };

} // namespace io::console::colors