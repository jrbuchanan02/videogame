/**
 * @file stringfunctions.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief String functions -- text wrapping, text justification
 * @version 1
 * @date 2022-01-26
 * 
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 * 
 */
#pragma once

#include <string>
#include <vector>

namespace io::console::manip
{
    /**
     * @brief Splits the string into a vector of its code-points.
     * @param std::string the string to split
     * @throw Throws std::runtime_error if there is an invalid / unknown code 
     * point.
     * @return std::vector < std::string > 
     */
    std::vector < std::string > splitByCodePoint ( std::string );
}