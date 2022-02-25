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

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

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
    rapidjson::Document   document;
    io::base::osyncstream stream { std::cout };
    stream << "Looking at text: \"" << text << "\"\n";
    stream.emit ( );

    document.Parse ( text.c_str ( ) );
    std::size_t maxTransliteration = document [ "transliterations" ].Size ( );
    stream << "Loaded transliteration count.\n";
    stream.emit ( );
    defines::IString language =
            document.FindMember ( "language" )->value.GetString ( );

    for ( std::size_t i = 0; i < maxTransliteration; i++ )
    {
        defines::ChrString rawTransliteration {
                document [ "transliterations" ][ i ].GetString ( ) };
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

        try
        {
            for ( auto item = document.GetObject ( ) [ "text" ][ i ]
                                      .GetObject ( )
                                      .MemberBegin ( );
                  item
                  != document.GetObject ( ) [ "text" ]
                             .GetObject ( )
                             .MemberEnd ( );
                  item++ )
            {
                defines::IString parsedString = IS ( "" );
                if ( item->value.IsString ( ) )
                {
                    parsedString = item->value.GetString ( );
                } else
                {
                    for ( std::size_t i = 0; i < item->value.Size ( ); i++ )
                    {
                        parsedString +=
                                item->value.GetArray ( ) [ i ].GetString ( );
                    }
                }
                std::shared_ptr< StringKey > key =
                        std::shared_ptr< StringKey > ( new StringKey ( ) );
                key->key                  = item->name.GetString ( );
                key->language             = language;
                key->transliterationLevel = parsedTransliteration;
                getMap ( ).insert_or_assign ( key, parsedString );
            }
        } catch ( ... )
        {
            io::base::osyncstream stream { std::cout };
            stream << "Noticed assertion failure. Attempting to parse index "
                   << ( i + 1 ) << " of document.text.\n";
            throw;
        }
    }
}