/**
 * @file screens.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Deserializes the screens.
 * @version 1
 * @date 2022-02-23
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

#include <io/console/console.h++>

#include <filesystem>
#include <memory>

namespace ux::serialization
{
    class ExternalizedScreen : public Externalized<console::Screen>
    {

    };
}