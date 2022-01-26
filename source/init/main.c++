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

#include <io/base/syncstream.h++>

#include <io/console/manip/stringfunctions.h++>

#ifdef WINDOWS
#include "windows.h"
#endif

io::base::basic_syncstreambuf < char > testConstruction;
io::base::basic_osyncstream < char > constructOsyncstream ( &testConstruction );

void dumpInformation ( int const & , char const *const *const & );
void testBasicSyncstreambuf ( );
void testStringFunctions ( );


int main ( int const argc , char const *const *const argv )
{
#ifdef WINDOWS
    SetConsoleOutputCP ( 65001 );
#endif
    dumpInformation ( argc , argv );
    testStringFunctions ( );
    io::base::basic_syncstreambuf < char > syncout ( std::cout.rdbuf ( ) );
    syncout.set_emit_on_sync ( true );
    testBasicSyncstreambuf ( );
    std::cout << "Testing complete. Press enter to exit.\n";
    std::cin.get ( );
    // FIXME: #2 Program Cannot Exit Noramlly on Linux
#ifdef LINUX
    throw std::runtime_error ( "I don't know why I can't get the program to exit normally." );
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

void testStringFunctions ( )
{
    static std::string testString = "A🅱©";

    std::vector < std::string > result = io::console::manip::splitByCodePoint ( testString );
    if ( result.at ( 0 ) != "A" )
    {
        std::cout << "Incorrect result: got ";
        std::cout << "\"" << result.at ( 0 ) << "\" instead of ";
        std::cout << "\"" << "A" << "\"";
        std::cout << std::endl;
    } else {
        std::cout << "Correct: \"" << result.at ( 0 ) << "\"";
        std::cout << std::endl;
    }

    if ( result.at ( 1 ) != "🅱" )
    {
        std::cout << "Incorrect result: got ";
        std::cout << "\"" << result.at ( 1 ) << "\" instead of ";
        std::cout << "\"" << "🅱" << "\"";
        std::cout << std::endl;
    } else {
        std::cout << "Correct: \"" << result.at ( 1 ) << "\"";
        std::cout << std::endl;
    }

    if ( result.at ( 2 ) != "©" )
    {
        std::cout << "Incorrect result: got ";
        std::cout << "\"" << result.at ( 2 ) << "\" instead of ";
        std::cout << "\"" << "©" << "\"";
        std::cout << std::endl;
    } else {
        std::cout << "Correct: \"" << result.at ( 2 ) << "\"";
        std::cout << std::endl;
    }
}

#include <atomic>
#include <thread>

// FIXME: the multithreading here prevents posix systems from exiting normally?
void testBasicSyncstreambuf ( )
{
    std::atomic_int counter = 4;
    auto sendFunction = [ & ] ( int id , std::ostream &os )
    {
        std::string chosen;
        switch ( id )
        {
        case 0:
            chosen = "░";
            break;
        case 1:
            chosen = "▒";
            break;
        case 2:
            chosen = "▓";
            break;
        case 3:
        default:
            chosen = " ";
        }

        for ( int i = 0; i < 50; i++ )
        {
            os << chosen;
        }
        os << "\n";
        counter.fetch_sub ( 1 );
    };

    io::base::syncbuf buffer1 ( std::cout.rdbuf ( ) );
    io::base::syncbuf buffer2 ( std::cout.rdbuf ( ) );

    std::ostream o1 ( &buffer1 );
    std::ostream o2 ( &buffer1 );
    std::ostream o3 ( &buffer2 );
    std::ostream o4 ( &buffer2 );

    std::thread t1 ( [ & ] ( ) {sendFunction ( 0 , o1 );} );
    std::thread t2 ( [ & ] ( ) {sendFunction ( 1 , o2 );} );
    std::thread t3 ( [ & ] ( ) {sendFunction ( 2 , o3 );} );
    std::thread t4 ( [ & ] ( ) {sendFunction ( 3 , o4 );} );

    t1.detach ( );
    t2.detach ( );
    t3.detach ( );
    t4.detach ( );

    while ( counter.load ( ) > 0 )
    {
        std::this_thread::sleep_for ( std::chrono::milliseconds ( 100 ) );
    }
}