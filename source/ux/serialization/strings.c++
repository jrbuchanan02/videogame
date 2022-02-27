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
#include <vector>

void ux::serialization::ExternalizedStrings::_parse (
        defines::ChrString const &text )
{
    YAML::Node       node     = YAML::Load ( text );
    defines::IString language = node [ "Language" ].Scalar ( );
    for ( std::size_t i = 0; i < node [ "Text" ].size ( ); i++ )
    {
        defines::ChrString rawTransliteration =
                node [ "Transliteration" ][ i ].as< defines::ChrString > ( );
        TransliterationLevel parsedTransliteration =
                defines::fromString< TransliterationLevel > (
                        rawTransliteration );
        if ( parsedTransliteration == TransliterationLevel::_MAX )
        {
            parsedTransliteration = TransliterationLevel::NOT;
            io::base::osyncstream { std::cout }
                    << "Warning: invalid transliteration level: \""
                    << rawTransliteration << "\"\n";
        }

        for ( auto item = node [ "Text" ][ i ].begin ( );
              item != node [ "Text" ][ i ].end ( );
              item++ )
        {
            defines::IString parsedString = IS ( "" );
            parsedString = item->second.as< defines::IString > ( );
            std::shared_ptr< ExternalID > key =
                    std::shared_ptr< ExternalID > ( new ExternalID ( ) );
            key->key = language + "." + item->first.as< defines::IString > ( )
                     + "."
                     + defines::rtToString< TransliterationLevel > (
                               parsedTransliteration );
            getMap ( ).insert_or_assign ( key, parsedString );
        }
    }
}