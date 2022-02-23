/**
 * @file indirect.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Indirect color
 * @version 1
 * @date 2022-02-14
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

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

namespace io::console::colors
{
    namespace blend_functions
    {
        using BlendFunction = std::function< defines::UnboundColor (
                double const &,
                defines::UnboundColor const &,
                defines::UnboundColor const &,
                defines::UnboundColor const &,
                defines::UnboundColor const &,
                defines::UnboundColor const & ) >;

        static inline BlendFunction defaultBlending =
                [] ( double const                &time,
                     defines::UnboundColor const &basic,
                     defines::UnboundColor const &amplitude,
                     defines::UnboundColor const &frequency,
                     defines::UnboundColor const &frequencyModulation,
                     defines::UnboundColor const &amplitudeModulation )
                -> defines::UnboundColor {
            defines::UnboundColor omega = 2 * std::numbers::pi * frequency;
            defines::UnboundColor phi =
                    frequencyModulation * 2 * std::numbers::pi;
            return basic + amplitude * std::sin ( omega * time + phi )
                 + amplitude * amplitudeModulation;
        };

        static inline BlendFunction averageAdjust =
                [] ( double const &,
                     defines::UnboundColor const &basic,
                     defines::UnboundColor const &amplitude,
                     defines::UnboundColor const &frequency,
                     defines::UnboundColor const &frequencyModulation,
                     defines::UnboundColor const &amplitudeModulation )
                -> defines::UnboundColor {
            return basic
                 + ( amplitude + frequency + frequencyModulation
                     + amplitudeModulation )
                           * 0.25;
        };

        static inline BlendFunction fullAverage =
                [] ( double const &,
                     defines::UnboundColor const &basic,
                     defines::UnboundColor const &amplitude,
                     defines::UnboundColor const &frequency,
                     defines::UnboundColor const &frequencyModulation,
                     defines::UnboundColor const &amplitudeModulation )
                -> defines::UnboundColor {
            return ( basic + amplitude + frequency + frequencyModulation
                     + amplitudeModulation )
                 * 0.20;
        };
    } // namespace blend_functions

    /**
     * @brief Color where its refresh generates a new color on the fly from a
     * series of other colors.
     * @note internally, all indirect colors are RGBA colors.
     * @details Indirect color has four internally referenced colors: Amplitude,
     * frequency, frequency-modulation, and amplitude modulation. However, these
     * four parameters can be used in any method by any specified blend
     * function.
     */
    class IndirectColor : public IColor
    {
    private:
        std::shared_ptr< IColor > delta;
        std::shared_ptr< IColor > fmMod;
        std::shared_ptr< IColor > amMod;
        std::shared_ptr< IColor > freqs;

        blend_functions::BlendFunction blender =
                blend_functions::defaultBlending;
    protected:
        virtual defines::UnboundColor const *const
                rgbaRaw ( ) const noexcept override final
        {
            return RGBAColor ( this->color [ 0 ],
                               this->color [ 1 ],
                               this->color [ 2 ],
                               this->color [ 3 ] )
                    .rgba ( );
        }
        virtual defines::UnboundColor const *const
                cmyaRaw ( ) const noexcept override final
        {
            return RGBAColor ( this->color [ 0 ],
                               this->color [ 1 ],
                               this->color [ 2 ],
                               this->color [ 3 ] )
                    .cmya ( );
        }
        virtual defines::UnboundColor const *const
                cmykRaw ( ) const noexcept override final
        {
            return RGBAColor ( this->color [ 0 ],
                               this->color [ 1 ],
                               this->color [ 2 ],
                               this->color [ 3 ] )
                    .cmyk ( );
        }
    public:
        POLYMORPHIC_IDENTIFIER ( IndirectColor )

        IndirectColor ( ) noexcept = default;
        virtual ~IndirectColor ( ) = default;
        IndirectColor (
                std::shared_ptr< IColor > const &amplitude,
                std::shared_ptr< IColor > const &frequency,
                std::shared_ptr< IColor > const &freqModulation,
                std::shared_ptr< IColor > const &ampModulation ) noexcept :
                IColor ( ),
                delta ( amplitude ), fmMod ( freqModulation ),
                amMod ( ampModulation ), freqs ( frequency )
        {
            this->refresh ( );
        }
        IndirectColor (
                defines::UnboundColor const     &r,
                defines::UnboundColor const     &g,
                defines::UnboundColor const     &b,
                defines::UnboundColor const     &a,
                std::shared_ptr< IColor > const &amplitude,
                std::shared_ptr< IColor > const &frequency,
                std::shared_ptr< IColor > const &freqModulation,
                std::shared_ptr< IColor > const &ampModulation ) noexcept :
                IndirectColor ( amplitude,
                                frequency,
                                freqModulation,
                                ampModulation )
        {
            this->basic [ 0 ] = r;
            this->basic [ 1 ] = g;
            this->basic [ 2 ] = b;
            this->basic [ 3 ] = a;
            this->refresh ( );
        }
        IndirectColor ( IndirectColor const & ) noexcept = default;
        IndirectColor ( IndirectColor && ) noexcept      = default;
        IndirectColor &operator= ( IndirectColor const & ) noexcept = default;
        IndirectColor &operator= ( IndirectColor && ) noexcept = default;

        void refresh ( double const &time = 0 ) const noexcept override final
        {
            auto deltas = delta->rgba ( time );
            auto fmMods = fmMod->rgba ( time - std::numbers::pi / 2 );
            auto amMods = amMod->rgba ( time );
            auto cfreqs = freqs->rgba ( time );

            for ( std::size_t i = 0; i < 4; i++ )
            {
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

        bool references ( IColor const *const &color ) const noexcept
        {
            if ( color == this )
            {
                return true;
            } else
            {
                if ( delta->references ( color ) )
                {
                    return true;
                } else if ( fmMod->references ( color ) )
                {
                    return true;
                } else if ( amMod->references ( color ) )
                {
                    return true;
                } else if ( freqs->references ( color ) )
                {
                    return true;
                } else
                {
                    return false;
                }
            }
        }

        blend_functions::BlendFunction const &
                getBlendFunction ( ) const noexcept
        {
            return blender;
        }
        void setBlendFunction (
                blend_functions::BlendFunction const &blender ) noexcept
        {
            this->blender = blender;
        }

        void setDelta ( std::shared_ptr< IColor > const &delta ) noexcept
        {
            if ( !this->references ( delta.get ( ) ) )
            {
                this->delta = delta;
            }
        }

        void setFmMod ( std::shared_ptr< IColor > const &fmMod ) noexcept
        {
            if ( !this->references ( fmMod.get ( ) ) )
            {
                this->fmMod = fmMod;
            }
        }

        void setAmMod ( std::shared_ptr< IColor > const &amMod ) noexcept
        {
            if ( !this->references ( amMod.get ( ) ) )
            {
                this->amMod = amMod;
            }
        }

        void setFreqs ( std::shared_ptr< IColor > const &freqs ) noexcept
        {
            if ( !this->references ( freqs.get ( ) ) )
            {
                this->freqs = freqs;
            }
        }

        void setParam ( std::uint8_t const              &param,
                        std::shared_ptr< IColor > const &to )
        {
            if ( param >= 4 )
            {
                RUNTIME_ERROR ( "Parameter out of range: ",
                                ( std::uint32_t ) param )
            } else
            {
                switch ( param & 3 )
                {
                    case 0: setDelta ( to ); break;
                    case 1: setFmMod ( to ); break;
                    case 2: setAmMod ( to ); break;
                    case 3:
                    default: setFreqs ( to );
                }
            }
        }
    };
} // namespace io::console::colors