/**
 * @file engine.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Engine manages... well... the engine. Everything that Videogame
 * instantiates ends up here where it is managed and connected.
 * @version 1
 * @date 2022-03-05
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

#include <ux/serialization/strings.h++>

#include <filesystem>
#include <memory>

namespace engine
{
    class Object;

    class Engine
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        Engine ( ) noexcept;
        Engine ( Engine const & ) noexcept = delete;
        Engine ( Engine && ) noexcept;
        virtual ~Engine ( );
        Engine &operator= ( Engine const & ) noexcept = delete;

        Engine &operator= ( Engine && ) noexcept;

        Engine ( std::filesystem::path const & ) noexcept;

        /**
         * @brief Set the locale used to translate strings. Since this does not
         * fundamentally change the objects held inside the engine it is not
         * considered to modify the engine.
         *
         */
        void setLocale ( defines::IString const & ) const noexcept;
        /**
         * @brief Get the locale currently used for string-translation.
         *
         * @return defines::IString const&
         */
        defines::IString const &getLocale ( void ) const noexcept;

        /**
         * @brief Set the TransliterationLevel used in string translation. Since
         * this does not fundamentally change the objects held inside the
         * engine, it is not considered to modify the engine.
         *
         */
        void setTransliteration ( ux::serialization::TransliterationLevel const
                                          & ) const noexcept;

        /**
         * @brief Get the current TransliterationLevel used for
         * string-translation
         *
         * @return ux::serialization::TransliterationLevel const&
         */
        ux::serialization::TransliterationLevel const &
                getTransliteration ( void ) const noexcept;

        // ticks the engine, calling tick functions on all objects contained.
        void tick ( );
    };

    /**
     * @brief An object in the engine.
     *
     */
    class Object
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        Object ( ) noexcept;
        Object ( Object const & ) noexcept;
        Object ( Object && ) noexcept;
        virtual ~Object ( ) noexcept;
        Object &operator= ( Object const & ) noexcept;
        Object &operator= ( Object && ) noexcept;

        defines::ObjectIdentifier const &getID ( ) const noexcept;

        // add in event-managing and tick function things.
    };
} // namespace engine