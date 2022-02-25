/**
 * @file externalized.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Generic Externalized Structure.
 * @version 1
 * @date 2022-02-24
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>

#include <io/base/syncstream.h++>

#include <compare>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
namespace ux::serialization
{
    inline std::strong_ordering
            translatePartial ( std::partial_ordering const order )
    {
        if ( order == std::partial_ordering::equivalent )
        {
            return std::strong_ordering::equivalent;
        }
        if ( order == std::partial_ordering::greater )
        {
            return std::strong_ordering::greater;
        }
        if ( order == std::partial_ordering::less )
        {
            return std::strong_ordering::less;
        }
        RUNTIME_ERROR (
                "Attempted to translate an unordered partial ordering to a "
                "strong ordering. The result doesn't make sense!" )
    }

    struct ExternalID
    {
        defines::IString key;
        POLYMORPHIC_IDENTIFIER ( ExternalID )
    protected:
        /**
         * @brief Attempts to order two external IDs which are children of
         * ExternalID. returns std::partial_ordering::unordered if they do not
         * know how to order each other.
         *
         * @return std::partial_ordering const
         */
        virtual std::partial_ordering const
                order ( ExternalID const & ) const noexcept
        {
            return std::partial_ordering::unordered;
        }
    public:
        ExternalID ( ) noexcept = default;
        ExternalID ( defines::IString const &key ) : key { key } { }
        std::strong_ordering const
                operator<=> ( ExternalID const &that ) const noexcept
        {
            // get our comparison
            auto strong = key <=> that.key;
            // get our weak comparison
            auto weak   = order ( that );

            // if the two have the same key...
            if ( strong == std::strong_ordering::equivalent )
            {
                // and the sub-components are not equal...
                if ( weak != std::partial_ordering::unordered )
                {
                    // return the inequality-part
                    return translatePartial ( weak );
                }
            }
            // otherwise, return the strong ordering.
            return strong;
        }
    };

    template < class T > class Externalized
    {
        // custom comparer for the internal map. Like std::less, but instead
        // uses shared pointers to the types that we want to compare.
        struct CustomLess
        {
        public:
            constexpr inline bool operator( ) (
                    std::shared_ptr< ExternalID > const &lhs,
                    std::shared_ptr< ExternalID > const &rhs ) const noexcept
            {
                return ( *lhs <=> *rhs ) == std::strong_ordering::less;
            }
        };
        std::map< std::shared_ptr< ExternalID >, T, CustomLess > contents;
    protected:
        std::map< std::shared_ptr< ExternalID >, T, CustomLess > &
                getMap ( ) noexcept
        {
            return contents;
        }

        virtual void _parse ( defines::ChrString const & ) = 0;
        virtual defines::ChrString
                  folder ( ) const noexcept = 0; // such as path for ./data/path
        virtual T defaultValue (
                std::shared_ptr< ExternalID > const & ) const = 0;
    public:
        POLYMORPHIC_IDENTIFIER ( Externalized )
        Externalized ( ) noexcept = default;
        virtual ~Externalized ( ) = default;

        void parse ( std::filesystem::path const &directory )
        {
            std::string directoryPath = directory.string ( );
            if ( !directoryPath.ends_with ( folder ( ) )
                 && !directoryPath.ends_with ( folder ( ) + "/" ) )
            {
                // I apologize for the invalid indentation here, it's
                // conditional based on which ssytem we're compiling for.
#ifdef WINDOWS
                if ( !directoryPath.ends_with ( folder ( ) + "\\" ) )
                {
#endif
                    RUNTIME_ERROR (
                            "Path is bad. Expected the last folder in the path "
                            "to be ",
                            folder ( ),
                            "but, as you can see, the path is ",
                            directoryPath,
                            " instead." )
#ifdef WINDOWS
                }
#endif
            }
            // now that we know the path is good, parse through it.
            auto iterator =
                    std::filesystem::recursive_directory_iterator ( directory );
            for ( auto &entry : iterator )
            {
                io::base::osyncstream stream { std::cout };
                stream << "Looking at file " << entry.path ( ).string ( )
                       << "\n";
                stream.emit ( );
                // if the file is a normal file and has a .json extension
                if ( entry.is_regular_file ( )
                     && entry.path ( ).string ( ).ends_with ( ".yaml" ) )
                {
                    // slurp the file then parse.
                    defines::ChrString     slurpee = "";
                    defines::ChrFileStream fstream {
                            entry.path ( ).string ( ) };
                    if ( fstream.bad ( ) )
                    {
                        RUNTIME_ERROR ( "Failed to open the file ",
                                        entry.path ( ).string ( ),
                                        " and that's all we know." )
                    }
                    for ( std::string temp = ""; !fstream.eof ( );
                          std::getline ( fstream, temp ) )
                    {
                        slurpee += temp + "\n";
                    }

                    _parse ( slurpee );
                }
            }
        }

        T get ( std::shared_ptr< ExternalID > const &id ) const
        {
            if ( contents.contains ( id ) )
            {
                return contents.at ( id );
            } else
            {
                return defaultValue ( id );
            }
        }

        void set ( std::shared_ptr< ExternalID > const &id, T t )
        {
            contents.insert_or_assign ( id, t );
        }
    };
} // namespace ux::serialization