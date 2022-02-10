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

std::vector<test::Unittest> unittests = { };
test::Unittest::Unittest ( std::function<bool ( std::ostream & )> const &test,
                           defines::ChrPString const                    &pass,
                           defines::ChrPString const                    &fail )
        : test ( test )
{
    if ( pass ) passMessage = pass;
    if ( fail ) failMessage = fail;
    unittests.push_back ( *this );
}

void test::runUnittests ( std::ostream &stream )
{
    std::size_t passCount = 0;
    std::size_t failCount = 0;
    for ( auto &test : unittests )
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
    stream << passCount << " / " << unittests.size ( ) << " tests passed.\n";
    stream << failCount << " / " << unittests.size ( ) << " tests failed.\n";
}