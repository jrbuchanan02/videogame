/**
 * @file character.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A character loaded in from the UCD
 * @version 1
 * @date 2022-02-07
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <cstdint>
#include <defines/types.h++>
#include <rapidxml-1.13/rapidxml.hpp>
#include <vector>

namespace io::unicode
{
    struct CharacterProperties
    {
#define B( N ) BITFIELD ( N )
        // 0 -> narrow, 1 -> wide
        B ( columns )
        // 0 -> non-control character, 1-> control character
        B ( control )
        // 0 -> does not "want" a line break, 1-> "wants" a line break
        B ( wantsLB )
        // 0 -> wantsLB is a preference, 1-> wantsLB is a requirement
        B ( require )
#undef B
        inline CharacterProperties ( ) noexcept = default;
        inline CharacterProperties ( CharacterProperties const & ) noexcept =
                default;
        inline CharacterProperties ( CharacterProperties && ) = delete;
    };

    std::vector<CharacterProperties> const &characterProperties ( );
} // namespace io::unicode