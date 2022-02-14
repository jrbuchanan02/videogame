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

#include <io/console/internal/colors/color.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <cmath>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace io::console::internal::colors
{
    /**
     * @brief Color where the blend arguments are the color.
     *
     */
    class DirectColor : public IColor
    {
    public:
        POLYMORPHIC_IDENTIFIER ( DirectColor )
        DirectColor ( ) noexcept = default;
        DirectColor ( defines::UnboundColor const &_1,
                      defines::UnboundColor const &_2,
                      defines::UnboundColor const &_3,
                      defines::UnboundColor const &_4 ) noexcept :
                DirectColor ( )
        {
            this->color [ 0 ] = _1;
            this->color [ 1 ] = _2;
            this->color [ 2 ] = _3;
            this->color [ 3 ] = _4;
        }
        virtual ~DirectColor ( )                     = default;
        DirectColor ( DirectColor const & ) noexcept = default;
        DirectColor ( DirectColor && ) noexcept      = default;
        DirectColor &operator= ( DirectColor const & ) noexcept = default;
        DirectColor &operator= ( DirectColor && ) noexcept = default;

        virtual void
                refresh ( double const &time = 0 ) const noexcept override final
        {
            for ( std::size_t i = 0; i < 4; i++ )
            {
                this->color [ i ] = this->basic [ i ];
            }
        }

        // the whole point of a direct color is that there are no references.
        // thus, all color references **must** eventually resolve to a direct
        // color of some form.
        virtual bool references (
                IColor const *const &color ) const noexcept override final
        {
            return color == this;
        }
    };

    class RGBAColor : public DirectColor
    {
    protected:
        virtual defines::UnboundColor const *const &
                rgbaRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result = new defines::UnboundColor [ 4 ];
            for ( std::size_t i = 0; i < 4; i++ )
            {
                result [ i ] = this->color [ i ];
            }
            return result;
        }

        virtual defines::UnboundColor const *const &
                cmykRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result =
                    ( defines::UnboundColor * ) rgbaRaw ( );
            defines::UnboundColor magnitude =
                    DirectColor::normalizeColor ( result );
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
                    result [ i ] = ( 1 - result [ i ] - result [ 3 ] )
                                 / ( 1 - result [ 3 ] );
                    result [ i ] *= magnitude;
                }
                result [ 3 ] *= magnitude;
            }
            return result;
        }

        virtual defines::UnboundColor const *const &
                cmyaRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result =
                    ( defines::UnboundColor * ) rgbaRaw ( );
            defines::UnboundColor magnitude =
                    DirectColor::normalizeColor ( result );
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
        virtual defines::UnboundColor const *const &
                rgbaRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result =
                    ( defines::UnboundColor * ) cmyaRaw ( );
            defines::UnboundColor magnitude =
                    DirectColor::normalizeColor ( result );
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
        virtual defines::UnboundColor const *const &
                cmykRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result =
                    ( defines::UnboundColor * ) cmyaRaw ( );
            defines::UnboundColor magnitude =
                    DirectColor::normalizeColor ( result );
            if ( magnitude == 0 )
            {
                result [ 0 ] = result [ 1 ] = result [ 2 ] = 0;
                result [ 3 ]                               = 255;
            } else
            {
                for ( std::size_t i = 0; i < 3; i++ )
                {
                    result [ i ] = ( result [ i ] - result [ 3 ] )
                                 / ( 1 - result [ 3 ] );
                    result [ i ] *= magnitude;
                }
                result [ 3 ] *= magnitude;
            }
            return result;
        }

        virtual defines::UnboundColor const *const &
                cmyaRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result = new defines::UnboundColor [ 4 ];
            for ( std::size_t i = 0; i < 4; i++ )
            {
                result [ i ] = this->color [ i ];
            }
            return result;
        }
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
        virtual defines::UnboundColor const *const &
                rgbaRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result =
                    ( defines::UnboundColor * ) cmykRaw ( );
            // more unique: hypot 4. Because of how the colors work,
            // all valid cmyk colors have a magnitude != 0.
            defines::UnboundColor magnitude = 0;
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
        virtual defines::UnboundColor const *const &
                cmykRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result = new defines::UnboundColor [ 4 ];
            for ( std::size_t i = 0; i < 4; i++ )
            {
                result [ i ] = this->color [ i ];
            }
            return result;
        }

        virtual defines::UnboundColor const *const &
                cmyaRaw ( ) const noexcept override final
        {
            defines::UnboundColor *result =
                    ( defines::UnboundColor * ) cmykRaw ( );
            // more unique: hypot 4. Because of how the colors work,
            // all valid cmyk colors have a magnitude != 0.
            defines::UnboundColor magnitude = 0;
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
                result [ i ] =
                        result [ i ] * ( 1 - result [ 3 ] ) + result [ 3 ];
                result [ i ] *= magnitude;
            }
            result [ 3 ] = 0;
            return result;
        }
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
} // namespace io::console::internal::colors