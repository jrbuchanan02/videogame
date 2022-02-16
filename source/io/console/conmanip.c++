/**
 * @file conmanip.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implementation of the functions in conmanip.h++
 * @version 1
 * @date 2022-02-16
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <io/console/conmanip.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <io/console/colors/color.h++>
#include <io/console/colors/direct.h++>
#include <io/console/colors/indirect.h++>
#include <io/console/console.h++>

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>

io::console::ConsoleManipulator
        io::console::textDelay ( unsigned long long int const &milliseconds )
{
    return [ = ] ( Console &console ) -> Console & {
        console.setTxtRate ( milliseconds );
        return console;
    };
}

io::console::ConsoleManipulator
        io::console::commandDelay ( unsigned long long int const &milliseconds )
{
    return [ = ] ( Console &console ) -> Console & {
        console.setCmdRate ( milliseconds );
        return console;
    };
}

/**
 * @brief Allows modification of a console color.
 *
 * @param at if at < 8, then modify the corresponding screen color, otherwise
 * modify the corresponding calculation color
 * @param red the red value
 * @param green the green value
 * @param blue the blue value
 * @param alpha the alpha. Currently, alpha is unused.
 * @return io::console::ConsoleManipulator
 */
io::console::ConsoleManipulator
        io::console::setDirectColor ( std::size_t const           &at,
                                      defines::UnboundColor const &red,
                                      defines::UnboundColor const &green,
                                      defines::UnboundColor const &blue,
                                      defines::UnboundColor const &alpha )
{
    return [ = ] ( Console &console ) -> Console & {
        if ( at < 8 )
        {
            console.setScreenColor (
                    at,
                    std::shared_ptr< colors::RGBAColor > (
                            new colors::RGBAColor ( red,
                                                    green,
                                                    blue,
                                                    alpha ) ) );

        } else
        {
            console.setCalculationColor (
                    at,
                    std::shared_ptr< colors::RGBAColor > (
                            new colors::RGBAColor ( red,
                                                    green,
                                                    blue,
                                                    alpha ) ) );
        }

        return console;
    };
}

io::console::ConsoleManipulator
        io::console::setIndirectColor ( std::size_t const &at,
                                        std::size_t const &amplitude,
                                        std::size_t const &frequency,
                                        std::size_t const &frequencyModulation,
                                        std::size_t const &amplitudeModulation,
                                        defines::UnboundColor const &r,
                                        defines::UnboundColor const &g,
                                        defines::UnboundColor const &b,
                                        defines::UnboundColor const &a )
{
    return [ = ] ( Console &console ) -> Console & {
        auto get = [ & ] ( std::size_t const &index )
                -> std::shared_ptr< colors::IColor > {
            if ( index < 8 )
            {
                return console.getScreenColor ( index );
            } else
            {
                return console.getCalculationColor ( index );
            }
        };

        std::shared_ptr< colors::IColor > ap = get ( amplitude );
        std::shared_ptr< colors::IColor > fq = get ( frequency );
        std::shared_ptr< colors::IColor > fm = get ( frequencyModulation );
        std::shared_ptr< colors::IColor > am = get ( amplitudeModulation );

        auto newColor = std::shared_ptr< colors::IndirectColor > (
                new colors::IndirectColor ( r, g, b, a, ap, fq, fm, am ) );
        if ( at < 8 )
        {
            console.setScreenColor ( at, newColor );
        } else
        {
            console.setCalculationColor ( at, newColor );
        }
        return console;
    };
}
io::console::ConsoleManipulator
        io::console::setBaseComponent ( std::size_t const           &at,
                                        defines::UnboundColor const &r,
                                        defines::UnboundColor const &g,
                                        defines::UnboundColor const &b,
                                        defines::UnboundColor const &a )
{
    return [ = ] ( Console &console ) -> Console & {
        std::shared_ptr< colors::IColor > color;
        if ( at < 8 )
        {
            color = console.getScreenColor ( at );
        } else
        {
            color = console.getCalculationColor ( at );
        }
        color->setBasicComponent ( 0, r );
        color->setBasicComponent ( 1, g );
        color->setBasicComponent ( 2, b );
        color->setBasicComponent ( 3, a );
        return console;
    };
}