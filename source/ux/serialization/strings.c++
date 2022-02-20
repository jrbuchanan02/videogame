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

struct ux::serialization::ExternalizedStrings::impl_s
{
    std::map< StringKey, defines::IString > contents;

    impl_s ( )  = default;
    ~impl_s ( ) = default;
};

ux::serialization::ExternalizedStrings::ExternalizedStrings ( ) noexcept :
        pimpl ( new impl_s ( ) )
{ }

ux::serialization::ExternalizedStrings::ExternalizedStrings (
        std::filesystem::path const &path ) :
        ExternalizedStrings ( )
{
    // step 1: this path should lead to data/text. ensure that
    // it ends in the proper names.

    bool validPath = path.u32string ( ).ends_with ( U"data/text" );
#ifdef WINDOWS
    validPath |= path.u32string ( ).ends_with ( U"data\\text" );
#endif
    if ( !validPath )
    {
#ifdef WINDOWS
        defines::ChrString failMessage = " is not data/text or data\\text!";
#else
        defines::ChrString failMessage = " is not data/text!";
#endif
        RUNTIME_ERROR ( ( defines::ChrCString ) path.u8string ( ).c_str ( ),
                        failMessage )
    }

    auto directoryEntries =
            std::filesystem::recursive_directory_iterator ( path );
    for ( auto &entry : directoryEntries )
    {
        if ( entry.is_regular_file ( ) )
        {
            // check if the entry is a .json file.
            auto path = entry.path ( );
            if ( path.extension ( ).u32string ( ) == U".json" )
            {
                defines::EString     filename = path.string ( );
                defines::EFileStream fstream =
                        defines::EFileStream ( filename );
                defines::EString slurped = ES ( "" );
                if ( fstream.bad ( ) )
                {
                    RUNTIME_ERROR ( "Failed to open file: ", filename );
                }
                while ( !fstream.eof ( ) )
                {
                    defines::EString temp;
                    std::getline ( fstream, temp );
                    slurped += temp;
                }
                rapidjson::Document document;
                document.Parse ( ( defines::ChrCString ) slurped.c_str ( ) );

                // iterate
                std::size_t maxTranslit =
                        document [ "transliterations" ].Size ( );
                defines::IString language =
                        document [ "language" ].GetString ( );
                for ( std::size_t i = 0; i < maxTranslit; i++ )
                {
                    TransliterationLevel parsed = TransliterationLevel::NOT;
                    defines::EString     value =
                            document [ "transliterations" ][ i ].GetString ( );
                    if ( value == "ALT" )
                    {
                        parsed = TransliterationLevel::ALT;
                    } else if ( value == "YES" )
                    {
                        parsed = TransliterationLevel::YES;
                    } else if ( value == "NOT" )
                    {
                        parsed = TransliterationLevel::NOT;
                    } else
                    {
                        io::base::osyncstream { std::cout }
                                << "Warning: invalid key " << value << "\n";
                    }
                    // I find the lack of references ... disturbing
                    auto items = document [ "text" ][ i ]
                                         .GetObject ( )
                                         .MemberBegin ( );
                    for ( ; items
                            != document [ "text" ][ i ]
                                       .GetObject ( )
                                       .MemberEnd ( );
                          items++ )
                    {
                        defines::IString parsedString = IS ( "" );
                        if ( items->value.IsString ( ) )
                        {
                            parsedString = items->value.GetString ( );
                        } else
                        {
                            for ( std::size_t i = 0; i < items->value.Size ( );
                                  i++ )
                            {
                                parsedString += items->value.GetArray ( ) [ i ]
                                                        .GetString ( );
                            }
                        }
                        set ( StringKey { language,
                                          items->name.GetString ( ),
                                          parsed },
                              parsedString );
                    }
                }
            }
        }
    }
}

ux::serialization::ExternalizedStrings::~ExternalizedStrings ( ) = default;

void ux::serialization::ExternalizedStrings::set (
        StringKey const        &key,
        defines::IString const &value )
{
    pimpl->contents.insert_or_assign ( key, value );
}

defines::IString const ux::serialization::ExternalizedStrings::get (
        StringKey const &key ) const noexcept
{
    if ( pimpl->contents.contains ( key ) )
    {
        return pimpl->contents.at ( key );
    } else
    {
        if ( key.level == TransliterationLevel::YES )
        {
            if ( pimpl->contents.contains (
                         StringKey { key.language,
                                     key.text,
                                     TransliterationLevel::ALT } ) )
            {
                return get ( StringKey { key.language,
                                         key.text,
                                         TransliterationLevel::ALT } );
            } else if ( pimpl->contents.contains (
                                StringKey { key.language,
                                            key.text,
                                            TransliterationLevel::NOT } ) )
            {
                return get ( StringKey { key.language,
                                         key.text,
                                         TransliterationLevel::NOT } );
            }
        } else if ( key.level == TransliterationLevel::ALT )
        {
            if ( pimpl->contents.contains (
                         StringKey { key.language,
                                     key.text,
                                     TransliterationLevel::YES } ) )
            {
                return get ( StringKey { key.language,
                                         key.text,
                                         TransliterationLevel::YES } );
            } else if ( pimpl->contents.contains (
                                StringKey { key.language,
                                            key.text,
                                            TransliterationLevel::NOT } ) )
            {
                return get ( StringKey { key.language,
                                         key.text,
                                         TransliterationLevel::NOT } );
            }
        } else
        {
            if ( pimpl->contents.contains (
                         StringKey { key.language,
                                     key.text,
                                     TransliterationLevel::YES } ) )
            {
                return get ( StringKey { key.language,
                                         key.text,
                                         TransliterationLevel::YES } );
            } else if ( pimpl->contents.contains (
                                StringKey { key.language,
                                            key.text,
                                            TransliterationLevel::ALT } ) )
            {
                return get ( StringKey { key.language,
                                         key.text,
                                         TransliterationLevel::ALT } );
            }
        }
    }

    return IS ( "!" ) + key.language + IS ( "." ) + key.text + IS ( "." )
         + toString ( key.level ) + IS ( "!" );
}