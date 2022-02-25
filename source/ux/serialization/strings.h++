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

#include <ux/serialization/externalized.h++>

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

    struct StringKey : public ExternalID
    {
        defines::IString     language;
        TransliterationLevel transliterationLevel;
        POLYMORPHIC_IDENTIFIER ( StringKey )

        StringKey ( ) noexcept = default;

        StringKey ( defines::IString     key,
                    defines::IString     lang,
                    TransliterationLevel tlit ) :
                language { lang },
                transliterationLevel ( tlit )
        {
            this->key = key;
        }
    protected:
        virtual std::partial_ordering const
                order ( ExternalID const &id ) const noexcept
        {
            if ( id.getIdentifier ( ) == StringKey::identifier )
            {
                // check against the language
                if ( language
                     != dynamic_cast< StringKey const & > ( id ).language )
                {
                    return language
                       <=> dynamic_cast< StringKey const & > ( id ).language;
                } else if ( transliterationLevel
                            != dynamic_cast< StringKey const & > ( id )
                                       .transliterationLevel )
                {
                    return transliterationLevel
                       <=> dynamic_cast< StringKey const & > ( id )
                                   .transliterationLevel;
                } else
                {
                    return std::partial_ordering::equivalent;
                }
            }
            return std::partial_ordering::unordered;
        }
    };

    class ExternalizedStrings : public Externalized< defines::IString >
    {
    protected:
        virtual void _parse ( defines::ChrString const & ) override;

        virtual defines::ChrString folder ( ) const noexcept override final
        {
            return "text";
        }
        virtual defines::IString defaultValue (
                std::shared_ptr< ExternalID > const &ext ) const override final
        {
            defines::IString result = "!" + ext->key;
            if ( ext->getIdentifier ( ) == StringKey::identifier )
            {
                auto const &temp = dynamic_cast< StringKey & > ( *ext );
                result += "." + temp.language + "."
                        + defines::rtToString< TransliterationLevel > (
                                  temp.transliterationLevel );
            }
            result += "!";
            return result;
        }
    public:
        POLYMORPHIC_IDENTIFIER ( ExternalizedStrings )
        ExternalizedStrings ( ) noexcept = default;
        virtual ~ExternalizedStrings ( ) = default;
    };
} // namespace ux::serialization