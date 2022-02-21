/**
 * @file unittester.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Unit tests
 * @version 1
 * @date 2022-01-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <defines/macros.h++>
#include <defines/types.h++>
#include <test/unittester.h++>

#include <sstream>
#include <vector>
struct TestRunner
{
    static inline std::vector< test::Unittest > *unittests { nullptr };
};
static std::size_t count = 0;
test::Unittest::Unittest ( std::function< bool ( std::ostream & ) > const &test,
                           defines::ChrPString const                      &pass,
                           defines::ChrPString const &fail ) :
        test ( test )
{
    if ( pass )
        passMessage = pass;
    if ( fail )
        failMessage = fail;
    if ( !TestRunner::unittests )
    {
        TestRunner::unittests = new std::vector< test::Unittest > ( );
    }
    TestRunner::unittests->push_back ( *this );
    if ( count && TestRunner::unittests->size ( ) == 1 )
    {
        RUNTIME_ERROR ( "Previous unittest did not add to vector!" )
        // throw std::runtime_error ( "Previous unittest did not add to vector!"
        // );
    }
    count++;
}

bool test::runUnittests ( std::ostream &stream )
{
    if ( !count )
    {
        RUNTIME_ERROR ( "Found no unittests! There's one in this file!!!" )
    }
    if ( count != TestRunner::unittests->size ( ) )
    {
        RUNTIME_ERROR ( "Invalid unittest count!" )
    }
    std::size_t passCount = 0;
    std::size_t failCount = 0;
    FOREACH ( test, *TestRunner::unittests )
    {
        if ( test.test ( stream ) )
        {
            passCount++;
            stream << "\n\t" << test.passMessage << std::endl;
        } else
        {
            failCount++;
            stream << "\n\t" << test.failMessage << std::endl;
        }
    }
    stream << "\n\n";
    stream << passCount << " / " << TestRunner::unittests->size ( )
           << " tests passed.\n";
    stream << failCount << " / " << TestRunner::unittests->size ( )
           << " tests failed.\n";
    stream << "Cleaning up after unittests...\n";
    TestRunner::unittests->clear ( );
    delete TestRunner::unittests;
    TestRunner::unittests = nullptr;

    if ( failCount )
    {
        return true;
    } else
    {
        return false;
    }
}

static bool testRuntimeErrorMacro ( std::ostream &os )
{
    os << "Testing that the runtime error macro gets the correct lines...\n";
    int         aroundHere = 0;
    std::string file       = __FILE__;
    try
    {
        aroundHere = __LINE__;
        RUNTIME_ERROR ( "Testing... ", "1, 2, 3" )
    } catch ( std::runtime_error &rt )
    {
        std::stringstream parser ( rt.what ( ) );
        if ( parser.str ( ).find ( file ) == std::string::npos )
        {
            BEGIN_UNIT_FAIL ( os, "Could not find file" )
            os << "Expected to find " << __FILE__ << " in " << parser.str ( );
            os << " but could not!";
            END_UNIT_FAIL ( os )
        }
        bool found = false;
        for ( int i = aroundHere - 5; i < aroundHere + 6; i++ )
        {
            std::stringstream temp;
            temp << i;
            if ( parser.str ( ).find ( temp.str ( ) ) != std::string::npos )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            BEGIN_UNIT_FAIL ( os, "Could not find line number" )
            os << "Expected to find " << aroundHere
               << " or any other number on the range [" << aroundHere - 5
               << ", " << aroundHere + 5
               << "], but could not while searching within " << parser.str ( );
            END_UNIT_FAIL ( os )
        }
        return true;
    } catch ( ... )
    {
        BEGIN_UNIT_FAIL ( os, "Got a different exception than expected" )
        os << "Expeccted a std::runtime_error, or subclass of it, but did not "
              "get that!";
        END_UNIT_FAIL ( os )
    }
    return false;
}

test::Unittest rtErrorMacro = { &testRuntimeErrorMacro };