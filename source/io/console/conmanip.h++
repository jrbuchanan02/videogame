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
} // namespace io::console