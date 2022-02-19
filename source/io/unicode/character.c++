/**
 * @file character.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implementation of character
 * @version 1
 * @date 2022-02-07
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <io/unicode/character.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <test/unittester.h++>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <rapidxml-1.13/rapidxml.hpp>

using namespace io::unicode;
using namespace defines;

#define LINE_BREAKING_CASE( val )                                              \
    if ( temp == ES ( #val ) )                                                 \
    {                                                                          \
        result.lineBreaking = ( std::uint8_t ) BreakingProperties::val;        \
    }

std::vector< CharacterProperties > properties = { };

void initializeProperties ( );

std::vector< CharacterProperties > const &io::unicode::characterProperties ( )
{
    if ( properties.empty ( ) )
    {
        initializeProperties ( );
    }
    return properties;
}

void initializeProperties ( )
{
    defines::EXMLDocument document;
    defines::EFileStream  file ( defines::ucdDataName );
    defines::EString      contents = ES ( "" );
    while ( !file.eof ( ) )
    {
        defines::EString temp;
        std::getline ( file, temp );
        contents += temp;
    }
    file.close ( );
    document.parse< 0 > ( ( defines::ECString ) contents.c_str ( ) );
    defines::EXMLNode *group = document.first_node ( "ucd" )
                                       ->first_node ( "repertoire" )
                                       ->first_node ( "group" );
    auto parseCharacterProperties = [ & ] ( defines::EXMLNode *node ) {
        auto getField =
                [ & ] ( defines::EXMLNode *p,
                        defines::EXMLNode *c,
                        defines::ECString  n ) -> defines::EXMLAttribute * {
            if ( c->first_attribute ( ES ( n ) ) )
            {
                return c->first_attribute ( ES ( n ) );
            } else if ( p->first_attribute ( ES ( n ) ) )
            {
                return p->first_attribute ( ES ( n ) );
            } else
            {
                RUNTIME_ERROR (
                        "Failed to find field ",
                        n,
                        " for unicode character ",
                        ( c->first_attribute ( "cp" )
                                  ? c->first_attribute ( "cp" )->value ( )
                                  : " in group" ) )
            }
        };
        CharacterProperties result;
        defines::EString    temp = ES ( "" );

        defines::ECString ea =
                getField ( node->parent ( ), node, ES ( "ea" ) )->value ( );
        temp = ea;
        if ( temp == ES ( "A" ) || temp == ES ( "F" ) || temp == ES ( "W" ) )
        {
            result.columns = 1;
        } else
        {
            result.columns = 0;
        }
        temp = "";

        defines::ECString emoji =
                getField ( node->parent ( ), node, ES ( "Emoji" ) )->value ( );

        temp = std::string ( emoji );
        if ( temp == ES ( "Y" ) )
        {
            result.emoji = 1;
        } else if ( temp == ES ( "N" ) )
        {
            result.emoji = 0;
        } else
        {
            RUNTIME_ERROR ( "Yes / No field was not yes or no!" )
        }

        // determine whether a line break is preferred and / or required.
        defines::ECString lineBreak =
                getField ( node->parent ( ), node, ES ( "lb" ) )->value ( );
        temp = lineBreak;
        // non-tailorable line-breaking classes
        LINE_BREAKING_CASE ( BK )
        LINE_BREAKING_CASE ( CR )
        LINE_BREAKING_CASE ( LF )
        LINE_BREAKING_CASE ( CM )
        LINE_BREAKING_CASE ( NL )
        LINE_BREAKING_CASE ( SG )
        LINE_BREAKING_CASE ( WJ )
        LINE_BREAKING_CASE ( ZW )
        LINE_BREAKING_CASE ( GL )
        LINE_BREAKING_CASE ( SP )
        LINE_BREAKING_CASE ( ZWJ )
        // break opportunities
        LINE_BREAKING_CASE ( B2 )
        LINE_BREAKING_CASE ( BA )
        LINE_BREAKING_CASE ( BB )
        LINE_BREAKING_CASE ( HY )
        LINE_BREAKING_CASE ( CB )
        // characters prohibiting certain breaks
        LINE_BREAKING_CASE ( CL )
        LINE_BREAKING_CASE ( CP )
        LINE_BREAKING_CASE ( EX )
        LINE_BREAKING_CASE ( IN )
        LINE_BREAKING_CASE ( NS )
        LINE_BREAKING_CASE ( OP )
        LINE_BREAKING_CASE ( QU )
        // numeric context
#pragma push_macro( "IS" ) // conflicts wiht the define "IS"
#undef IS
        LINE_BREAKING_CASE ( IS )
#pragma pop_macro( "IS" )
        LINE_BREAKING_CASE ( NU )
        LINE_BREAKING_CASE ( PO )
        LINE_BREAKING_CASE ( PR )
        LINE_BREAKING_CASE ( SY )
        // other characters
        LINE_BREAKING_CASE ( AI )
        LINE_BREAKING_CASE ( AL )
        LINE_BREAKING_CASE ( CL )
        LINE_BREAKING_CASE ( CJ )
        LINE_BREAKING_CASE ( EB )
        LINE_BREAKING_CASE ( EM )
        LINE_BREAKING_CASE ( H2 )
        LINE_BREAKING_CASE ( H3 )
        LINE_BREAKING_CASE ( HL )
        LINE_BREAKING_CASE ( ID )
        LINE_BREAKING_CASE ( JL )
        LINE_BREAKING_CASE ( JV )
        LINE_BREAKING_CASE ( JT )
        LINE_BREAKING_CASE ( RI )
        LINE_BREAKING_CASE ( SA )
        LINE_BREAKING_CASE ( XX )

        properties.push_back ( result );
    };
    while ( group )
    {
        defines::EXMLNode *child = group->first_node ( );
        while ( child )
        {
            if ( child->first_attribute ( "cp" ) )
            {
                // push back one character
                parseCharacterProperties ( child );
            } else if ( child->first_attribute ( "first-cp" ) )
            {
                if ( !child->first_attribute ( "last-cp" ) )
                {
                    RUNTIME_ERROR (
                            "Found the first code point, but the last "
                            "code point is missing!" )
                }
                // parse code point
                auto parseCodePoint =
                        [ & ] ( defines::ECString field ) -> std::uint32_t {
                    defines::EStringStream stream (
                            child->first_attribute ( field )->value ( ) );
                    std::uint32_t result = 0;
                    stream >> std::hex >> result;
                    return result;
                };

                std::uint32_t first, last;
                first = parseCodePoint ( ES ( "first-cp" ) );
                last  = parseCodePoint ( ES ( "last-cp" ) );
                for ( std::uint32_t i = first; i <= last; i++ )
                {
                    parseCharacterProperties ( child );
                }
            } else
            {
                RUNTIME_ERROR (
                        "Cannot parse node: it's not a single "
                        "character nor a range of them!" )
            }
            child = child->next_sibling ( );
        }
        group = group->next_sibling ( );
    }
}

bool propertyInitializationTest ( std::ostream &stream )
{
    static defines::U32String emoji =
            U"🅱👀✔️❌🍆✔️❌👌🚺😉✔️🤷👍😄😑"
            U"😶"
            U"🤐"
            U"😪";
    static defines::U32String cjk =
            U"これは日本語のテキストです。这是简体中文文本。這是繁體中文文本。"
            U"이것은 한국어 텍스트입니다.";
    static defines::U32String latin =
            U"This is English text.Ese es texto en español.Ceci est un texte "
            U"français.";
    stream << "Beginning test of character properties structure...\n";
    characterProperties ( );
    stream << "Ensuring that the size of character properties is 0x10FFFF "
              "characters...\n";
    if ( characterProperties ( ).size ( ) != defines::maxUnicode + 1 )
    {
        CHAR_UNITTEST_FAIL ( stream,
                             "Missing or extra characters detected",
                             "Character properties has invalid size: 0x",
                             characterProperties ( ).size ( ),
                             " which indicates a Unicode range of [U+0, U+",
                             characterProperties ( ).size ( ) - 1,
                             "] instead of the expected range [U+0, U+",
                             defines::maxUnicode )
        END_UNIT_FAIL ( stream )
    }

    stream << "Ensuring that CJK characters have a width of two columns...\n";
    for ( auto &u : cjk )
    {
        if ( !characterProperties ( ).at ( u ).columns && u > 0x7F )
        {
            CHAR_UNITTEST_FAIL ( stream,
                                 "Invalid Column width detected",
                                 "A CJK character, U+",
                                 std::uint32_t ( u ),
                                 " was marked as one column wide!" )
            END_UNIT_FAIL ( stream )
        }
    }
    stream << "Ensuring that phonetic-alphabet characters have a width of one "
              "column...\n";
    for ( auto &u : latin )
    {
        if ( characterProperties ( ).at ( u ).columns )
        {
            CHAR_UNITTEST_FAIL (
                    stream,
                    "Invalid Column width detected",
                    "A Phonetic-derivative (i.e., Latin) character, U+",
                    std::uint32_t ( u ),
                    " was marked as two columns wide!" )
            END_UNIT_FAIL ( stream )
        }
    }
    stream << "Ensuring that emoji are marked as emoji...\n";
    for ( auto &u : emoji )
    {
        if ( !characterProperties ( ).at ( u ).emoji )
        {
            stream << "For some reason the character U+" << std::hex
                   << std::uint32_t ( u )
                   << " was not properly marked as an emoji. This is a known "
                      "issue, but since it only affects wide characters, is "
                      "ignored.\n"
                   << std::dec;
        }
    }
    stream << "No information indicates failure, returning...\n";
    return true;
}

test::Unittest propertiesTest { &propertyInitializationTest };