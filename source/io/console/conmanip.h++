/**
 * @file conmanip.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Console manipulators.
 * @version 1
 * @date 2022-02-16
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <io/console/console.h++>

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace io::console
{
    ConsoleManipulator textDelay ( unsigned long long int const & );
    ConsoleManipulator commandDelay ( unsigned long long int const & );

    ConsoleManipulator setDirectColor ( std::size_t const &,
                                        defines::UnboundColor const &,
                                        defines::UnboundColor const &,
                                        defines::UnboundColor const &,
                                        defines::UnboundColor const &a = 0 );
    ConsoleManipulator setIndirectColor ( std::size_t const &,
                                          std::size_t const &,
                                          std::size_t const &,
                                          std::size_t const &,
                                          std::size_t const &,
                                          defines::UnboundColor const &r = 0,
                                          defines::UnboundColor const &g = 0,
                                          defines::UnboundColor const &b = 0,
                                          defines::UnboundColor const &a = 0 );

    ConsoleManipulator setBaseComponent ( std::size_t const &,
                                          defines::UnboundColor const &,
                                          defines::UnboundColor const &,
                                          defines::UnboundColor const &,
                                          defines::UnboundColor const &a = 0 );

    inline Console &doWaitForText ( Console &console )
    {
        console.setWaitOnText ( true );
        return console;
    }
    inline Console &noWaitForText ( Console &console )
    {
        console.setWaitOnText ( false );
        return console;
    }

    inline Console &doTextWrapping ( Console &console )
    {
        console.setWrapping ( true );
        return console;
    }

    inline Console &noTextWrapping ( Console &console )
    {
        console.setWrapping ( false );
        return console;
    }

    inline Console &doTextCenter ( Console &console )
    {
        console << doTextWrapping;
        console.setCentering ( true );
        return console;
    }

    inline Console &noTextCenter ( Console &console )
    {
        console.setCentering ( false );
        return console;
    }

    inline ConsoleManipulator doSGR ( SGRCommand const &cmd )
    {
        return [ & ] ( Console &console ) -> Console & {
            console.sgrCommand ( cmd, true );
            return console;
        };
    }

    inline ConsoleManipulator noSGR ( SGRCommand const &cmd )
    {
        return [ & ] ( Console &console ) -> Console & {
            console.sgrCommand ( cmd, false );
            return console;
        };
    }

    inline Console &resetSGR ( Console &console )
    {
        for ( std::size_t i = 0; i < std::size_t ( SGRCommand::_MAX ); i++ )
        {
            console << noSGR ( ( SGRCommand ) i );
        }
        return console;
    }

    inline ConsoleManipulator setForeground256 ( std::uint8_t const &color )
    {
        return [ & ] ( Console &console ) -> Console & {
            console.setForeground ( ( color << 8 ) + 9 );
            return console;
        };
    }
    inline ConsoleManipulator setBackground256 ( std::uint8_t const &color )
    {
        return [ & ] ( Console &console ) -> Console & {
            console.setBackground ( ( color << 8 ) + 9 );
            return console;
        };
    }

    inline ConsoleManipulator setForegroundTrue ( std::uint8_t const &r,
                                                  std::uint8_t const &g,
                                                  std::uint8_t const &b )
    {
        return [ & ] ( Console &console ) -> Console & {
            console.setForeground ( ( r << 24 ) + ( g << 16 ) + ( b << 8 )
                                    + 10 );
            return console;
        };
    }

    inline ConsoleManipulator setBackgroundTrue ( std::uint8_t const &r,
                                                  std::uint8_t const &g,
                                                  std::uint8_t const &b )
    {
        return [ & ] ( Console &console ) -> Console & {
            console.setBackground ( ( r << 24 ) + ( g << 16 ) + ( b << 8 )
                                    + 10 );
            return console;
        };
    }
} // namespace io::console