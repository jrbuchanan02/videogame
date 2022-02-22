/**
 * @file screen.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implementation for the screen and all its stuff.
 * @version 1
 * @date 2022-02-22
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <ux/console/screen.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <io/console/colors/color.h++>
#include <io/console/colors/direct.h++>
#include <io/console/colors/indirect.h++>
#include <io/console/conmanip.h++>
#include <io/console/console.h++>

#include <io/console/manip/stringfunctions.h++>

#include <sstream>
#include <stdexcept>
#include <string>

#include <list>
#include <memory>

struct ux::console::Screen::ColorBlock::impl_s
{
    std::shared_ptr< io::console::colors::IColor > colors [ 8 ] = {
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
    };

    impl_s ( ) noexcept                = default;
    impl_s ( impl_s const & ) noexcept = default;
    impl_s ( impl_s && ) noexcept      = default;
    ~impl_s ( )                        = default;
    impl_s &operator= ( impl_s const & ) noexcept = default;
    impl_s &operator= ( impl_s && ) noexcept = default;

    impl_s ( std::shared_ptr< io::console::colors::IColor > const
                     colors [ 8 ] ) noexcept
    {
        for ( std::size_t i = 0; i < 8; i++ )
        {
            this->colors [ i ] = colors [ i ];
        }
    }
};

struct ux::console::Screen::Line::impl_s
{
    std::shared_ptr< Screen >           parent      = nullptr;
    defines::IString                    key         = "";
    std::shared_ptr< ColorBlock >       colors      = nullptr;
    std::uint8_t                        wrap   : 1  = 0;
    std::uint8_t                        center : 1  = 0;
    std::uint8_t                        block  : 1  = 0;
    std::uint8_t                        input  : 1  = 0;
    std::uint64_t                       tickRate    = 100;
    std::shared_ptr< defines::EString > destination = nullptr;
    std::shared_ptr< IInputFormat >     format      = nullptr;
    std::shared_ptr< Line >             correction  = nullptr;

    impl_s ( ) noexcept                = default;
    impl_s ( impl_s const & ) noexcept = default;
    impl_s ( impl_s && ) noexcept      = default;
    ~impl_s ( )                        = default;

    impl_s &operator= ( impl_s const & ) noexcept = default;
    impl_s &operator= ( impl_s && ) noexcept = default;

    impl_s ( std::shared_ptr< Screen > const           &parent,
             defines::IString const                    &key,
             std::shared_ptr< ColorBlock > const       &colors,
             bool const                                &wrap,
             bool const                                &center,
             std::uint64_t const                       &tickRate,
             bool const                                &block,
             std::shared_ptr< defines::EString > const &dest,
             std::shared_ptr< IInputFormat > const     &fmt,
             std::shared_ptr< Line > const             &correction ) noexcept :
            parent { parent },
            key { key }, colors { colors }, wrap { wrap }, center { center },
            block { block }, input { dest != nullptr }, tickRate { tickRate },
            destination { dest }, format { format }, correction { correction }
    { }
};

struct ux::console::Screen::impl_s
{
    defines::IString                    language = "";
    serialization::TransliterationLevel translit =
            serialization::TransliterationLevel::NOT;
    std::shared_ptr< ColorBlock > colors =
            std::shared_ptr< ColorBlock > ( new ColorBlock ( ) );
    std::list< Line > lines       = { };
    std::list< Line > corrections = { };
};

ux::console::Screen::ColorBlock::ColorBlock ( ) noexcept :
        pimpl ( new impl_s ( ) )
{ }
ux::console::Screen::ColorBlock::ColorBlock ( ColorBlock const &that ) noexcept
        :
        pimpl ( new impl_s ( *that.pimpl ) )
{ }
ux::console::Screen::ColorBlock::ColorBlock ( ColorBlock &&that ) noexcept :
        pimpl ( new impl_s ( *std::move ( that.pimpl ) ) )
{ }
ux::console::Screen::ColorBlock::ColorBlock (
        std::shared_ptr< io::console::colors::IColor > const
                colors [ 8 ] ) noexcept :
        pimpl ( new impl_s ( colors ) )
{ }
ux::console::Screen::ColorBlock::~ColorBlock ( ) = default;

ux::console::Screen::ColorBlock::operator io::console::ConsoleManipulator ( )
        const noexcept
{
    return [ & ] ( io::console::Console &console ) -> io::console::Console & {
        for ( std::size_t i = 0; i < 8; i++ )
        {
            if ( pimpl->colors [ i ] )
            {
                console.setScreenColor ( i, pimpl->colors [ i ] );
            }
        }
        return console;
    };
}

ux::console::Screen::Line::Line ( ) noexcept : pimpl ( new impl_s ( ) ) { }
ux::console::Screen::Line::Line ( Line const &that ) noexcept :
        pimpl ( new impl_s ( *that.pimpl ) )
{ }
ux::console::Screen::Line::Line ( Line &&that ) noexcept :
        pimpl ( new impl_s ( std::move ( *that.pimpl ) ) )
{ }
ux::console::Screen::Line::~Line ( ) = default;

ux::console::Screen::Line &
        ux::console::Screen::Line::operator= ( Line const &that ) noexcept
{
    pimpl.release ( );
    pimpl.reset ( new impl_s ( *that.pimpl ) );
    return *this;
}

ux::console::Screen::Line &
        ux::console::Screen::Line::operator= ( Line &&that ) noexcept
{
    pimpl.release ( );
    pimpl.reset ( new impl_s ( std::move ( *that.pimpl ) ) );
    return *this;
}

ux::console::Screen::Line::Line (
        std::shared_ptr< Screen > const           &parent,
        defines::IString const                    &key,
        std::shared_ptr< ColorBlock > const       &colors,
        bool const                                &wrap,
        bool const                                &center,
        std::uint64_t const                       &tickRate,
        bool const                                &block,
        std::shared_ptr< defines::EString > const &dest,
        std::shared_ptr< IInputFormat > const     &fmt,
        std::shared_ptr< Line > const             &correction ) noexcept :
        pimpl ( new impl_s ( parent,
                             key,
                             colors,
                             wrap,
                             center,
                             tickRate,
                             block,
                             dest,
                             fmt,
                             correction ) )
{ }

ux::console::Screen::Line::operator io::console::ConsoleManipulator ( )
        const noexcept
{
    return [ & ] ( io::console::Console &console ) -> io::console::Console & {
        bool inputAttained = false;
        if ( !pimpl->colors )
        {
            console << *( pimpl->parent->pimpl->colors );
        } else
        {
            console << *( pimpl->colors );
        }
        if ( pimpl->wrap )
        {
            console << io::console::doTextWrapping;
        } else
        {
            console << io::console::noTextWrapping;
        }
        if ( pimpl->center )
        {
            console << io::console::doTextCenter;
        } else
        {
            console << io::console::noTextCenter;
        }
        if ( pimpl->block )
        {
            console << io::console::doWaitForText;
        } else
        {
            console << io::console::noWaitForText;
        }
        console << io::console::textDelay ( pimpl->tickRate );
        console << Screen::source.get (
                serialization::StringKey { pimpl->parent->pimpl->language,
                                           pimpl->key,
                                           pimpl->parent->pimpl->translit } );
        if ( !pimpl->input )
        {
            return console;
        } else
        {
            do {
                defines::ChrString temp;
                std::getline ( std::cin, temp );
                if ( pimpl->format->matches ( temp ) )
                {
                    *pimpl->destination = temp;
                    inputAttained       = true;
                } else
                {
                    console << *pimpl->correction;
                }
            } while ( !inputAttained );
        }
        return console;
    };
}

ux::console::Screen::Screen ( ) noexcept : pimpl ( new impl_s ( ) ) { }
ux::console::Screen::Screen ( Screen const &that ) noexcept :
        pimpl ( new impl_s ( *that.pimpl ) )
{ }
ux::console::Screen::Screen ( Screen &&that ) noexcept :
        pimpl ( new impl_s ( std::move ( *that.pimpl ) ) )
{ }
ux::console::Screen::~Screen ( ) = default;

ux::console::Screen &
        ux::console::Screen::operator= ( Screen const &that ) noexcept
{
    pimpl.reset ( new impl_s ( *that.pimpl ) );
    return *this;
}
ux::console::Screen &ux::console::Screen::operator= ( Screen &&that ) noexcept
{
    pimpl.reset ( new impl_s ( std::move ( *that.pimpl ) ) );
    return *this;
}

void ux::console::Screen::pushLine ( Line const &line,
                                     bool const &correction ) noexcept
{
    if ( correction )
    {
        pimpl->corrections.push_back ( line );
    } else
    {
        pimpl->lines.push_back ( line );
    }
}

void ux::console::Screen::pullLine ( bool const &correction ) noexcept
{
    if ( correction )
    {
        pimpl->corrections.pop_back ( );
    } else
    {
        pimpl->lines.pop_back ( );
    }
}

std::list< ux::console::Screen::Line >::iterator ux::console::Screen::begin ( )
{
    return pimpl->lines.begin ( );
}

std::list< ux::console::Screen::Line >::iterator ux::console::Screen::end ( )
{
    return pimpl->lines.end ( );
}

ux::console::Screen::operator io::console::ConsoleManipulator ( ) const noexcept
{
    return [ & ] ( io::console::Console &console ) -> io::console::Console & {
        console << *pimpl->colors;
        for ( auto &line : pimpl->lines ) { console << line; }
        return console;
    };
}