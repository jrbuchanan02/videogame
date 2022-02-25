/**
 * @file strings.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implements the string externalization
 * @version 1
 * @date 2022-02-20
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <ux/serialization/strings.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>
#include <io/base/syncstream.h++>

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

void ux::serialization::ExternalizedStrings::_parse (
        defines::ChrString const &text )
{
    YAML::Node       node               = YAML::Load ( text );
    std::size_t      maxTransliteration = node [ "Transliterations" ].size ( );
    defines::IString language           = node [ "Language" ].Scalar ( );
    std::cout << "Language: " << language << "\n";
    for ( std::size_t i = 0; i < maxTransliteration; i++ )
    {
        std::cout << "here " << i << "\n";
        defines::ChrString rawTransliteration =
                node [ "transliterations" ][ i ].as< defines::ChrString > ( );
        std::cout << "here " << i << "\n";
        TransliterationLevel parsedTransliteration =
                defines::fromString< TransliterationLevel > (
                        rawTransliteration );
        std::cout << "here " << i << "\n";
        if ( parsedTransliteration == TransliterationLevel::_MAX )
        {
            std::cout << "here " << i << "\n";
            parsedTransliteration = TransliterationLevel::NOT;
            std::cout << "here " << i << "\n";
            io::base::osyncstream { std::cout }
                    << "Warning: invalid transliteration level: \""
                    << rawTransliteration << "\"\n";
        }
        std::cout << "here " << i << "\n";

        for ( auto const &item : node [ "Text" ][ i ] )
        {
            std::cout << "here " << i << "\n";
            defines::IString parsedString = IS ( "" );
            std::cout << "here " << i << "\n";
            parsedString = item.as< defines::IString > ( );
            std::cout << "here " << i << "\n";
            std::shared_ptr< StringKey > key =
                    std::shared_ptr< StringKey > ( new StringKey ( ) );
            std::cout << "here " << i << "\n";
            key->key                  = item.Tag ( );
            key->language             = language;
            key->transliterationLevel = parsedTransliteration;
            std::cout << "here " << i << "\n";
            getMap ( ).insert_or_assign ( key, parsedString );
        }
        std::cin.get ( );
    }
}