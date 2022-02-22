/**
 * @file screens.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implements the screen externalization
 * @version 1
 * @date 2022-02-22
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <ux/serialization/screens.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

std::strong_ordering ux::serialization::ScreenID::operator<=> (
        ScreenID const &id ) const noexcept
{
    return this->screen <=> id.screen;
}