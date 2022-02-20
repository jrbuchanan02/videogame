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

#include <ux/serialization/strings.h++>

#include <filesystem>
#include <iostream>
#include <string>

void dumpInformation ( int const &, char const *const *const & );

int main ( int const argc, char const *const *const argv )
{
    std::filesystem::path dataPath { argv [ 0 ] };
    dataPath = dataPath.parent_path ( );
    dataPath /= "data";
    dataPath /= "text";

    ux::serialization::TransliterationLevel translit =
            ux::serialization::TransliterationLevel::NOT;
    defines::IString locale = "en-US";

    ux::serialization::ExternalizedStrings strings { dataPath };
    auto getString = [ & ] ( defines::IString const &key ) -> defines::IString {
        using ux::serialization::StringKey;
        return strings.get ( StringKey { locale, key, translit } );
    };
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
    con << doWaitForText << getString ( "title" ) << "\n";
    con << "\u001b[31m" << getString ( "start-message" ) << "\n"
        << noWaitForText;
    std::cin.get ( );
    con << "\u001b[39;49m\u001b[3J\u001b[2J\u001b[H";
    con << doTextWrapping;
    con << setDirectColor ( 8, 2, 2, 2 );
    con << doWaitForText << getString ( "introduction.0" ) << "\n";
    con << getString ( "introduction.1" ) << "\n";
    con << getString ( "introduction.2" ) << "\n";
    con << "\u001b[31m" << getString ( "continue-message" ) << "\u001b[39m";
    std::cin.get ( );
    con << getString ( "character-creation.0" ) << "\n";
    std::string temp;
    std::getline ( std::cin, temp );
    con << getString ( "character-creation.1" ) << temp
        << getString ( "character-creation.2" ) << "\n";
    defines::IString exampleAttributes [] = {
            getString ( "attribute-name.0" ),
            getString ( "attribute-name.1" ),
            getString ( "attribute-name.2" ),
            getString ( "attribute-name.3" ),
            getString ( "attribute-name.4" ),
            getString ( "attribute-name.5" ),
            getString ( "attribute-name.6" ),
    };

    for ( int i = 0; i < 3; i++ )
    {
        con << getString ( "attribute-select-prompt" ) << "\n";
        for ( unsigned j = 0;
              j < sizeof ( exampleAttributes ) / sizeof ( std::string );
              j++ )
        {
            con << "\t" << j + 1 << exampleAttributes [ j ] << "\n";
        }
        std::string line;
        std::getline ( std::cin, line );
        unsigned temp  = 0;
        // check to see if line contains any digits
        char digits [] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
        for ( int j = 0; j < 10; j++ )
        {
            if ( line.find ( digits [ j ] ) != std::string::npos )
            {
                std::stringstream { line } >> temp;
                break;
            }
        }
        temp %= 7;
        con << getString ( "attribute-select-confirm.0" );
        con << exampleAttributes [ temp ];
        con << getString ( "attribute-select-confirm.1" );
        std::string _temp;
        std::getline ( std::cin, _temp );
    }

    con << getString ( "introduction.5" ) << "\n";
    con << getString ( "introduction.6" ) << "\n";
    std::cin.get ( );
    // cute little easter-egg in that it's the color "Coffee" (even though it
    // looks minty)
    con << "\u001b[32m" << getString ( "exit-message" ) << "\n";
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
