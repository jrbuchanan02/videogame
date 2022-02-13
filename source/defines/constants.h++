/**
 * @file constants.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Constants
 * @version 1
 * @date 2022-02-13
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/macros.h++>
#include <defines/types.h++>

namespace defines
{
    constexpr ChrPString ucdDataFile = "ucd.all.grouped.xml";
    constexpr ChrPString ucdDataPath = "./data/unicode/";
    constexpr ChrPString ucdDataName = "./data/unicode/ucd.all.grouped.xml";

    constexpr std::uint32_t maxUnicode = 0x10FFFF;

    constexpr std::uint64_t sigmaTestSamples = 1000000;
    constexpr std::int64_t  sigmaCheckValues [] =
            { -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
} // namespace defines