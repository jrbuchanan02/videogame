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
#include <random>

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