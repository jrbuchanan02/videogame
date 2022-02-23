/**
 * @file colors.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A block of colors.
 * @version 1
 * @date 2022-02-23
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <io/console/colors/color.h++>
#include <io/console/console.h++>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include <memory>

namespace ux::console
{
    enum class SerializedColorFunction : std::uint8_t
    {
        WAVEFORM,
        AVERAGE4,
        AVERAGE5,
    };

    constexpr inline defines::ChrCString const
            toString ( SerializedColorFunction const scf ) noexcept
    {
        using enum SerializedColorFunction;
        switch ( scf )
        {
            case WAVEFORM: return "WAVEFORM";
            case AVERAGE4: return "AVERAGE4";
            case AVERAGE5: return "AVERAGE5";
        }
    }

    template < class CharT, class Traits = std::char_traits< CharT > >
    inline std::basic_istream< CharT, Traits > &
            operator>> ( std::basic_istream< CharT, Traits > &is,
                         SerializedColorFunction             &scf )
    {
        std::basic_string< CharT, Traits > temp;
        is >> temp;
        // string compare on strings of not-necessarily the same size.
        // we only need to check up to eight values, since all lengths
        // in SerializedColorFunction are eight.
        if ( temp.size ( ) < 8 || temp.size ( ) > 8 )
        {
            return is;
        } else
        {
            using enum SerializedColorFunction;
            bool waveformEligible = true;
            bool average4Eligible = true;
            bool average5Eligible = true;
            for ( std::size_t i = 0; i < 8; i++ )
            {
                waveformEligible &=
                        temp.at ( i ) == toString ( WAVEFORM ) [ i ];
                average4Eligible &=
                        temp.at ( j ) == toString ( AVERAGE4 ) [ i ];
                average5Eligible &=
                        temp.at ( j ) == toString ( AVERAGE5 ) [ i ];
            }

            if ( waveformEligible )
            {
                scf = WAVEFORM;
            } else if ( average4Eligible )
            {
                scf = AVERAGE4;
            } else if ( average5Eligible )
            {
                scf = AVERAGE5;
            }

            return is;
        }
    }

    class ColorBlock
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        ColorBlock ( ) noexcept;
        ColorBlock ( ColorBlock const & ) noexcept;
        ColorBlock ( ColorBlock && ) noexcept;
        virtual ~ColorBlock ( );
        ColorBlock &operator= ( ColorBlock const & ) noexcept;
        ColorBlock &operator= ( ColorBlock && ) noexcept;

        ColorBlock ( rapidjson::Value::Object const & );

        void addColor ( std::size_t const &,
                        std::shared_ptr< io::console::colors::IColor > const
                                & ) noexcept;

        void delColor ( std::size_t const & ) noexcept;

        std::shared_ptr< io::console::colors::IColor > const &
                getColor ( std::size_t const & ) noexcept;

        void setColor ( std::size_t const &,
                        std::shared_ptr< io::console::colors::IColor > const
                                & ) noexcept;

        bool hasColor ( std::size_t const & ) noexcept;

        operator io::console::ConsoleManipulator ( ) const noexcept;
    };
} // namespace ux::console