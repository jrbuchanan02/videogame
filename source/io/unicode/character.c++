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
#include <vector>
using namespace io::unicode;
using namespace defines;

#define MAX_UNICODE 0x10FFFF
#define DATA_FILE   "ucd.all.grouped.xml"
#define DATA_PATH   "./data/unicode/"

int line = __LINE__;

std::vector<CharacterProperties> properties = { };

void initializeProperties ( );

std::vector<CharacterProperties> const &io::unicode::characterProperties ( )
{
    if ( properties.empty ( ) )
    {
        try
        {
            initializeProperties ( );
        } catch ( std::bad_cast &badCast )
        {
            std::cerr << "Bad Cast detected on line " << __LINE__
                      << ", it must have occurred while initializing "
                         "properties. Last known good line was "
                      << line << "\n";
            throw badCast;
        }
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
    defines::EXMLNode *ucd = document.first_node ( ES ( "ucd" ) );
    if ( !ucd )
    {
        throw std::runtime_error (
                "The file was not the unicode character database!" );
    }
    defines::EXMLNode *repertoire = ucd->first_node ( ES ( "repertoire" ) );
    if ( !repertoire )
    {
        throw std::runtime_error ( "The repertoire isn't here!" );
    }
    defines::EXMLNode *group = repertoire->first_node ( ES ( "group" ) );
    if ( !group )
    {
        throw std::runtime_error ( "Expected grouped XML Database!" );
    }
    for ( ; group; group = group->next_sibling ( ) )
    {
        auto getProperty =
                [ & ] ( defines::EXMLNode *pref,
                        defines::EXMLNode *back,
                        defines::ECString  propName ) -> defines::ECString {
            if ( pref->first_attribute ( propName ) )
            {
                return pref->first_attribute ( propName )->value ( );
            } else if ( back->first_attribute ( propName ) )
            {
                return back->first_attribute ( propName )->value ( );
            } else
            {
                throw std::runtime_error ( "No place to get property from!" );
            }
        };

        defines::EXMLNode *character = group->first_node ( );

        if ( !character )
        {
            throw std::runtime_error ( "Empty Group!" );
        }
        // process as character.
        CharacterProperties properties;
        // only work with properties for now
        defines::ECString ea = getProperty ( character, group, ES ( "ea" ) );

        if ( ea )
        {
            defines::EString width ( ea );
            if ( width == ES ( "A" ) || width == ES ( "W" )
                 || width == ES ( "F" ) )
            {
                properties.columns = 1;
            } else
            {
                properties.columns = 0;
            }
        } else
        {
            properties.columns = 0;
        }

        defines::ECString emoji =
                getProperty ( character, group, ES ( "Emoji" ) );

        if ( emoji )
        {
            defines::EString width ( emoji );

            if ( width == ES ( "Y" ) )
            {
                properties.columns = 1;
            }
        }

        if ( character->first_attribute ( ES ( "cp" ) ) )
        {
            ::properties.push_back ( properties );
        } else
        {
            if ( !character->first_attribute ( ES ( "first-cp" ) ) )
            {
                throw std::runtime_error (
                        "Illegal Character database entry!" );
            } else
            {
                if ( !character->first_attribute ( ES ( "last-cp" ) ) )
                {
                    throw std::runtime_error (
                            "Illegal Character database entry!" );
                }

                defines::EStringStream temp;
                std::uint32_t          first = 0, last = 0;

                temp = defines::EStringStream (
                        character->first_attribute ( ES ( "first-cp" ) )
                                ->value ( ) );

                temp >> std::hex >> first;

                temp = defines::EStringStream (
                        character->first_attribute ( ES ( "last-cp" ) )
                                ->value ( ) );

                temp >> std::hex >> last;
                for ( std::uint32_t i = first; i <= last; i++ )
                {
                    ::properties.push_back ( properties );
                }
            }
        }
    }
}