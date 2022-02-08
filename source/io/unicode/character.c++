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
#include <fstream>
#include <io/unicode/character.h++>
#include <iostream>
#include <rapidxml-1.13/rapidxml.hpp>
#include <sstream>
#include <vector>
using namespace io::unicode;

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
    XMLDoc                    document;
    std::basic_ifstream<Char> file ( DATA_PATH DATA_FILE );
    std::basic_string<Char>   contents = "";
    while ( !file.eof ( ) )
    {
        std::basic_string<Char> temp;
        std::getline ( file, temp );
        contents += temp;
    }
    file.close ( );
    document.parse<0> ( ( Char * ) contents.c_str ( ) );
    XMLNode *ucd = document.first_node ( "ucd" );
    if ( !ucd )
    {
        throw std::runtime_error (
                "The file was not the unicode character database!" );
    }
    XMLNode *repertoire = ucd->first_node ( "repertoire" );
    if ( !repertoire )
    {
        throw std::runtime_error ( "The repertoire isn't here!" );
    }
    XMLNode *group = repertoire->first_node ( "group" );
    if ( !group )
    {
        throw std::runtime_error ( "Expected grouped XML Database!" );
    }
    for ( ; group; group = group->next_sibling ( ) )
    {
        auto getProperty = [ & ] ( XMLNode *pref,
                                   XMLNode *back,
                                   Char    *propName ) -> Char * {
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

        XMLNode *character = group->first_node ( );

        if ( !character )
        {
            throw std::runtime_error ( "Empty Group!" );
        }

        // process as character.
        CharacterProperties properties;
        // only work with properties for now
        Char *ea = getProperty ( character, group, ( Char * ) "ea" );
        if ( ea )
        {
            std::string width ( ea );
            if ( width == "A" || width == "W" || width == "F" )
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
        Char *emoji = getProperty ( character, group, ( Char * ) "Emoji" );
        if ( emoji )
        {
            std::string width ( emoji );
            if ( width == "Y" )
            {
                properties.columns = 1;
            }
        }

        if ( character->first_attribute ( "cp" ) )
        {
            ::properties.push_back ( properties );
        } else
        {
            if ( !character->first_attribute ( "first-cp" ) )
            {
                throw std::runtime_error (
                        "Illegal Character database entry!" );
            } else
            {
                if ( !character->first_attribute ( "last-cp" ) )
                {
                    throw std::runtime_error (
                            "Illegal Character database entry!" );
                }
                std::basic_stringstream<Char> temp;
                std::uint32_t                 first = 0, last = 0;
                temp = std::basic_stringstream<Char> (
                        character->first_attribute ( "first-cp" )->value ( ) );
                temp >> std::hex >> first;
                temp = std::basic_stringstream<Char> (
                        character->first_attribute ( "last-cp" )->value ( ) );
                temp >> std::hex >> last;
                for ( std::uint32_t i = first; i <= last; i++ )
                {
                    ::properties.push_back ( properties );
                }
            }
        }
    }
}