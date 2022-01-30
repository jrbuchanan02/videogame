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

std::mt19937_64 generator { std::random_device ( ) ( ) };

std::normal_distribution<RandomNumber> distribution ( 0, 1 );

void          seedTable ( );
std::uint64_t grabFromTable ( );

bool engine::rand::sigmaCheck ( RandomNumber const against )
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

void seedTable ( )
{
    static std::random_device device;
    tableSeed = device ( ) % ( sizeof ( table ) / sizeof ( std::uint64_t ) );
    tableSpot = tableSeed;
}

std::uint64_t grabFromTable ( )
{
    std::uint64_t result = table [ tableSpot++ ];
    if ( tableSpot == tableSeed )
    {
        seedTable ( );
    }
    return result;
}