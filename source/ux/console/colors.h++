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
#include <defines/manip.h++>
#include <defines/types.h++>

#include <io/console/colors/color.h++>
#include <io/console/console.h++>

#include <yaml-cpp/yaml.h>

#include <memory>

namespace ux::console
{
    enum class SerializedColorFunction : std::uint8_t
    {
        WAVEFORM,
        AVERAGE4,
        AVERAGE5,
        _MAX,
    };

    template < class CharT, class Traits = std::char_traits< CharT > >
    inline std::basic_istream< CharT, Traits > &
            operator>> ( std::basic_istream< CharT, Traits > &is,
                         SerializedColorFunction             &scf )
    {
        std::basic_string< CharT, Traits > temp;
        is >> temp;
        scf = defines::fromString< SerializedColorFunction > ( temp );
        return is;
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

        ColorBlock ( YAML::Node const & );

        void addColor ( std::size_t const &,
                        std::shared_ptr< io::console::colors::IColor > const
                                & ) noexcept;

        void delColor ( std::size_t const & ) noexcept;

        std::shared_ptr< io::console::colors::IColor > const
                getColor ( std::size_t const & ) noexcept;

        void setColor ( std::size_t const &,
                        std::shared_ptr< io::console::colors::IColor > const
                                & ) noexcept;

        bool hasColor ( std::size_t const & ) noexcept;

        operator io::console::ConsoleManipulator ( ) const noexcept;
    };
} // namespace ux::console