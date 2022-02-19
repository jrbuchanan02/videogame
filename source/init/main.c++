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

#include <io/console/conmanip.h++>
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
    bool runUnittests    = false;
    bool dumpInformation = false;
    for ( int i = 0; i < argc; i++ )
    {
        if ( std::string ( argv [ i ] ) == "--unittest" )
        {
            runUnittests = true;
        } else if ( std::string ( argv [ i ] ) == "--dump-information" )
        {
            dumpInformation = true;
        }
    }

    if ( runUnittests )
    {
        if ( test::runUnittests ( std::cout ) )
        {
            return 1;
        } else
        {
            return 0;
        }
    }

    if ( dumpInformation )
    {
        ::dumpInformation ( argc, argv );
        return 0;
    }

    using namespace io::console;
    Console con;
    // set up some (hopefully) flashing text
    con << setDirectColor ( 8, 1, 1, 1 );
    con << setDirectColor ( 9, 0x80, 0x80, 0x80, 0x80 );
    con << setDirectColor ( 10, 0, 0, 0 );
    con << setIndirectColor ( 1, 9, 8, 10, 10, 0x7F, 0x7F, 0x7F );
    con << setBaseComponent ( 2, 0xC0, 0xFF, 0xEE );
    con << commandDelay ( 100 );
    con << doWaitForText << "Videogame\n";
    con << "\u001b[31mPress enter to start!\n" << noWaitForText;
    std::cin.get ( );
    con << "\u001b[39;49m\u001b[3J\u001b[2J\u001b[H";
    con << doTextWrapping;
    con << setDirectColor ( 8, 2, 2, 2 );
    con << doWaitForText
        << "Placeholder storyline. Pretend that this text contains an "
           "intricate plot and interesting backstory about the imaginary, "
           "not-yet-existent world within Videogame, which does not even have "
           "a title yet. Learn about the final boss's evil-ness and the "
           "generic worldbuilding style assumed. Learn about the hero that "
           "will prevent the final boss from achieving their ultimate plan and "
           "the prophercy or something which fortells their arrival -- or "
           "whatever foreshadowing describes the player's introduction. Remain "
           "prepared for text in countless languages. "
           "グーグル翻訳とduolingo教育の私のひどい混合を許してください and "
           "enjoy the ride!";
    con << "\n\n";
    con << "\u001b[31m[Press enter to continue]\u001b[38m";
    std::cin.get ( );
    // cute little easter-egg in that it's the color "Coffee" (even though it
    // looks minty)
    con << "\u001b[32mVideogame has exited. Press enter to close the window or "
           "return to the shell.\n";
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
