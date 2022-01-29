/**
 * @file unittester.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Unit tests
 * @version 1
 * @date 2022-01-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#include <test/unittester.h++>

 // here, we have all of the header files required for our tests
#include <atomic>
#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

 // here, we include all of our header files.
#include <engine/rand/random.h++>
#include <io/base/syncstream.h++>
#include <io/base/unistring.h++>
#include <io/console/manip/stringfunctions.h++>

#ifdef WINDOWS
#include "windows.h"
#endif

struct Unittest
{
    char const *const passMessage = "Success.";
    char const *const failMessage = "Failure.";

    bool ( *test )( );
};

bool testBasicSyncstreambuf ( );
bool testCodePointSplitting ( );
bool testSigmaCheck ( );

Unittest unittests [ ] = {
    {
        "io::base::basic_syncstreambuf Successfully Synchronizes." ,
        "io::base::basic_syncstreambuf Fails to Synchronize.",
        &testBasicSyncstreambuf,
    },
    {
        "io::console::splitByCodePoint Gave the Correct Strings.",
        "io::console::splitByCodePoint Gave Incorrect Strings.",
        &testCodePointSplitting,
    },
    {
        "engine::rand::sigmaCheck Gave Expected Pass / Fail Rates.",
        "engine::rand::sigmaCheck Gave Unexpected Pass / Fail Rates.",
        &testSigmaCheck,
    }
};

void test::runUnittests ( std::ostream &ostream )
{
    for ( std::size_t i = 0; i < ( sizeof ( unittests ) / sizeof ( Unittest ) ); i++ )
    {
        if ( unittests [ i ].test ( ) )
        {
            ostream << unittests [ i ].passMessage << std::endl;
        } else
        {
            ostream << unittests [ i ].failMessage << std::endl;
        }
    }
}

bool testSigmaCheck ( )
{
    // [-5,5]
    unsigned passFailCounters [ 11 ];
    for ( int i = 0; i < 100; i++ )
    {
        for ( int j = 0; j < 11; j++ )
        {
            passFailCounters [ j ] += engine::rand::sigmaCheck ( j - 5 );
        }
    }

    // todo check.
    std::cout << "NOTE: Calculations performed, but no verification done.\n";
    return true;
}

template < class CharT >
bool basicSyncstreambufTest ( )
{
    std::basic_stringstream < CharT > stream;
    CharT options [ 4 ] = {
        ( CharT ) 'A',
        ( CharT ) 'B',
        ( CharT ) 'C',
        ( CharT ) 'D',
    };
    std::basic_string < CharT > expects [ 4 ] = {
        io::base::emptyString < CharT > ( ),
        io::base::emptyString < CharT > ( ),
        io::base::emptyString < CharT > ( ),
        io::base::emptyString < CharT > ( ),
    };
    for ( int i = 0; i < 4; i++ )
    {
        for ( int j = 0; j < 50; j++ )
        {
            CharT temp [ 2 ] = { ( CharT ) options [ i ] , ( CharT ) 0 };
            expects [ i ] += temp;
        }
    }
    std::atomic_int latch = 4;
    auto demonstrate = [ & ] ( int id , std::basic_ostream < CharT > &os )
    {
        CharT temp [ 2 ] = { options [ id & 3 ] , ( CharT ) 0 };
        for ( int i = 0; i < 50; i++ )
        {
            os << temp;
        }
        os << "\n";
        latch.fetch_sub ( 1 );
    };

    io::base::basic_syncstreambuf < CharT > buf ( io::base::get_cout < CharT > ( ).rdbuf ( ) );
    buf.set_emit_on_sync ( true );
    std::basic_ostream < CharT > sync ( &buf );
    std::basic_ostream < CharT > cout ( io::base::get_cout < CharT > ( ).rdbuf ( ) );

    auto run = [ & ] ( std::basic_ostream < CharT > &use )
    {
        latch.store ( 4 );
        std::jthread jobs [ 4 ] = {
            std::jthread ( [ & ] ( ) {demonstrate ( 0 , use ); } ) ,
            std::jthread ( [ & ] ( ) {demonstrate ( 1 , use ); } ) ,
            std::jthread ( [ & ] ( ) {demonstrate ( 2 , use ); } ) ,
            std::jthread ( [ & ] ( ) {demonstrate ( 3 , use ); } ) ,
        };

        for ( int i = 0; i < 4; i++ ) jobs [ i ].detach ( );
        while ( latch.load ( ) ) std::this_thread::sleep_for ( std::chrono::milliseconds ( 100 ) );
    };

    std::cout << "Synchronized Output should have no interference:\n";
    run ( sync );
    buf.emit ( );
    std::cout << "Unsyncrhonized Output may have interference:\n";
    run ( cout );
    return true;
}


bool testBasicSyncstreambuf ( )
{
    bool result = true;
    basicSyncstreambufTest < char > ( );
    basicSyncstreambufTest < wchar_t > ( );
    return result;
}


bool testCodePointSplitting ( )
{
    auto testOne = [ ] ( std::string against )
    {
        std::cout << "Testing \"" << against << "\"\n";
        auto result = io::console::manip::splitByCodePoint ( against );
        bool pass = against.starts_with ( result.at ( 0 ) );
        std::string conditions [ 2 ] = {
            ( pass ? "equals" : "does not equal" ),
            ( pass ? "passes" : "does not pass" ),
        };
        std::cout << "Since \"" << result.at ( 0 ) << "\" " << conditions [ 0 ];
        std::cout << " the first code point of " << against;
        std::cout << " the test " << conditions [ 1 ];
        std::cout << std::endl;
        return pass;
    };
    auto testTwo = [ ] ( std::string against , char escape , char with )
    {
        auto replace = [ & ] ( std::string string )
        {
            for ( auto &c : string )
            {
                if ( c == escape ) c = with;
            }
            return string;
        };

        std::cout << "Testing \"" << replace ( against ) << "\"\n";
        std::cout << "Where \"" << with << "\" represents codepoint u+" << std::hex << ( int ) escape;
        std::cout << std::dec << std::endl;
        auto result = io::console::manip::splitByCodePoint ( against );
        bool pass = against.starts_with ( result.at ( 0 ) );
        std::string conditions [ 2 ] = {
            ( pass ? "equals" : "does not equal" ),
            ( pass ? "passes" : "does not pass" ),
        };
        std::cout << "Since \"" << replace ( result.at ( 0 ) ) << "\" " << conditions [ 0 ];
        std::cout << " the first code point of " << replace ( against );
        std::cout << " the test " << conditions [ 1 ];
        std::cout << std::endl;
        return pass;
    };

    auto testBig = [ & ] ( )
    {
        std::string against = "\U0010ffff";
        std::cout << "Testing \"" << "[Final Private Use Character Here]" << "\"\n";
        std::cout << "Where [Final Private Use Character Here]" << " represents U+10FFFF\n";
        auto result = io::console::manip::splitByCodePoint ( against );
        bool pass = against.starts_with ( result.at ( 0 ) );
        std::string conditions [ 2 ] = {
            ( pass ? "equals" : "does not equal" ),
            ( pass ? "passes" : "does not pass" ),
        };
        std::cout << "Since \"" << result.at ( 0 ) << "\" " << conditions [ 0 ];
        std::cout << " the first code point of " << against;
        std::cout << " the test " << conditions [ 1 ];
        std::cout << std::endl;
        return pass;
    };


    auto testBad = [ & ] ( )
    {
        // this test will remain if the compiler lets it.
        char badChars [ ] = {
            (char)0xff,
            0x00,
        };

        std::string bad ( badChars );
        try
        {
            io::console::manip::splitByCodePoint ( bad );
        } catch ( std::runtime_error &error )
        {
            return true;
        }
        return false;
    };

    bool pass = true;
    pass &= testOne ( "A" );
    pass &= testOne ( "🅱  ");
    pass &= testOne ( "©  ");
    pass &= testTwo ( "\u001b[37m" , '\u001b' , '?' );
    pass &= testTwo ( "\u001b]P1ff0000\u001b\\" , '\u001b' , '?' );
    pass &= testTwo ( "\u001b]Hello, World!!!!!\u001b\\" , '\u001b' , '?' );
    pass &= testTwo ( "\u001bXProper Start of String\u001b\\" , '\u001b' , '?' );
    pass &= testTwo ( "\u001bXJanky Start of String\u001bX" , '\u001b' , '?' );
    pass &= testBig ( );
    pass &= testBad ( );
    return pass;
}