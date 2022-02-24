/**
 * @file screen.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A screen as shown on a console.
 * @version 1
 * @date 2022-02-20
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <filesystem>
#include <memory>

namespace ux::serialization
{
    /**
     * @brief Transliteration level dictates that, for non-translated text, we
     * choose between:
     * 先生、どうもありがとうございます！
     * and
     * Sensei, doumoarigatogozaimas!
     */
    enum class TransliterationLevel : std::uint8_t
    {
        NOT,  // no transliteration
        YES,  // transliterated according to general conventions, leading to
              // accented text
        ALT,  // transliterated in a way that keeps an ASCII alphabet.
        _MAX, // unused maximum value to make this a VideoEnumeration
    };

    struct StringKey
    {
        defines::IString const     language;
        defines::IString const     text;
        TransliterationLevel const level;

        inline std::strong_ordering const
                operator<=> ( StringKey const &key ) const noexcept
        {
            // first, compare language
            if ( language != key.language )
            {
                return language <=> key.language;
            } else if ( text != key.text )
            {
                return text <=> key.text;
            } else
            {
                return ( std::uint8_t ) level <=> ( std::uint8_t ) key.level;
            }
        }
    };

    class ExternalizedStrings
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        ExternalizedStrings ( ) noexcept;
        ExternalizedStrings ( std::filesystem::path const & );

        virtual ~ExternalizedStrings ( );

        void set ( StringKey const &, defines::IString const & );
        defines::IString const get ( StringKey const & ) const noexcept;
    };
} // namespace ux::serialization