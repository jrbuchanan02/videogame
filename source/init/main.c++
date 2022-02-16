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
#include <test/unittester.h++>

#include <io/console/console.h++>

#include <iostream>
#include <string>

#ifdef WINDOWS
#    include "windows.h"
#endif

void dumpInformation ( int const &, char const *const *const & );

int main ( int const argc, char const *const *const argv )
{
#ifdef WINDOWS
    SetConsoleOutputCP ( 65001 );
#endif
    for ( int i = 0; i < argc; i++ )
    {
        if ( std::string ( argv [ i ] ) == "--unittest" )
        {
            test::runUnittests ( std::cout );
            std::cin.get ( );
        } else if ( std::string ( argv [ i ] ) == "--dump-args" )
        {
            dumpInformation ( argc, argv );
            std::cin.get ( );
        }
    }
    using namespace io::console;
    Console con;
    con << "Videogame has exited. Press enter to close the window or return to "
           "the shell.\n";
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
