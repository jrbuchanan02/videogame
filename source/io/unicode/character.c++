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
#include <cstdint>
#include <defines/macros.h++>
#include <fstream>
#include <io/unicode/character.h++>
#include <iostream>
#include <rapidxml-1.13/rapidxml.hpp>
#include <sstream>
#include <test/unittester.h++>
#include <vector>

using namespace io::unicode;
using namespace defines;

#define MAX_UNICODE 0x10FFFF
#define DATA_FILE   "ucd.all.grouped.xml"
#define DATA_PATH   "./data/unicode/"

std::vector<CharacterProperties> properties = { };

void initializeProperties ( );

std::vector<CharacterProperties> const &io::unicode::characterProperties ( )
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
    defines::EFileStream  file ( DATA_PATH DATA_FILE );
    defines::EString      contents = ES ( "" );
    while ( !file.eof ( ) )
    {
        defines::EString temp;
        std::getline ( file, temp );
        contents += temp;
    }
    file.close ( );
    document.parse<0> ( ( defines::ECString ) contents.c_str ( ) );
    defines::EXMLNode *group = document.first_node ( "ucd" )
                                       ->first_node ( "repertoire" )
                                       ->first_node ( "group" );
    auto parseCharacterProperties = [ & ] ( defines::EXMLNode *node ) {
        auto getField =
                [ & ] ( defines::EXMLNode *p,
                        defines::EXMLNode *c,
                        defines::ECString  n ) -> defines::EXMLAttribute * {
            if ( c->first_attribute ( n ) )
                return c->first_attribute ( n );
            else if ( p->first_attribute ( n ) )
                return p->first_attribute ( n );
            else
            {
                defines::ChrString failMessage = "Missing field \"";
                failMessage += n;
                failMessage += "\"";
                throw std::runtime_error ( failMessage.c_str ( ) );
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
            defines::ECString emoji =
                    getField ( node->parent ( ), node, ES ( "Emoji" ) )
                            ->value ( );
            temp = emoji;
            if ( temp == ES ( "Y" ) )
            {
                result.columns = 1;
            }
        }

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
                    throw std::runtime_error (
                            "First code point in range exists, but no last "
                            "code point in range!" );
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
                throw std::runtime_error ( "Cannot parse node!" );
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
    if ( characterProperties ( ).size ( ) != MAX_UNICODE + 1 )
    {
        stream << "Character properties has an invalid size: 0x" << std::hex
               << characterProperties ( ).size ( );
        stream << std::dec << "\n";

        return false;
    }
    stream << "Ensuring that emoji have a width of two columns...\n";
    for ( auto &u : emoji )
    {
        if ( !characterProperties ( ).at ( u ).columns && u > 0x7F )
        {
            stream << "An emoji (U+" << std::hex << std::uint32_t ( u )
                   << std::dec << ") has a length registered as one column!\n";
            return false;
        }
    }
    stream << "Ensuring that CJK characters have a width of two columns...\n";
    for ( auto &u : cjk )
    {
        if ( !characterProperties ( ).at ( u ).columns && u > 0x7F )
        {
            stream << "A CJK character (U+" << std::hex << std::uint32_t ( u )
                   << std::dec << ") has a length registered as one column!\n";
            return false;
        }
    }
    stream << "Ensuring that phonetic-alphabet characters have a width of one "
              "column...\n";
    for ( auto &u : latin )
    {
        if ( characterProperties ( ).at ( u ).columns )
        {
            stream << "A Phonetic-alphabet character (U+" << std::hex
                   << std::uint32_t ( u ) << std::dec
                   << ") has a length registered as one column!\n";
            return false;
        }
    }
    stream << "No information indicates failure, returning...\n";
    return true;
}

test::Unittest propertiesTest { &propertyInitializationTest };