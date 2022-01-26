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

// basic testing environment for both char and wchar_t
template < class CharT >
bool basicSyncstreambufTest ( )
{
    std::atomic_int counter = 4;
    std::basic_string < CharT > spaces;
    std::basic_string < CharT > lshade;
    std::basic_string < CharT > mshade;
    std::basic_string < CharT > hshade;

    std::basic_stringstream < CharT > _spaces;
    std::basic_stringstream < CharT > _lshade;
    std::basic_stringstream < CharT > _mshade;
    std::basic_stringstream < CharT > _hshade;

    for ( int i = 0; i < 50; i++ )
    {
        _spaces << " ";
        _lshade << "░";
        _mshade << "▒";
        _hshade << "▓";
    }
    spaces = _spaces.str();
    lshade = _lshade.str();
    mshade = _mshade.str();
    hshade = _hshade.str();

    auto sendFunction = [ & ] ( int id , std::basic_ostream < CharT > &os )
    {
        std::basic_stringstream < CharT > chosen;
        switch ( id )
        {
        case 0:
            chosen << "░";
            break;
        case 1:
            chosen << "▒";
            break;
        case 2:
            chosen << "▓";
            break;
        case 3:
        default:
            chosen << " ";
        }

        for ( int i = 0; i < 50; i++ )
        {
            os << chosen.str ( );
        }
        os << "\n";
        counter.fetch_sub ( 1 );
    };
    std::basic_stringstream < CharT > cout;

    io::base::basic_syncstreambuf < CharT > buffer1 ( cout.rdbuf ( ) );
    io::base::basic_syncstreambuf < CharT > buffer2 ( cout.rdbuf ( ) );

    std::basic_ostream < CharT > o1 ( &buffer1 );
    std::basic_ostream < CharT > o2 ( &buffer1 );
    std::basic_ostream < CharT > o3 ( &buffer2 );
    std::basic_ostream < CharT > o4 ( &buffer2 );

    std::thread t1 ( [ & ] ( ) {sendFunction ( 0 , o1 ); } );
    std::thread t2 ( [ & ] ( ) {sendFunction ( 1 , o2 ); } );
    std::thread t3 ( [ & ] ( ) {sendFunction ( 2 , o3 ); } );
    std::thread t4 ( [ & ] ( ) {sendFunction ( 3 , o4 ); } );

    t1.detach ( );
    t2.detach ( );
    t3.detach ( );
    t4.detach ( );

    while ( counter.load ( ) > 0 )
    {
        std::this_thread::sleep_for ( std::chrono::milliseconds ( 100 ) );
    }

    std::basic_string < CharT > string = cout.str ( );
    if ( !string.find ( spaces.c_str ( ) ) )
    {
        std::cout << "The channel sending spaces was interrupted for CharT = ";
        std::cout << ( sizeof ( CharT ) == sizeof ( char ) ? "char" : "wchar_t" );
        std::cout << std::endl;
        return false;
    }
    if ( !string.find ( lshade.c_str ( ) ) )
    {
        std::cout << "The channel sending light shade was interrupted for CharT = ";
        std::cout << ( sizeof ( CharT ) == sizeof ( char ) ? "char" : "wchar_t" );
        std::cout << std::endl;
        return false;
    }
    if ( !string.find ( mshade.c_str ( ) ) )
    {
        std::cout << "The channel sending medium shade was interrupted for CharT = ";
        std::cout << ( sizeof ( CharT ) == sizeof ( char ) ? "char" : "wchar_t" );
        std::cout << std::endl;
        return false;
    }
    if ( !string.find ( hshade.c_str ( ) ) )
    {
        std::cout << "The channel sending heavy shade was interrupted for CharT = ";
        std::cout << ( sizeof ( CharT ) == sizeof ( char ) ? "char" : "wchar_t" );
        std::cout << std::endl;
        return false;
    }
    return true;
}

bool testBasicSyncstreambuf ( )
{
    bool result = true;
    result &= basicSyncstreambufTest < char > ( );
    result &= basicSyncstreambufTest < wchar_t > ( );
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