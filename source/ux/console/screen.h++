/**
 * @file screen.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A screen.
 * @version 1
 * @date 2022-02-23
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <ux/console/colors.h++>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include <memory>

namespace ux::console
{
    class Line
    {
        struct impl_s;
        std::unique_ptr<impl_s> pimpl;
    public:

    };

    class Screen
    {
        struct impl_s;
        std::unique_ptr<impl_s> pimpl;
    public:
        
    };
}