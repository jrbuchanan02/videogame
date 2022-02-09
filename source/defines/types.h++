/**
 * @file types.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Defines types
 * @version 1
 * @date 2022-02-09
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <io/base/syncstream.h++>
#include <rapidxml-1.13/rapidxml.hpp>
#include <string>

// syntactic sugar to prevent each define from being enclosed with
// an #ifndef DEFINE ... #endif // #ifndef DEFINE
#ifndef SOURCE_DEFINES_TYPES
#    define SOURCE_DEFINES_TYPES

namespace defines
{
    using Char   = char;
    using Char08 = char8_t;
    using Char16 = char16_t;
    using Char32 = char32_t;

    using U08Char = Char08;
    using U16Char = Char16;
    using U32Char = Char32;
    using ChrChar = Char;

    // define Internal character size and External character size if they were
    // not defined already.
#    if !defined( I_CHAR_SIZE )                                                \
            || ( I_CHAR_SIZE != 1 && I_CHAR_SIZE != 2 && I_CHAR_SIZE != 4 )
#        undef I_CHAR_SIZE
#        define I_CHAR_SIZE 1
#    endif

#    if !defined( E_CHAR_SIZE )                                                \
            || ( E_CHAR_SIZE != 1 && E_CHAR_SIZE != 2 && E_CHAR_SIZE != 4 )
#        undef E_CHAR_SIZE
#        define E_CHAR_SIZE 1
#    endif

#    define INTERNAL( X ) I##X
#    define EXTERNAL( X ) E##X

#    define UTF08( X ) U08##X
#    define UTF16( X ) U16##X
#    define UTF32( X ) U32##X
#    define _CHAR( X ) Chr##X

#    if INTERNAL( _CHAR_SIZE ) == 1
    using INTERNAL ( Char ) = ChrChar;
#        define IS( X ) ( ( ChrChar * ) X )
#    elif INTERNAL( _CHAR_SIZE ) == 2
    using INTERNAL ( Char ) = Char16;
#        define IS( X ) L##X
#    elif INTERNAL( _CHAR_SIZE ) == 4
    using INTERNAL ( Char ) = Char32;
#        define IS( X ) U##X
#    else
#        error "Invalid Character size"
#    endif

#    if EXTERNAL( _CHAR_SIZE ) == 1
    using EXTERNAL ( Char ) = ChrChar;
#        define ES( X ) ( ( ChrChar * ) X )
#    elif EXTERNAL( _CHAR_SIZE ) == 2
    using EXTERNAL ( Char ) = Char16;
#        define ES( X ) L##X
#    elif EXTERNAL( _CHAR_SIZE ) == 4
    using EXTERNAL ( Char ) = Char32;
#        define ES( X ) U##X
#    else
#        error "Invalid Character size:"
#    endif

#    define CHARACTER_USING( Name, Expr )                                      \
        using INTERNAL ( Name ) = INTERNAL ( Expr );                           \
        using EXTERNAL ( Name ) = EXTERNAL ( Expr );                           \
        using UTF08 ( Name )    = UTF08 ( Expr );                              \
        using UTF16 ( Name )    = UTF16 ( Expr );                              \
        using UTF32 ( Name )    = UTF32 ( Expr );                              \
        using _CHAR ( Name )    = _CHAR ( Expr );
#    define CHARACTER_TEMPLATE_USING( Name, Template )                         \
        using INTERNAL ( Name ) = Template<INTERNAL ( Char )>;                 \
        using EXTERNAL ( Name ) = Template<EXTERNAL ( Char )>;                 \
        using UTF08 ( Name )    = Template<UTF08 ( Char )>;                    \
        using UTF16 ( Name )    = Template<UTF16 ( Char )>;                    \
        using UTF32 ( Name )    = Template<UTF32 ( Char )>;                    \
        using _CHAR ( Name )    = Template<_CHAR ( Char )>;

    CHARACTER_USING ( CString, Char * )
    CHARACTER_USING ( PString, Char const * )
    CHARACTER_TEMPLATE_USING ( String, std::basic_string )
    CHARACTER_TEMPLATE_USING ( StringStream, std::basic_stringstream )
    CHARACTER_TEMPLATE_USING ( FileStream, std::basic_fstream )
    CHARACTER_TEMPLATE_USING ( OutputStream, std::basic_ostream )
    CHARACTER_TEMPLATE_USING ( InputStream, std::basic_istream )
    CHARACTER_TEMPLATE_USING ( XMLDocument, rapidxml::xml_document )
    CHARACTER_TEMPLATE_USING ( XMLNode, rapidxml::xml_node )
    CHARACTER_TEMPLATE_USING ( XMLAttribute, rapidxml::xml_attribute )
    CHARACTER_TEMPLATE_USING ( Syncbuf, io::base::basic_syncstreambuf )
    CHARACTER_TEMPLATE_USING ( Syncstream, io::base::basic_osyncstream )

#    define CHAR08_AS_CHAR( X ) ( ( char ) X )
    // end the characters

    // now other types
    using Flag = std::uint8_t;
#    define BITFIELD( X ) defines::Flag X : 1 = 0;

    using RandomNumber = double;
} // namespace defines

#endif // ifndef SOURCE_DEFINES_TYPES