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
#include <defines/types.h++>
#include <test/unittester.h++>
#include <vector>

struct TestRunner
{
    static inline std::vector<test::Unittest> *unittests { nullptr };
};
static std::size_t count = 0;
test::Unittest::Unittest ( std::function<bool ( std::ostream & )> const &test,
                           defines::ChrPString const                    &pass,
                           defines::ChrPString const                    &fail )
        : test ( test )
{
    if ( pass ) passMessage = pass;
    if ( fail ) failMessage = fail;
    if ( !TestRunner::unittests )
    {
        TestRunner::unittests = new std::vector<test::Unittest> ( );
    }
    TestRunner::unittests->push_back ( *this );
    if ( count && TestRunner::unittests->size ( ) == 1 )
    {
        throw std::runtime_error ( "Previous unittest did not add to vector!" );
    }
    count++;
    std::cout << "Found one more unittest to run, bringing the total to "
              << count << " from " << count - 1 << ". There are accordingly "
              << TestRunner::unittests->size ( ) << " test"
              << ( TestRunner::unittests->size ( ) == 1 ? "" : "s" )
              << " in the vector.\n";
}

void test::runUnittests ( std::ostream &stream )
{
    stream << "Found " << count << " unittest" << ( count == 1 ? "" : "s" )
           << " to run.\n";
    stream << "Accordingly, the unittest vector contains "
           << TestRunner::unittests->size ( ) << " element"
           << ( TestRunner::unittests->size ( ) == 1 ? "" : "s" ) << "\n";
    if ( !count )
    {
        throw std::runtime_error ( "Found no unittests!" );
    }
    if ( count != TestRunner::unittests->size ( ) )
    {
        throw std::runtime_error ( "Invalid unittest count!" );
    }
    std::size_t passCount = 0;
    std::size_t failCount = 0;
    for ( auto &test : *TestRunner::unittests )
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
}