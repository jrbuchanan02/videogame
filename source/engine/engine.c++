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

using ObjectPtr = std::shared_ptr< Object >;
using ObjectSet = std::set< ObjectPtr, ObjectLess >;
struct ObjectLess
{
    constexpr bool const operator( ) ( ObjectPtr const &lhs,
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
    InteractionGraph ( ) noexcept;
    InteractionGraph ( InteractionGraph const & ) noexcept;
    InteractionGraph ( InteractionGraph && ) noexcept;
    virtual ~InteractionGraph ( ) noexcept;
    InteractionGraph &operator= ( InteractionGraph const & ) noexcept;
    InteractionGraph &operator= ( InteractionGraph && ) noexcept;

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

    defines::IString     mutable locale;
    TransliterationLevel mutable transliteration;
};