/**
 * @file info.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Outputs information on the system that we are currently running on
 * @version 1
 * @date 2022-01-22
 * 
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 * 
 */
#pragma once

#include <iostream>

template < class CharT , class Traits = std::char_traits < CharT > >
void outputCommandline ( std::basic_ostream < CharT , Traits > & , int const & , char const *const *const &);

#define IN_SOURCE_INFO_HPP
#include "info.c++"
#undef IN_SOURCE_INFO_HPP