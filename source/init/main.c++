/**
 * @file main.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Entry point for our program.
 * @version 1
 * @date 2022-01-24
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#include <iostream>

#ifdef UNITTEST
#include <test/unittester.h++>
#endif

#ifdef WINDOWS
#include "windows.h"
#endif

void dumpInformation ( int const & , char const *const *const & );


int main ( int const argc , char const *const *const argv )
{
#ifdef WINDOWS
    SetConsoleOutputCP ( 65001 );
#endif
    dumpInformation ( argc , argv );
    
#ifdef UNITTEST
    test::runUnittests ( std::cout );
#endif
    return 0;
}


void dumpInformation ( int const &argc , char const *const *const &argv )
{
#ifdef WINODWS
    std::cout << "Compiled for Windows.\n";
#else
#ifdef LINUX
    std::cout << "Compiled for Linux.\n";
#else
    std::cout << "Compiled for an unknown target.\n";
#endif // ifdef LINUX
#endif // ifdef WINDOWS
    std::cout << "Received the following string from the command line: \n";
    for ( int i = 0; i < argc; i++ )
    {
        std::cout << argv [ i ] << " ";
    }
    std::cout << std::endl;
}

