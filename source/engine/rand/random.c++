/**
 * @file random.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implementation for the random numbers.
 * @version 1
 * @date 2022-01-25
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include "random.h++"

#include <cstdlib>
#include <defines/macros.h++>
#include <random>
#include <test/unittester.h++>

using namespace engine::rand;

// global random number generator for the normal distribution
std::mt19937_64 generator { std::random_device ( ) ( ) };
// random number generator for the sigma-checks
std::normal_distribution<defines::RandomNumber> distribution ( 0, 1 );
// function that seeds the random number table with hardware-entropy
void seedTable ( );
// function that grabs a random number from the table.
std::uint64_t grabFromTable ( );

/**
 * @brief Checks against a random number. Quite simply, if the internally
 * generated number exceeds the number to check against, the check succeeds
 *
 * @param against the sigma value required to pass.
 * @return whether the check succeeds.
 */
bool engine::rand::sigmaCheck ( defines::RandomNumber const against )
{
    auto roll = distribution ( generator );
    if ( roll * roll >= 100 )
    {
        return true;
    } else
        return roll > against;
}

std::uint32_t tableSeed = 0;
std::uint32_t tableSpot = 0;
// random table, fill with values more random than this soon.
std::uint64_t table [] = {
        1,
        2,
        3,
        4,
};

/**
 * @brief Seeds the random table with hardware entropy (if available)
 *
 */
void seedTable ( )
{
    static std::random_device device;
    tableSeed = device ( ) % ( sizeof ( table ) / sizeof ( std::uint64_t ) );
    tableSpot = tableSeed;
}

/**
 * @brief Grabs a number from the table, seeding if there was a repetition
 *
 * @return std::uint64_t
 */
std::uint64_t grabFromTable ( )
{
    std::uint64_t result = table [ tableSpot++ ];
    if ( tableSpot == tableSeed )
    {
        seedTable ( );
    }
    return result;
}

defines::RandomNumber engine::rand::generatePRandom ( )
{
    return ( defines::RandomNumber ) ( grabFromTable ( ) );
}

bool sigmaCheckTest ( std::ostream &os )
{
    os << "Beginning self test for the sigmaCheck algorithm.\n";
    std::uint64_t       counters [ 11 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    static std::int64_t checks [ 11 ]   = {
            -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
    constexpr std::uint64_t samples = 1000000;
    for ( std::uint64_t i = 0; i < samples; i++ )
    {
#pragma GCC unroll 11
        for ( std::size_t j = 0; j < 11; j++ )
        {
            if ( engine::rand::sigmaCheck ( checks [ j ] ) )
            {
                counters [ j ]++;
            }
        }
    }

    for ( std::size_t i = 0; i < 10; i++ )
    {
        if ( counters [ i ] < counters [ i + 1 ] )
        {
            BEGIN_UNIT_FAIL ( os, "Really weird randomness" )
            os << "Despite " << samples << " samples, the sigma check against "
               << checks [ i ] << " passed less less often than those against "
               << checks [ i + 1 ]
               << ". While this is theoretically not an error, the sample size "
                  "is so large that this test fails!";
            END_UNIT_FAIL ( os )
        }
    }
    return true;
}

test::Unittest sigmaTest = { &sigmaCheckTest };