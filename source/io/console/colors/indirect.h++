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
        enum class IndirectColorBlendingFunctions
        {
            WAVEFORM,
            AVERAGE4,
            AVERAGE5,
            _MAX,
        };
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
                    -frequencyModulation * 2 * std::numbers::pi;
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
                rgbaRaw ( ) const noexcept override final;
        virtual defines::UnboundColor const *const
                cmyaRaw ( ) const noexcept override final;
        virtual defines::UnboundColor const *const
                cmykRaw ( ) const noexcept override final;
    public:
        POLYMORPHIC_IDENTIFIER ( IndirectColor )

        IndirectColor ( ) noexcept = default;
        virtual ~IndirectColor ( ) = default;
        IndirectColor (
                std::shared_ptr< IColor > const &amplitude,
                std::shared_ptr< IColor > const &frequency,
                std::shared_ptr< IColor > const &freqModulation,
                std::shared_ptr< IColor > const &ampModulation ) noexcept;
        IndirectColor (
                defines::UnboundColor const     &r,
                defines::UnboundColor const     &g,
                defines::UnboundColor const     &b,
                defines::UnboundColor const     &a,
                std::shared_ptr< IColor > const &amplitude,
                std::shared_ptr< IColor > const &frequency,
                std::shared_ptr< IColor > const &freqModulation,
                std::shared_ptr< IColor > const &ampModulation ) noexcept;
        IndirectColor ( IndirectColor const & ) noexcept = default;
        IndirectColor ( IndirectColor && ) noexcept      = default;
        IndirectColor &operator= ( IndirectColor const & ) noexcept = default;
        IndirectColor &operator= ( IndirectColor && ) noexcept = default;

        void refresh ( double const & = 0 ) const noexcept override final;

        bool references ( IColor const *const &color ) const noexcept;

        blend_functions::BlendFunction const &
                getBlendFunction ( ) const noexcept;

        void setBlendFunction (
                blend_functions::BlendFunction const & ) noexcept;

        void setParam ( std::uint8_t const &,
                        std::shared_ptr< IColor > const & );
    };
} // namespace io::console::colors