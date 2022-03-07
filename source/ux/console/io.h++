/**
 * @file io.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief I/O Things for both input and screen classes
 * @version 1
 * @date 2022-03-07
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <io/console/colors/color.h++>
#include <io/console/conmanip.h++>
#include <io/console/console.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>

#include <ux/serialization/externalized.h++>
#include <ux/serialization/strings.h++>

#include <io/base/syncstream.h++>

#include <any>
#include <memory>

namespace ux::console
{
    enum class InputMode
    {
        NONE,
        FULL_NAME,
        _MAX,
    };

    class Line
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        Line ( ) noexcept;
        Line ( Line const & ) noexcept;
        Line ( Line && ) noexcept;
        virtual ~Line ( );
        Line &operator= ( Line const & ) noexcept;
        Line &operator= ( Line && ) noexcept;

        void setTextID ( defines::IString const & ) noexcept;
        void setTxtRate ( std::uint64_t const & ) noexcept;
        void setCmdRate ( std::uint64_t const & ) noexcept;
        void setCentered ( bool const & ) noexcept;
        void setWrapped ( bool const & ) noexcept;
        void setBold ( bool const & ) noexcept;
        void setFaint ( bool const & ) noexcept;
        void setItalic ( bool const & ) noexcept;
        void setUnderline ( bool const & ) noexcept;
        void setSlowBlink ( bool const & ) noexcept;
        void setFastBlink ( bool const & ) noexcept;
        void setInvert ( bool const & ) noexcept;
        void setHide ( bool const & ) noexcept;
        void setStrike ( bool const & ) noexcept;
        void setFont ( std::uint8_t const & ) noexcept;
        void setFraktur ( bool const & ) noexcept;
        void setDoubleUnderline ( bool const & ) noexcept;
        void setForegroundColor ( std::uint32_t const & ) noexcept;
        void setBackgroundColor ( std::uint32_t const & ) noexcept;

        io::console::ConsoleManipulator operator( ) (
                std::shared_ptr< serialization::ExternalizedStrings > const &,
                std::shared_ptr< defines::IString > const &,
                serialization::TransliterationLevel const & );
    };

    class IOThing
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    protected:
        virtual void secretSauce (
                io::console::Console &,
                std::shared_ptr< serialization::ExternalizedStrings > const &,
                std::shared_ptr< defines::IString > const &,
                serialization::TransliterationLevel const & );
    public:
        POLYMORPHIC_IDENTIFIER ( IOThing )
        IOThing ( ) noexcept;
        IOThing ( IOThing const & ) noexcept;
        IOThing ( IOThing && ) noexcept;
        virtual ~IOThing ( );
        IOThing &operator= ( IOThing const & ) noexcept;
        IOThing &operator= ( IOThing && ) noexcept;

        io::console::ConsoleManipulator operator( ) (
                std::shared_ptr< serialization::ExternalizedStrings > const &,
                std::shared_ptr< defines::IString > const &,
                serialization::TransliterationLevel const & );

        /**
         * @brief List of the possible IOThings to display next.
         *
         * @return std::list<serialization::ExternalID> const&
         */
        std::list< std::shared_ptr< IOThing > > const &
                availableScreens ( ) const noexcept;

        /**
         * @brief Attempts to add an option to the internal list. Returns false
         * if the operation fails
         * 
         * @return true added the option to the internal list and it should now
         * appear in the list returned by availableScreens
         * @return false did not add the option to the internal list because it
         * is already there. (You could instead modify the logic to assign the
         * next screen!)
         */
        bool addOption(std::shared_ptr<IOThing> const &) noexcept;
    };

    class Input;

    class Screen : public IOThing
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    protected:
        void secretSauce (
                io::console::Console &,
                std::shared_ptr< serialization::ExternalizedStrings > const &,
                std::shared_ptr< defines::IString > const &,
                serialization::TransliterationLevel const & ) override final;
    public:
        POLYMORPHIC_IDENTIFIER ( Screen )
        Screen ( ) noexcept;
        Screen ( Screen const & ) noexcept;
        Screen ( Screen && ) noexcept;
        virtual ~Screen ( );
        Screen &operator= ( Screen const & ) noexcept;
        Screen &operator= ( Screen && ) noexcept;

        void setPrompt ( std::shared_ptr< Input > const & ) noexcept;
        std::shared_ptr< Input > const &getPrompt ( void ) noexcept;
    };

    class Input : public IOThing
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    protected:
        void secretSauce (
                io::console::Console &,
                std::shared_ptr< serialization::ExternalizedStrings > const &,
                std::shared_ptr< defines::IString > const &,
                serialization::TransliterationLevel const & ) override final;
    public:
        POLYMORPHIC_IDENTIFIER ( Input )
        Input ( ) noexcept;
        Input ( Input const & ) noexcept;
        Input ( Input && ) noexcept;
        virtual ~Input ( );
        Input &operator= ( Input const & ) noexcept;
        Input &operator= ( Input && ) noexcept;

        InputMode const getInputMode ( ) const noexcept;
        std::any const  getInput ( ) const noexcept;
    };

} // namespace ux::console

// move to io.c++
