/**
 * @file screens.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief deserializer for screens.
 * @version 1
 * @date 2022-02-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <ux/serialization/externalized.h++>
#include <ux/serialization/strings.h++>

#include <ux/console/screen.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>

#include <io/console/colors/color.h++>
#include <io/console/conmanip.h++>
#include <io/console/console.h++>

namespace ux::serialization
{
    class ExternalizedScreens : public Externalized< console::Screen >
    {
    protected:
        void _parse ( defines::ChrString const & ) override final;

        virtual defines::IString folder ( ) const noexcept override final
        {
            return "screen";
        }
        virtual console::Screen defaultValue (
                std::shared_ptr< ExternalID > const &ext ) const override
        {
            return console::Screen {
                    { console::Line { ext->key } },
                    { console::InputModes::NONE, false, nullptr },
                    { console::Line { ext->key } },
                    { },
                    { { "EmptyString" } },
            };
        }
    public:
        POLYMORPHIC_IDENTIFIER ( ExternalizedScreens )
        ExternalizedScreens ( ) noexcept = default;
        virtual ~ExternalizedScreens ( ) = default;
    };
} // namespace ux::serialization