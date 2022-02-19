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

#include <defines/macros.h++>
#include <defines/types.h++>

#include <cstdint>
#include <vector>

namespace io::unicode
{
    // six bits.
    enum class BreakingProperties : std::uint8_t
    {
        BK,
        CR,
        LF,
        CM,
        NL,
        SG,
        WJ,
        ZW,
        GL,
        SP,
        ZWJ,
        B2,
        BA,
        BB,
        HY,
        CB,
        CL,
        CP,
        EX,
        IN,
        NS,
        OP,
        QU,
        IS,
        NU,
        PO,
        PR,
        SY,
        AI,
        AL,
        CJ,
        EB,
        EM,
        H2,
        H3,
        HL,
        ID,
        JL,
        JV,
        JT,
        RI,
        SA,
        XX,
    };

    struct CharacterProperties
    {
#define B( N ) BITFIELD ( N )
        // 0 -> narrow, 1 -> wide
        B ( columns )
        // 0 -> non-control character, 1-> control character
        B ( control )
        // 0 -> not emoji, 1 -> emoji
        B ( emoji )
        B ( reserved1 )
        B ( reserved2 )
        B ( reserved3 )
        B ( reserved4 )
        B ( reserved5 )
        std::uint8_t lineBreaking : 6 = (std::uint8_t)BreakingProperties::XX;
#undef B
        inline CharacterProperties ( ) noexcept = default;
        inline CharacterProperties ( CharacterProperties const & ) noexcept =
                default;
        inline CharacterProperties ( CharacterProperties && ) = delete;
    };

    std::vector< CharacterProperties > const &characterProperties ( );
} // namespace io::unicode