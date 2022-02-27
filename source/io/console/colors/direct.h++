/**
 * @file direct.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Direct color things
 * @version 1
 * @date 2022-02-14
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <io/console/colors/color.h++>

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
    /**
     * @brief Color where the blend arguments are the color.
     *
     */
    class DirectColor : public IColor
    {
        void baseRefresh ( ) const noexcept;
    public:
        POLYMORPHIC_IDENTIFIER ( DirectColor )
        DirectColor ( ) noexcept = default;
        DirectColor ( defines::UnboundColor const &,
                      defines::UnboundColor const &,
                      defines::UnboundColor const &,
                      defines::UnboundColor const & ) noexcept;
        virtual ~DirectColor ( )                     = default;
        DirectColor ( DirectColor const & ) noexcept = default;
        DirectColor ( DirectColor && ) noexcept      = default;
        DirectColor &operator= ( DirectColor const & ) noexcept = default;
        DirectColor &operator= ( DirectColor && ) noexcept = default;

        virtual void
                refresh ( double const & = 0 ) const noexcept override final;

        // the whole point of a direct color is that there are no references.
        // thus, all color references **must** eventually resolve to a direct
        // color of some form.
        virtual bool references (
                IColor const *const & ) const noexcept override final;
    };

    class RGBAColor : public DirectColor
    {
    protected:
        virtual defines::UnboundColor const *const
                rgbaRaw ( ) const noexcept override final;

        virtual defines::UnboundColor const *const
                cmykRaw ( ) const noexcept override final;

        virtual defines::UnboundColor const *const
                cmyaRaw ( ) const noexcept override final;
    public:
        POLYMORPHIC_IDENTIFIER ( RGBAColor )
        RGBAColor ( ) noexcept = default;
        virtual ~RGBAColor ( ) = default;
        RGBAColor ( defines::UnboundColor const &r,
                    defines::UnboundColor const &g,
                    defines::UnboundColor const &b,
                    defines::UnboundColor const &a ) noexcept :
                DirectColor ( r, g, b, a )
        { }
        RGBAColor ( RGBAColor const & ) noexcept = default;
        RGBAColor ( RGBAColor && ) noexcept      = default;
        RGBAColor &operator= ( RGBAColor const & ) noexcept = default;
        RGBAColor &operator= ( RGBAColor && ) noexcept = default;
    };

    class CMYAColor : public DirectColor
    {
    protected:
        virtual defines::UnboundColor const *const
                rgbaRaw ( ) const noexcept override final;

        virtual defines::UnboundColor const *const
                cmykRaw ( ) const noexcept override final;

        virtual defines::UnboundColor const *const
                cmyaRaw ( ) const noexcept override final;
    public:
        POLYMORPHIC_IDENTIFIER ( CMYAColor )
        CMYAColor ( ) noexcept = default;
        virtual ~CMYAColor ( ) = default;
        CMYAColor ( defines::UnboundColor const &c,
                    defines::UnboundColor const &m,
                    defines::UnboundColor const &y,
                    defines::UnboundColor const &a ) noexcept :
                DirectColor ( c, m, y, a )
        { }
        CMYAColor ( CMYAColor const & ) noexcept = default;
        CMYAColor ( CMYAColor && ) noexcept      = default;
        CMYAColor &operator= ( CMYAColor const & ) noexcept = default;
        CMYAColor &operator= ( CMYAColor && ) noexcept = default;
    };

    class CMYKColor : public DirectColor
    {
    protected:
        virtual defines::UnboundColor const *const
                rgbaRaw ( ) const noexcept override final;
        virtual defines::UnboundColor const *const
                cmykRaw ( ) const noexcept override final;

        virtual defines::UnboundColor const *const
                cmyaRaw ( ) const noexcept override final;
    public:
        POLYMORPHIC_IDENTIFIER ( CMYKColor )
        CMYKColor ( ) noexcept = default;
        virtual ~CMYKColor ( ) = default;
        CMYKColor ( defines::UnboundColor const &c,
                    defines::UnboundColor const &m,
                    defines::UnboundColor const &y,
                    defines::UnboundColor const &k ) noexcept :
                DirectColor ( c, m, y, k )
        { }
        CMYKColor ( CMYKColor const & ) noexcept = default;
        CMYKColor ( CMYKColor && ) noexcept      = default;
        CMYKColor &operator= ( CMYKColor const & ) noexcept = default;
        CMYKColor &operator= ( CMYKColor && ) noexcept = default;
    };
} // namespace io::console::colors