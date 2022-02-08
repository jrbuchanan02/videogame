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
#include <rapidxml-1.13/rapidxml.hpp>
#include <vector>

namespace io::unicode
{
#if !defined( CHAR_SIZE ) || CHAR_SIZE == 3 || CHAR_SIZE > 4
// tell the user what we're doing
#    ifndef CHAR_SIZE
#        pragma message                                                        \
                "CHAR_SIZE was undefined. It will use the default value: 1"
#    else
#        pragma message                                                        \
                "CHAR_SIZE was defined to an invalid value (one other than 1," \
                " 2, or 4). It will use the default value: 1"
#    endif
// ignored when CHAR_SIZE is undefined
#    undef CHAR_SIZE
// defines CHAR_SIZE to the default value
#    define CHAR_SIZE 1
#endif // if !defined( CHAR_SIZE ) || CHAR_SIZE == 3 || CHAR_SIZE > 4
#if CHAR_SIZE == 1
    using Char = char;
#elif CHAR_SIZE == 2
    using Char = char16_t;
#elif CHAR_SIZE == 4
    using Char = char32_t;
#else
#    error "Char was not defined? How did this happen!?"
#endif // if CHAR_SIZE == 1
    using XMLDoc  = rapidxml::xml_document<Char>;
    using XMLNode = rapidxml::xml_node<Char>;
    using XMLAttr = rapidxml::xml_attribute<Char>;
    using Flag    = std::uint8_t;
#define BITFIELD( N ) Flag N : 1 = 0;

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