/**
 * @file engine.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implements the engine things.
 * @version 1
 * @date 2022-03-05
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <engine/engine.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>

#include <ux/console/screen.h++>
#include <ux/serialization/externalized.h++>
#include <ux/serialization/screens.h++>
#include <ux/serialization/strings.h++>

#include <io/console/console.h++>

#include <filesystem>
#include <memory>
#include <set>

using namespace ux::serialization;
using namespace ux;
using namespace io::console;
using namespace engine;

struct ObjectLess;

using ObjectPtr = std::shared_ptr< Object >;
using ObjectSet = std::set< ObjectPtr, ObjectLess >;

struct ObjectLess
{
    bool const operator( ) ( ObjectPtr const &lhs,
                             ObjectPtr const &rhs ) const noexcept
    {
        return lhs->getID ( ) < rhs->getID ( );
    }
};

// graph things.
class InteractionGraph
{
    using Map = std::map< ObjectPtr, ObjectSet, ObjectLess >;

    Map interactions;
public:
    InteractionGraph ( ) noexcept                          = default;
    InteractionGraph ( InteractionGraph const & ) noexcept = default;
    InteractionGraph ( InteractionGraph && ) noexcept      = default;
    virtual ~InteractionGraph ( )                          = default;
    InteractionGraph &operator= ( InteractionGraph const & ) noexcept = default;
    InteractionGraph &operator= ( InteractionGraph && ) noexcept = default;

    void addObject ( ObjectPtr const & );
    void delObject ( ObjectPtr const & );

    void addConnection ( ObjectPtr const &, ObjectPtr const & );
    void delConnection ( ObjectPtr const &, ObjectPtr const & );
    bool hasConnection ( ObjectPtr const &, ObjectPtr const & );
};

struct engine::Engine::impl_s
{
    std::map< defines::IString, InteractionGraph > interactions;
    ObjectSet                                      objects;
    std::shared_ptr< ExternalizedStrings >         strings;
    std::shared_ptr< ExternalizedScreens >         screens;
    std::shared_ptr< Console >                     console;

    defines::IString mutable locale;
    TransliterationLevel mutable transliteration;

    std::shared_ptr< ExternalID >
            generateID ( defines::IString const & ) const noexcept;

    impl_s ( ) noexcept                = default;
    impl_s ( impl_s const & ) noexcept = delete;
    impl_s ( impl_s && ) noexcept      = default;
    virtual ~impl_s ( )                = default;
    impl_s &operator= ( impl_s const & ) noexcept = delete;
    impl_s &operator= ( impl_s && ) noexcept = default;

    void parseFrom ( std::filesystem::path const & ) noexcept;
};

struct engine::Object::impl_s
{
    static inline std::atomic< defines::ObjectIdentifier > nextID = 0;

    defines::ObjectIdentifier identifier = 0;
    std::shared_ptr< Engine > engine     = nullptr;

    impl_s ( ) noexcept;
    impl_s ( impl_s const & ) noexcept;
    impl_s ( impl_s && ) noexcept;
    virtual ~impl_s ( );
    impl_s &operator= ( impl_s const & ) noexcept;
    impl_s &operator= ( impl_s && ) noexcept;
};

void InteractionGraph::addObject ( ObjectPtr const &object )
{
    if ( !interactions.contains ( object ) )
    {
        interactions.emplace ( object, ObjectSet ( ) );
    }
}
void InteractionGraph::delObject ( ObjectPtr const &object )
{
    if ( interactions.contains ( object ) )
    {
        interactions.erase ( object );
        for ( auto &pair : interactions )
        {
            if ( pair.second.contains ( object ) )
            {
                pair.second.erase ( object );
            }
        }
    }
}

void InteractionGraph::addConnection ( ObjectPtr const &from,
                                       ObjectPtr const &to )
{
    addObject ( from );
    addObject ( to );
    interactions.at ( from ).insert ( to );
}
void InteractionGraph::delConnection ( ObjectPtr const &from,
                                       ObjectPtr const &to )
{
    addObject ( from );
    addObject ( to );
    interactions.at ( from ).erase ( to );
}
bool InteractionGraph::hasConnection ( ObjectPtr const &from,
                                       ObjectPtr const &to )
{
    addObject ( from );
    addObject ( to );
    return interactions.at ( from ).contains ( to );
}

std::shared_ptr< ExternalID > engine::Engine::impl_s::generateID (
        defines::IString const &key ) const noexcept
{
    return std::make_shared< ExternalID > (
            locale + "." + key + "."
            + defines::rtToString< TransliterationLevel > ( transliteration ) );
}

void engine::Engine::impl_s::parseFrom (
        std::filesystem::path const &path ) noexcept
{
    // assume this path leads to the data directory (for now)
    std::filesystem::path screens = path / defines::screenFolderName;
    std::filesystem::path strings = path / defines::textFolderName;

    this->screens->parse ( screens );
    this->strings->parse ( strings );
}

defines::ObjectIdentifier const &engine::Object::getID ( ) const noexcept
{
    return pimpl->identifier;
}

engine::Object::impl_s::impl_s ( ) noexcept
{
    this->identifier = nextID.fetch_add ( 1 );
}
engine::Object::impl_s::impl_s ( impl_s const &that ) noexcept
{
    this->identifier = nextID.fetch_add ( 1 );
    this->engine     = that.engine;
}
engine::Object::impl_s::impl_s ( impl_s && ) noexcept = default;
engine::Object::impl_s::~impl_s ( )                   = default;
engine::Object::impl_s &
        engine::Object::impl_s::operator= ( impl_s const &that ) noexcept
{
    this->engine = that.engine;
    return *this;
}
engine::Object::impl_s &
        engine::Object::impl_s::operator= ( impl_s && ) noexcept = default;