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
    static std::vector < std::string > expected =
    {
        "A",                // 1-byte UTF-8
        "\u001bN@",         // SS2
        "\u001b[37m",       // CSI (set white foreground)
        "\u001b[38:1m",     // CSI (set transparent foreground)
        "\u001b]P1ff000000\u001b\\", // Linux command to set color 1 to bright red
        "\u001b]4;1;rgb:ff/00/00\u001b\\" , // Equivalent windows command
        "\u001bXHello, World!\u001b[37m\u001b\\", // valid SOS sequence
        "\u001bXHello, World!\u001b[37m\u001b\u001bX" , // *technically* valid SOS sequence
        "©", // copyright sign, (2-byte UTF-8)
        "€", // euro sign, (3-byte UTF-8)
        "כּ", // Hebrew Letter Kaf with Dagesh (4-byte UTF-8)
        "\U0010FFFF" , // final character in Supplementary Private Use Area B
    };

    std::string test = "A\u001bN@\u001b[37m\u001b[38:1m\u001b]P1ff000000\u001b\\"
        "\u001b]4;1;rgb:ff/00/00\u001b\\\u001bXHello, World!\u001b[37m\u001b\\"
        "\u001bXHello, World!\u001b[37m\u001b\\\u001b"
        "X©€כּ\U0010FFFF";

    std::vector < std::string > result = io::console::manip::splitByCodePoint ( test );
    bool pass = true;
    try
    {
        for ( std::size_t i = 0; i < result.size ( ); i++ )
        {
            if ( result.at ( i ) != expected.at ( i ) )
            {
                pass = false;
                std::cout << "Failed since item " << i + 1 << " does not match!\n";
                std::cout << "Offending sequence is: " << result.at ( i ) << "\n";
            }
        }
    } catch ( ... )
    {
        pass = false;
        std::cout << "Failed because an exception was thrown.\n";
    }

    return pass;
}