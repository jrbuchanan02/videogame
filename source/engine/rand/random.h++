/**
 * @file random.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Random number generation
 * @version 1
 * @date 2022-01-25
 * 
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 * 
 */
#pragma once


namespace engine::rand
{
    using RandomNumber = long double;

    /**
     * @brief Performs a check against a given z-score.
     * @note If the internally generated value defies all odds and has an absolute
     * value >= 10, the check succeeds since that's so stupidly unlikely.
     * @note We use the following values for reference:
     * 
     * - against <= -5 -> Certain Success (still slightly random)
     * - against <= -4 -> Almost Certain Success
     * - against <= -3 -> Highly Likely
     * - against <= -2 -> Likely
     * - against <= -1 -> Probable
     * - against ~= 0  -> Coin Flip
     * - against >= 1  -> Improbable
     * - against >= 2  -> Unlikely
     * - against >= 3  -> Highly Unlikely
     * - against >= 4  -> Almost Certain Failure
     * - against >= 5  -> Certain Failure (still slightly random)
     * 
     * @param against The required value to pass the probability check
     * @return true success
     * @return false failure
     */
    bool sigmaCheck ( RandomNumber const against );

    /**
     * @brief Generates a Pseudorandom Number.
     * 
     * @return RandomNumber 
     */
    RandomNumber generatePRandom ( );
}