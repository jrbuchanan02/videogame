/**
 * @file screens.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Reads in screens from the data files.
 * @version 1
 * @date 2022-02-22
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <ux/console/screen.h++>
#include <ux/serialization/strings.h++>

#include <filesystem>
#include <memory>

namespace ux::serialization
{
    struct ScreenID
    {
        defines::IString screen;

        std::strong_ordering operator<=> ( ScreenID const & ) const noexcept;
    };

    class ExternalizedScreens
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        ExternalizedScreens ( ) noexcept;
        ExternalizedScreens ( std::filesystem::path const & );

        virtual ~ExternalizedScreens ( );

        void set ( ScreenID const &, console::Screen const & );

        console::Screen const get ( ScreenID const & ) const noexcept;
    };
} // namespace ux::serialization