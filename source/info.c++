/**
 * @file info.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Definitions for info.h++
 * @version 1
 * @date 2022-01-22
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#include "info.h++"

#include <iostream>

#ifdef IN_SOURCE_INFO_HPP
template < class CharT , class Traits = std::char_traits < CharT > >
void outputCommandline ( std::basic_ostream < CharT , Traits > &ostream , int const &argc , char const *const *const &argv )
{
    ostream << ( argc ) << " argument" << (argc != 1 ? "s" : "" ) << " passed.\n";
    for ( int i = 0; i < argc; i++ ) {
        ostream << "Argument " << ( i + 1 ) << ": \"" << argv [ i ] << "\"\n";
    }
}
#endif