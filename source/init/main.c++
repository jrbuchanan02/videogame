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
        << "When video game receives a plot, this text will inform you about "
           "the inciting incident, and all of your tasks that you will have as "
           "the main character and hero of the story. Be prepared for text in "
           "multiple languages, sometimes without translation, and feel "
           "comfortable with the unexpected. Aunque el español es mi lengua "
           "segundo, lo siento para los errores. 日本語は私の三言語です, and I "
           "wouldn't really say I know Japanese yet, lol.\n";
    con << "Either way, be prepared for a wild ride of a story and an RNG "
           "filled adventure! I take my inspiration from Miguel Cervantes, "
           "author of Don Quixote, and Rumiko Takahashi, author of Ranma 1/2 "
           "and Inuyasha, but also my own weird dreams.";
    con << "\n\n";
    con << "\u001b[31m[Press enter to continue]\u001b[39m";
    std::cin.get ( );
    con << "\n\n";
    con << "At this point, you would enter the character creation process. The "
           "game would ask for your name, and have a line break as it shows "
           "you the prompt.\n";
    std::string temp;
    std::getline ( std::cin, temp );
    con << "Then the game would list out attributes that you can choose for "
           "your character. These attributes would give your character, " << temp << ", certain "
           "advantages and or disadvantages throughout the game.\n";
    std::string exampleAttributes [] = {
            "Attribute",
            "Attribute",
            "Attribute",
            "Attribute",
            "Attribute",
            "Attribute",
    };

    for ( int i = 0; i < 3; i++ )
    {
        con << "Please select attribute " << ( i + 1 ) << ":\n";
        for ( unsigned j = 0;
              j < sizeof ( exampleAttributes ) / sizeof ( std::string );
              j++ )
        {
            con << "\t" << j + 1 << exampleAttributes [ j ] << "\n";
        }
        std::string line;
        std::getline(std::cin, line);
        unsigned temp = 0;
        // check to see if line contains any digits
        char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        for ( int j = 0; j < 10; j++ )
        {
            if ( line.find(digits[j]) != std::string::npos)
            {
                std::stringstream { line } >> temp;
                break;
            }
        }
        con << "So you have chosen \""
            << exampleAttributes [ temp
                                   % ( sizeof ( exampleAttributes )
                                       / sizeof ( std::string ) ) ]
            << "\"\n";
        con << "Aquí el videojuego verificará su repuesta y cambiaría "
               "lenguas.\n";
    }

    con << "After the character has selected their attributes, the game will "
           "ask for the gender of the requested character. Part of the RNG is "
           "that the game may choose to randomize the gender and attributes of "
           "the character after creation.\n";
    con << "A reference to the anime ranma 1/2, The character may change forms "
           "when hit with certain temperatures of water, or, as a reference to "
           "a fever dream I had a while back, the character may become two "
           "people. Also, technically a reference still to Ranma 1/2, the "
           "videogame may lock the characters gender. It would not lock the "
           "characters form to a non human one, Because I plan on all "
           "humanoids having certain attributes, advantages and disadvantages, "
           "that the player should have.\n";
    con << "References to other video games, anime, manga, and pop culture "
           "should also exist. For example, a reference to the video game "
           "Fallout New Vegas might be cool.\n";
    con << "The goal with the RNG within the character creation is to create a "
           "unique experience each time somebody plays the game.\n";
    con << "Regardless, you have reached the end of the hypothetical character "
           "creation scene.There is no storyline yet, So what you just did was "
           "just symbolic, and the video game will exit as soon as you press "
           "enter.\n";
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
