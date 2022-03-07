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

#include <ux/serialization/screens.h++>
#include <ux/serialization/strings.h++>

#include <ux/console/screen.h++>

#include <any>
#include <filesystem>
#include <iostream>
#include <string>

void dumpInformation ( int const &, char const *const *const & );

int main ( int const argc, char const *const *const argv )
{
    std::filesystem::path dataPath { argv [ 0 ] };
    dataPath = dataPath.parent_path ( ); // get the outer directory
    dataPath /= defines::dataFolderName;

    std::filesystem::path textPath   = dataPath / defines::textFolderName;
    std::filesystem::path screenPath = dataPath / defines::screenFolderName;

    ux::serialization::TransliterationLevel translit =
            ux::serialization::TransliterationLevel::NOT;
    defines::IString locale = "en-US";

    std::shared_ptr< ux::serialization::ExternalizedStrings > strings =
            std::shared_ptr< ux::serialization::ExternalizedStrings > (
                    new ux::serialization::ExternalizedStrings ( ) );
    std::shared_ptr< ux::serialization::ExternalizedScreens > screens =
            std::shared_ptr< ux::serialization::ExternalizedScreens > (
                    new ux::serialization::ExternalizedScreens ( ) );
    strings->parse ( textPath );
    screens->parse ( screenPath );
    // auto getString = [ & ] ( defines::IString const &key ) ->
    // defines::IString {
    //     using ux::serialization::ExternalID;
    //     return strings->get ( std::shared_ptr< ExternalID > ( new ExternalID
    //     (
    //             locale + "." + key + "."
    //             + defines::rtToString<
    //                     ux::serialization::TransliterationLevel > (
    //                     translit ) ) ) );
    // };

    auto getScreen =
            [ & ] ( defines::IString const &key ) -> ux::console::Screen {
        using ux::serialization::ExternalID;
        return screens->get (
                std::shared_ptr< ExternalID > ( new ExternalID ( key ) ) );
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

    defines::IString currentScreenName = "Title";

    // TODO #53 should get implemented here.
    auto chooseNext = [ & ] ( ux::console::Screen const &current ) {
        // screen choosing logic, potentially moved eventually
        // to Screen as a member function.
        //
        // also eventually fleshed out into more than choosing the first
        // option if available.
        if ( current == getScreen ( "Exit" ) )
        {
            std::exit ( 0 );
        } else if ( current.nextScreen.empty ( ) )
        {
            currentScreenName = "Exit";
            // exit screen.
            return getScreen ( "Exit" );
        } else
        {
            currentScreenName = current.nextScreen.front ( ).key;
            return getScreen ( current.nextScreen.front ( ).key );
        }
    };

    // first and last name as grabbed from the name input.
    defines::IString firstName;
    defines::IString lastName;

    for ( ux::console::Screen screen = getScreen ( "Title" );;
          screen                     = chooseNext ( screen ) )
    {
        std::cin.clear ( );
        con << screen.output ( *strings, locale, translit );
        // check if the screen is the part which asks for the first name and
        // last name of the hypothetical character.
        if ( currentScreenName == "CharacterCreationPart1" )
        {
            while ( !screen.inputPrompt.inputReady ) { }
            defines::ChrString *temp = nullptr;
            try
            {
                temp = std::any_cast< defines::ChrString * > (
                        screen.inputPrompt.result );
            } catch ( std::bad_any_cast &bac )
            {
                io::base::osyncstream { std::cout }
                        << "Failed to read in the name. Despite what the "
                           "result "
                           "types say, we see the input as a "
                        << screen.inputPrompt.result.type ( ).name ( ) << "\n";
            }

            if ( temp )
            {
                con << "Read in the data " << temp [ 0 ] << " and "
                    << temp [ 1 ] << "\n";
            }
        }
    }

    std::cin.get ( );
    // set up some (hopefully) flashing text
    // con << setDirectColor ( 8, 1, 1, 1 );
    // con << setDirectColor ( 9, 0x80, 0x80, 0x80, 0x80 );
    // con << setDirectColor ( 10, 0, 0, 0 );
    // con << setIndirectColor ( 1, 9, 8, 10, 10, 0x7F, 0x7F, 0x7F );
    // con << setBaseComponent ( 2, 0xC0, 0xFF, 0xEE );
    // con << commandDelay ( 100 );
    // con << doWaitForText << doTextCenter << getString ( "Title" )
    //    << noTextCenter << noTextWrapping << "\n";
    // con << "\u001b[31m" << doTextCenter << getString ( "StartMessage" )
    //    << noTextCenter << noTextWrapping << "\n"
    //    << noWaitForText;
    // std::cin.get ( );
    // con << "\u001b[39;49m\u001b[3J\u001b[2J\u001b[H";
    // con << doTextWrapping;
    // con << setDirectColor ( 8, 2, 2, 2 );
    // con << doWaitForText << getString ( "Introduction0" ) << "\n";
    // con << getString ( "Introduction1" ) << "\n";
    // con << getString ( "Introduction2" ) << "\n";
    // con << "\u001b[31m" << getString ( "ContinueMessage" ) << "\u001b[39m";
    // std::cin.get ( );
    // con << getString ( "CharacterCreation0" ) << "\n";
    // std::string temp;
    // std::getline ( std::cin, temp );
    // con << getString ( "CharacterCreation1" ) << temp
    //    << getString ( "CharacterCreation2" ) << "\n";
    // defines::IString exampleAttributes [] = {
    //        getString ( "AttributeName0" ),
    //        getString ( "AttributeName1" ),
    //        getString ( "AttributeName2" ),
    //        getString ( "AttributeName3" ),
    //        getString ( "AttributeName4" ),
    //        getString ( "AttributeName5" ),
    //        getString ( "AttributeName6" ),
    //};
    //
    // for ( int i = 0; i < 3; i++ )
    //{
    //    con << getString ( "AttributeSelectPrompt" ) << "\n";
    //    for ( unsigned j = 0;
    //          j < sizeof ( exampleAttributes ) / sizeof ( std::string );
    //          j++ )
    //    {
    //        con << "\t" << j + 1 << exampleAttributes [ j ] << "\n";
    //    }
    //    std::string line;
    //    std::getline ( std::cin, line );
    //    unsigned temp  = 0;
    //    // check to see if line contains any digits
    //    char digits [] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    //    for ( int j = 0; j < 10; j++ )
    //    {
    //        if ( line.find ( digits [ j ] ) != std::string::npos )
    //        {
    //            std::stringstream { line } >> temp;
    //            break;
    //        }
    //    }
    //    temp %= 7;
    //    con << getString ( "AttributeSelectConfirm0" );
    //    con << exampleAttributes [ temp ];
    //    con << getString ( "AttributeSelectConfirm1" );
    //    std::string _temp;
    //    std::getline ( std::cin, _temp );
    //}
    //
    // con << getString ( "Introduction5" ) << "\n";
    // con << getString ( "Introduction6" ) << "\n";
    // std::cin.get ( );
    //// cute little easter-egg in that it's the color "Coffee" (even though it
    //// looks minty)
    // con << "\u001b[32m" << getString ( "ExitMessage" ) << "\n";
    // std::cin.get ( );
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
