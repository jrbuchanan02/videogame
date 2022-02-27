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

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>

#include <cstdint>
#include <vector>

namespace io::unicode
{
    // six bits.
    enum class BreakingProperties : defines::Flag
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
        _MAX,
    };

    struct CharacterProperties
    {
        // 0 -> narrow, 1 -> wide
        BITFIELD ( columns )
        // 0 -> non-control character, 1-> control character
        BITFIELD ( control )
        // 0 -> not emoji, 1 -> emoji
        BITFIELD ( emoji )
        BITFIELD ( reserved1 )
        BITFIELD ( reserved2 )
        BITFIELD ( reserved3 )
        BITFIELD ( reserved4 )
        BITFIELD ( reserved5 )
        defines::Flag lineBreaking : 6 =
                ( std::uint8_t ) BreakingProperties::XX;
        CharacterProperties ( ) noexcept                             = default;
        CharacterProperties ( CharacterProperties const & ) noexcept = default;
        CharacterProperties ( CharacterProperties && )               = delete;
    };

    std::vector< CharacterProperties > const &characterProperties ( );
} // namespace io::unicode