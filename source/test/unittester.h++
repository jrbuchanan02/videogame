/**
 * @file unittester.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Performs the unit testing.
 * @version 1
 * @date 2022-01-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/types.h++>
#include <functional>
#include <iostream>
namespace test
{
    using namespace defines;
    /**
     * @brief Runs the unittests, outputting information on their pass/fail
     * rates to the specified stream.
     *
     * @param stream the stream to output information to.
     */
    void runUnittests ( std::ostream &stream );

    /**
     * @brief A unittest to run.
     *
     */
    struct Unittest
    {
        ChrPString passMessage = "Unittest passed.";
        ChrPString failMessage = "Unittest failed.";

        std::function<bool ( std::ostream & )> test;

        Unittest ( std::function<bool ( std::ostream & )> const &test,
                   ChrPString const                             &pass = nullptr,
                   ChrPString const &fail = nullptr );
    };

} // namespace test