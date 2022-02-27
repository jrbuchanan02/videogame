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
    defines::BoundColor const bind ( defines::UnboundColor const color );

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

        static defines::UnboundColor const
                normalizeColor ( defines::UnboundColor *const &color ) noexcept;
    public:
        POLYMORPHIC_IDENTIFIER ( IColor )

        IColor ( ) noexcept;
        virtual ~IColor ( )                = default;
        IColor ( IColor const & ) noexcept = default;
        IColor ( IColor && ) noexcept      = default;
        IColor &operator= ( IColor const & ) noexcept = default;
        IColor &operator= ( IColor && ) noexcept = default;

        defines::UnboundColor const *const
                rgba ( double const & = 0 ) const noexcept;
        defines::UnboundColor const *const
                cmyk ( double const & = 0 ) const noexcept;
        defines::UnboundColor const *const
                cmya ( double const & = 0 ) const noexcept;

        virtual void refresh ( double const &time = 0 ) const noexcept   = 0;
        virtual bool references ( IColor const *const & ) const noexcept = 0;

        defines::UnboundColor const &
                getBasicComponent ( std::size_t const & ) const;

        void setBasicComponent ( std::size_t const &,
                                 defines::UnboundColor const & );
    };

} // namespace io::console::colors