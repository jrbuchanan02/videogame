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

#ifdef WINDOWS
#include "windows.h"
#endif

io::base::basic_syncstreambuf < char > testConstruction;
io::base::basic_osyncstream < char > constructOsyncstream ( &testConstruction );

void dumpInformation ( int const & , char const *const *const & );
void testBasicSyncstreambuf ( );

int main ( int const argc , char const *const *const argv )
{
    dumpInformation ( argc , argv );
    io::base::basic_syncstreambuf < char > syncout ( std::cout.rdbuf ( ) );
    syncout.set_emit_on_sync ( true );
    testBasicSyncstreambuf ( );
    std::cout << "Testing complete. Press enter to exit.\n";
    std::cin.get ( );
    // FIXME: #2 Program Cannot Exit Noramlly on Linux
#ifdef LINUX
    throw std::runtime_error ( "I don't know why I can't get the program to exit normally.");
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

#include <thread>

// FIXME: #1 occaisionally segfaults on linux
// FIXME: the multithreading here prevents posix systems from exiting normally?
void testBasicSyncstreambuf ( )
{
#ifdef WINDOWS
    SetConsoleOutputCP ( 65001 );
#endif
    auto sendFunction = [ & ] ( const char *chosen , std::ostream &ostream )
    {
        for ( int i = 0; i < 80; i++ )
        {
            ostream << chosen;
        }
        ostream << "\n";
    };
    static const char *letters [ ] = {
        "1",
        "2",
        " ",
        "_",
        "=",
        "\u2591\0",
        "\u2592\0",
        "\u2593\0",
    };
    std::jthread threads [ 16 ];
    for ( int i = 0; i < 4; i++ )
    {
        std::cout << "Iteration " << i + 1 << " of " << 4 << std::endl;
        io::base::basic_syncstreambuf < char > buffers [ ] = {
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
            io::base::basic_syncstreambuf ( std::cout.rdbuf ( ) ) ,
        };
        std::ostream streams [ 16 ] = {
            std::ostream ( &buffers [ 0 >> i ] ),
            std::ostream ( &buffers [ 1 >> i ] ),
            std::ostream ( &buffers [ 2 >> i ] ),
            std::ostream ( &buffers [ 3 >> i ] ),
            std::ostream ( &buffers [ 4 >> i ] ),
            std::ostream ( &buffers [ 5 >> i ] ),
            std::ostream ( &buffers [ 6 >> i ] ),
            std::ostream ( &buffers [ 7 >> i ] ),
            std::ostream ( &buffers [ 8 >> i ] ),
            std::ostream ( &buffers [ 9 >> i ] ),
            std::ostream ( &buffers [ 10 >> i ] ),
            std::ostream ( &buffers [ 11 >> i ] ),
            std::ostream ( &buffers [ 12 >> i ] ),
            std::ostream ( &buffers [ 13 >> i ] ),
            std::ostream ( &buffers [ 14 >> i ] ),
            std::ostream ( &buffers [ 15 >> i ] ),
        };
        for ( int j = 0; j < 16; j++ )
        {
            threads [ j ] = std::jthread ( [ = , &sendFunction , &streams ] ( ) { sendFunction ( letters [ j % ( sizeof ( letters ) / sizeof ( letters [ 0 ] ) ) ] , streams [ j ] ); } );
            threads [ j ].detach ( );
        }
        for ( int j = 0; j < 16; j++ )
        {
            if ( threads [ j ].joinable ( ) ) threads [ j ].join ( );
        }
    }
#ifdef LINUX
    for ( int j = 0; j < 16; j++ )
    {
        try {
            while ( threads[j].joinable() ) threads[j].join();
        } catch ( std::system_error &error )
        {
            // do nothing.
        }
    }
#endif

    std::cout << "The each iteration may have less than 16 lines since the stop signal is sent quickly after the threads are started.\n";
}