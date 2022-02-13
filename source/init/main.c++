/**
 * @file main.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Entry point for our program.
 * @version 1
 * @date 2022-01-24
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <io/base/syncstream.h++>
#include <io/unicode/character.h++>
#include <test/unittester.h++>

#include <iostream>

#ifdef WINDOWS
#    include "windows.h"
#endif

void dumpInformation ( int const &, char const *const *const & );

int main ( int const argc, char const *const *const argv )
{
#ifdef WINDOWS
    SetConsoleOutputCP ( 65001 );
#endif
    dumpInformation ( argc, argv );
    // todo: move to a unittest.
    std::cout << "According to the properties, 🅱 takes up ";
    auto columns = io::unicode::characterProperties ( ).at ( U'🅱' ).columns;
    std::cout << ( 1 + columns ) << " columns.\n";

    test::runUnittests ( std::cout );
    // if we're on an actual terminal, wait for user input to exit.
    std::cin.get ( );
    return 0;
}

void dumpInformation ( int const &argc, char const *const *const &argv )
{
#ifdef WINDOWS
    std::cout << "Compiled for Windows.\n";
#else
#    ifdef LINUX
    std::cout << "Compiled for Linux.\n";
#    else
    std::cout << "Compiled for an unknown target.\n";
#    endif // ifdef LINUX
#endif     // ifdef WINDOWS
    std::cout << "There is/are " << argc << " entry/entries in argv.\n";
    std::cout << "Received the following string from the command line: \n";
    for ( int i = 0; i < argc; i++ )
    {
        std::cout << "\"" << argv [ i ] << "\" ";
    }
    std::cout << std::endl;
}
