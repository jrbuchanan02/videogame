/**
 * @file io.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Handles the implementation of the io things.
 * @version 1
 * @date 2022-03-07
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <ux/console/io.h++>

#include <io/console/colors/color.h++>
#include <io/console/conmanip.h++>
#include <io/console/console.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/manip.h++>
#include <defines/types.h++>

#include <ux/serialization/externalized.h++>
#include <ux/serialization/strings.h++>

#include <io/base/syncstream.h++>

#include <any>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

using namespace io::console;
using namespace io::console::colors;
using namespace ux::serialization;

using InputGetter = std::function< std::any ( std::istream & ) >;

struct ux::console::Line::impl_s
{
    defines::IString textID              = "";
    std::uint64_t    txtRate             = 17;
    std::uint64_t    cmdRate             = 100;
    defines::Flag    centered        : 1 = 0;
    defines::Flag    wrapped         : 1 = 0;
    defines::Flag    bold            : 1 = 0;
    defines::Flag    faint           : 1 = 0;
    defines::Flag    italic          : 1 = 0;
    defines::Flag    underline       : 1 = 0;
    defines::Flag    slowBlink       : 1 = 0;
    defines::Flag    fastBlink       : 1 = 0;
    defines::Flag    invert          : 1 = 0;
    defines::Flag    hide            : 1 = 0;
    defines::Flag    strike          : 1 = 0;
    defines::Flag    font            : 4 = 0;
    defines::Flag    fraktur         : 1 = 0;
    defines::Flag    doubleUnderline : 1 = 0;
    // 32-bit values for the color.
    // 0-7 mean this value is a CGA color
    // 8 means this is the default color
    // 9 (in the lowest byte) means the second highest byte
    // is the 256-color color
    // 10 means that the higher 3 bytes represent the
    // rrggbb color value.
    // Any value > 10 in the lowest byte is interpreted as
    // the alpha value. As a result, it is not possible to
    // have an alpha <= 10.
    std::uint32_t    foreground          = 7;
    std::uint32_t    background          = 0;

    impl_s ( ) noexcept                = default;
    impl_s ( impl_s const & ) noexcept = default;
    impl_s ( impl_s && ) noexcept      = default;
    virtual ~impl_s ( )                = default;
    impl_s &operator= ( impl_s const & ) noexcept = default;
    impl_s &operator= ( impl_s && ) noexcept = default;
};

struct ux::console::IOThing::impl_s
{
    std::map< std::size_t, std::shared_ptr< IColor > > palette;
    std::list< std::shared_ptr< IOThing > >            nextOptions;

    // has to be explicitly defined.
    impl_s ( ) noexcept;
    impl_s ( impl_s const & ) noexcept = default;
    impl_s ( impl_s && ) noexcept      = default;
    virtual ~impl_s ( )                = default;
    impl_s &operator= ( impl_s const & ) noexcept = default;
    impl_s &operator= ( impl_s && ) noexcept = default;
};

struct ux::console::Screen::impl_s
{
    std::list< Line >        lines;
    std::shared_ptr< Input > input;

    impl_s ( ) noexcept;
    impl_s ( impl_s const & ) noexcept;
    impl_s ( impl_s && ) noexcept;
    virtual ~impl_s ( );
    impl_s &operator= ( impl_s const & ) noexcept;
    impl_s &operator= ( impl_s && ) noexcept;
};

struct ux::console::Input::impl_s
{
    InputMode                 mode;
    std::any                  value;
    std::shared_ptr< Screen > wrongAnswer;

    impl_s ( ) noexcept;
    impl_s ( impl_s const & ) noexcept;
    impl_s ( impl_s && ) noexcept;
    virtual ~impl_s ( );
    impl_s &operator= ( impl_s const & ) noexcept;
    impl_s &operator= ( impl_s && ) noexcept;
};

ux::console::Line::Line ( ) noexcept : pimpl ( new impl_s ( ) )
{
    setTextID ( "EmptyString" );
    setTxtRate ( 17 );
    setCmdRate ( 100 );
    setCentered ( false );
    setWrapped ( false );
    setBold ( false );
    setItalic ( false );
    setUnderline ( false );
    setSlowBlink ( false );
    setFastBlink ( false );
    setInvert ( false );
    setHide ( false );
    setStrike ( false );
    setFont ( 0 );
    setFraktur ( false );
    setDoubleUnderline ( false );
    setForegroundColor ( 0x00000007 );
    setBackgroundColor ( 0x00000000 );
}
ux::console::Line::Line ( Line const &that ) noexcept :
        pimpl ( new impl_s ( *that.pimpl ) )
{ }
ux::console::Line::Line ( Line &&that ) noexcept :
        pimpl ( std::move ( that.pimpl ) )
{ }
ux::console::Line::~Line ( )                  = default;
ux::console::Line &ux::console::Line::operator= ( Line const &that ) noexcept
{
    pimpl.reset ( new impl_s ( *that.pimpl ) );
    return *this;
}
ux::console::Line &ux::console::Line::operator= ( Line &&that ) noexcept
{
    pimpl.reset ( new impl_s ( *that.pimpl ) );
    that.pimpl.release ( );
    return *this;
}

void ux::console::Line::setTextID ( defines::IString const &id ) noexcept
{
    pimpl->textID = id;
}
void ux::console::Line::setTxtRate ( std::uint64_t const &rate ) noexcept
{
    pimpl->txtRate = rate;
}
void ux::console::Line::setCmdRate ( std::uint64_t const &rate ) noexcept
{
    pimpl->cmdRate = rate;
}
void ux::console::Line::setCentered ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->centered = 1;
    } else
    {
        pimpl->centered = 0;
    }
}
void ux::console::Line::setWrapped ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->wrapped = 1;
    } else
    {
        pimpl->wrapped = 0;
    }
}
void ux::console::Line::setBold ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->bold = 1;
    } else
    {
        pimpl->bold = 0;
    }
}
void ux::console::Line::setFaint ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->faint = 1;
    } else
    {
        pimpl->faint = 0;
    }
}
void ux::console::Line::setItalic ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->italic = 1;
    } else
    {
        pimpl->italic = 0;
    }
}
void ux::console::Line::setUnderline ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->underline = 1;
    } else
    {
        pimpl->underline = 0;
    }
}
void ux::console::Line::setSlowBlink ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->slowBlink = 1;
    } else
    {
        pimpl->slowBlink = 0;
    }
}
void ux::console::Line::setFastBlink ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->fastBlink = 1;
    } else
    {
        pimpl->fastBlink = 0;
    }
}
void ux::console::Line::setInvert ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->invert = 1;
    } else
    {
        pimpl->invert = 0;
    }
}
void ux::console::Line::setHide ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->hide = 1;
    } else
    {
        pimpl->hide = 0;
    }
}
void ux::console::Line::setStrike ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->strike = 1;
    } else
    {
        pimpl->strike = 0;
    }
}
void ux::console::Line::setFont ( std::uint8_t const &font ) noexcept
{
    if ( font > 9 )
    {
        setFont ( font % 10 );
    } else
    {
        pimpl->font = font;
    }
}
void ux::console::Line::setFraktur ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->fraktur = 1;
    } else
    {
        pimpl->fraktur = 0;
    }
}
void ux::console::Line::setDoubleUnderline ( bool const &value ) noexcept
{
    if ( value )
    {
        pimpl->doubleUnderline = 1;
    } else
    {
        pimpl->doubleUnderline = 0;
    }
}
void ux::console::Line::setForegroundColor (
        std::uint32_t const &color ) noexcept
{
    pimpl->foreground = color;
}
void ux::console::Line::setBackgroundColor (
        std::uint32_t const &color ) noexcept
{
    pimpl->background = color;
}

SGRCommand fixColor ( SGRCommand, SGRCommand, SGRCommand );

void parseColor ( Console &, std::uint32_t const &, bool const & );

ConsoleManipulator ux::console::Line::operator( ) (
        std::shared_ptr< ExternalizedStrings > const &strings,
        std::shared_ptr< defines::IString > const    &locale,
        TransliterationLevel const                   &translit )
{
    return [ & ] ( Console &console ) -> Console & {
        console << resetSGR;
        console << textDelay ( pimpl->txtRate );
        console << commandDelay ( pimpl->cmdRate );

        // centering text affects this flag, so set this flag first.
        if ( pimpl->wrapped )
        {
            console << doTextWrapping;
        } else
        {
            console << noTextWrapping;
        }

        if ( pimpl->centered )
        {
            console << doTextCenter;
        } else
        {
            console << noTextCenter;
        }

        if ( pimpl->bold )
        {
            console << doSGR ( SGRCommand::BOLD );
        }
        if ( pimpl->faint )
        {
            console << doSGR ( SGRCommand::FAINT );
        }
        if ( pimpl->italic )
        {
            console << doSGR ( SGRCommand::ITALIC );
        }
        if ( pimpl->underline )
        {
            console << doSGR ( SGRCommand::UNDERLINE );
        }
        if ( pimpl->slowBlink )
        {
            console << doSGR ( SGRCommand::SLOW_BLINK );
        }
        if ( pimpl->fastBlink )
        {
            console << doSGR ( SGRCommand::FAST_BLINK );
        }
        if ( pimpl->invert )
        {
            console << doSGR ( SGRCommand::INVERT );
        }
        if ( pimpl->hide )
        {
            console << doSGR ( SGRCommand::HIDE );
        }
        if ( pimpl->strike )
        {
            console << doSGR ( SGRCommand::STRIKE );
        }
        if ( pimpl->fraktur )
        {
            console << doSGR ( SGRCommand::FRAKTUR );
        }
        if ( pimpl->doubleUnderline )
        {
            console << doSGR ( SGRCommand::DOUBLE_UNDERLINE );
        }

        switch ( pimpl->font )
        {
            case 9: console << doSGR ( SGRCommand::ALT_FONT_9 ); break;
            case 8: console << doSGR ( SGRCommand::ALT_FONT_8 ); break;
            case 7: console << doSGR ( SGRCommand::ALT_FONT_7 ); break;
            case 6: console << doSGR ( SGRCommand::ALT_FONT_6 ); break;
            case 5: console << doSGR ( SGRCommand::ALT_FONT_5 ); break;
            case 4: console << doSGR ( SGRCommand::ALT_FONT_4 ); break;
            case 3: console << doSGR ( SGRCommand::ALT_FONT_3 ); break;
            case 2: console << doSGR ( SGRCommand::ALT_FONT_2 ); break;
            case 1: console << doSGR ( SGRCommand::ALT_FONT_1 ); break;
            case 0: console << doSGR ( SGRCommand::PRIMARY_FONT ); break;
            default: console << doSGR ( SGRCommand::PRIMARY_FONT ); break;
        }

        parseColor ( console, pimpl->background, true );
        parseColor ( console, pimpl->foreground, false );

        defines::IString key =
                *locale + "." + pimpl->textID + "."
                + defines::rtToString< TransliterationLevel > ( translit );
        std::shared_ptr< ExternalID > id { new ExternalID ( key ) };
        console << doWaitForText;
        console << strings->get ( id );
        return console;
    };
}

void ux::console::IOThing::secretSauce (
        Console &,
        std::shared_ptr< serialization::ExternalizedStrings > const &,
        std::shared_ptr< defines::IString > const &,
        serialization::TransliterationLevel const & )
{
    return;
}

ux::console::IOThing::IOThing ( ) noexcept : pimpl ( new impl_s ( ) ) { }
ux::console::IOThing::IOThing ( IOThing const &that ) noexcept :
        pimpl ( new impl_s ( *that.pimpl ) )
{ }
ux::console::IOThing::IOThing ( IOThing &&that ) noexcept :
        pimpl ( std::move ( that.pimpl ) )
{ }
ux::console::IOThing::~IOThing ( ) = default;
ux::console::IOThing &
        ux::console::IOThing::operator= ( IOThing const &that ) noexcept
{
    pimpl.reset ( new impl_s ( *that.pimpl ) );
    return *this;
}
ux::console::IOThing &
        ux::console::IOThing::operator= ( IOThing &&that ) noexcept
{
    pimpl = std::move ( that.pimpl );
    return *this;
}

io::console::ConsoleManipulator ux::console::IOThing::operator( ) (
        std::shared_ptr< ExternalizedStrings > const &strings,
        std::shared_ptr< defines::IString > const    &locale,
        TransliterationLevel const                   &translit )
{
    return [ & ] ( Console &console ) -> Console & {
        for ( auto const &pair : pimpl->palette )
        {
            if ( pair.first > 7 )
            {
                console.setCalculationColor ( pair.first, pair.second );
            } else
            {
                console.setScreenColor ( pair.first, pair.second );
            }
        }
        secretSauce ( console, strings, locale, translit );
        return console;
    };
}

std::list< std::shared_ptr< ux::console::IOThing > > const &
        ux::console::IOThing::availableScreens ( ) const noexcept
{
    return pimpl->nextOptions;
}

bool ux::console::IOThing::addOption (
        std::shared_ptr< IOThing > const &option ) noexcept
{
    for ( auto const &existing : pimpl->nextOptions )
    {
        if ( existing == option )
        {
            return false;
        }
    }
    pimpl->nextOptions.push_back ( option );
    return true;
}

void ux::console::Screen::secretSauce (
        Console                                      &console,
        std::shared_ptr< ExternalizedStrings > const &strings,
        std::shared_ptr< defines::IString > const    &locale,
        TransliterationLevel const                   &translit )
{
    for ( auto &line : pimpl->lines )
    {
        console << ( line ( strings, locale, translit ) );
    }

    // stupid hack. The polymorphism within IOThing resides entirely within
    // the function call operator overload. Unfortunately, since none of the
    // subclasses of IOThing overload a function call opeator (since it would
    // just call the superclass version anyways), we have to explicitly grab
    // the input as a pointer to an IOThing.
    if ( pimpl->input )
    {
        IOThing *actuallyItsInput = pimpl->input.get ( );
        console << ( actuallyItsInput->operator( ) ( strings,
                                                     locale,
                                                     translit ) );
    }
}

ux::console::Screen::Screen ( ) noexcept : IOThing ( ), pimpl ( new impl_s ( ) )
{ }
ux::console::Screen::Screen ( Screen const &that ) noexcept :
        IOThing ( that ), pimpl ( new impl_s ( *that.pimpl ) )
{ }
ux::console::Screen::Screen ( Screen &&that ) noexcept :
        IOThing ( std::move ( that ) ), pimpl ( std::move ( that.pimpl ) )
{ }
ux::console::Screen::~Screen ( ) = default;
ux::console::Screen &
        ux::console::Screen::operator= ( Screen const &that ) noexcept
{
    this->IOThing::operator= ( that );
    pimpl.reset ( new impl_s ( *that.pimpl ) );
    return *this;
}
ux::console::Screen &ux::console::Screen::operator= ( Screen &&that ) noexcept
{
    this->IOThing::operator= ( std::move ( that ) );

    pimpl = std::move ( that.pimpl );
    return *this;
}

void ux::console::Screen::setPrompt (
        std::shared_ptr< Input > const &input ) noexcept
{
    pimpl->input = input;
}
std::shared_ptr< ux::console::Input > const &
        ux::console::Screen::getPrompt ( ) noexcept
{
    return pimpl->input;
}

InputGetter chooseInputFunction ( ux::console::InputMode const & );

void ux::console::Input::secretSauce (
        Console                                      &console,
        std::shared_ptr< ExternalizedStrings > const &strings,
        std::shared_ptr< defines::IString > const    &locale,
        TransliterationLevel const                   &translit )
{
    // this secret sauce function is arguably the most involved of the three
    // since we have to parse any given form of input that Videogame knows how
    // to parse. We also have to deal with std::cin.

    // right now, ensure that the console has finished printing before
    // proceeding.
    while ( console.printing ( ) )
    {
        std::this_thread::sleep_for ( std::chrono::milliseconds ( 1 ) );
    }
    // now that the prompt is out there, clear std::cin
    while ( std::cin.peek ( ) != std::char_traits< char >::eof ( ) )
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
        int _ = std::cin.get ( );
#pragma GCC diagnostic pop
    }
    // choose our input function
    InputGetter inputFunction = chooseInputFunction ( pimpl->mode );
    pimpl->value              = nullptr;
    while ( !pimpl->value.has_value ( ) )
    {
        pimpl->value = inputFunction ( std::cin );

        if ( !pimpl->value.has_value ( ) )
        {
            // if we can remind the user, then do so. This maneuver is
            // almost identical to the one within screen which called
            // this function.
            if ( pimpl->wrongAnswer )
            {
                IOThing *actuallyItsScreen = pimpl->wrongAnswer.get ( );
                console << ( actuallyItsScreen->operator( ) ( strings,
                                                              locale,
                                                              translit ) );
            }
        }
    }
    // all done.
}

ux::console::Input::Input ( ) noexcept : pimpl ( new impl_s ( ) ) { }
ux::console::Input::Input ( Input const &that ) noexcept :
        pimpl ( new impl_s ( *that.pimpl ) )
{ }
ux::console::Input::Input ( Input &&that ) noexcept :
        pimpl ( std::move ( that.pimpl ) )
{ }
ux::console::Input::~Input ( ) = default;

ux::console::Input &ux::console::Input::operator= ( Input const &that ) noexcept
{
    pimpl.reset ( *that.pimpl );
    return *this;
}
ux::console::Input &ux::console::Input::operator= ( Input &&that ) noexcept
{
    pimpl = std::move ( that.pimpl );
    return *this;
}

ux::console::InputMode const ux::console::Input::getInputMode ( ) const noexcept
{
    return pimpl->mode;
}
std::any const ux::console::Input::getInput ( ) const noexcept
{
    return pimpl->value;
}

void parseColor ( Console             &console,
                  std::uint32_t const &color,
                  bool const          &offset )
{
    std::function< ConsoleManipulator ( SGRCommand ) >           cga;
    std::function< ConsoleManipulator ( std::uint8_t const & ) > vga;
    std::function< ConsoleManipulator ( std::uint8_t const &,
                                        std::uint8_t const &,
                                        std::uint8_t const & ) >
            trueColor;

    cga = doSGR;
    if ( offset )
    {
        vga       = setBackground256;
        trueColor = setBackgroundTrue;
    } else
    {
        vga       = setForeground256;
        trueColor = setForegroundTrue;
    }

    // parse CGA and defualt color
    if ( color <= 8 )
    {
        std::uint8_t starting =
                offset ? ( std::uint8_t ) SGRCommand::CGA_BACKGROUND_0
                       : ( std::uint8_t ) SGRCommand::CGA_FOREGROUND_0;
        starting += color & 0xff;
        // ensure starting is in bounds. If we somehow went out of bounds,
        // set to the default color

        SGRCommand command = ( SGRCommand ) starting;

        command = fixColor ( command,
                             offset ? SGRCommand::CGA_BACKGROUND_0
                                    : SGRCommand::CGA_FOREGROUND_0,
                             offset ? SGRCommand::BACKGROUND_DEFAULT
                                    : SGRCommand::FOREGROUND_DEFAULT );
        console << cga ( command );
    } else if ( ( color & 0xff ) == 9 )
    {
        // set the color per a bitmap in the second byte
        std::uint8_t secondByte = 0xff & ( color >> 8 );
        console << vga ( secondByte );
    } else
    {
        console << trueColor ( 0xff & ( color >> 24 ),
                               0xff & ( color >> 16 ),
                               0xff & ( color >> 8 ) );
    }
}

SGRCommand fixColor ( SGRCommand in, SGRCommand lo, SGRCommand hi )
{
    std::uint8_t x = ( std::uint8_t ) in;
    std::uint8_t y = ( std::uint8_t ) lo;
    std::uint8_t z = ( std::uint8_t ) hi;
    if ( x < y || x > z )
    {
        return hi;
    } else
    {
        return in;
    }
}

// input parsing functions.

/**
 * @brief Parses dummy input which requires the user to press enter (i.e., waits
 * on std::getline(string, cin))
 *
 * @param istream std::cin
 * @return std::any the boolean value true.
 */
std::any parseInputNone ( std::istream &istream )
{
    defines::ChrString temp = "";
    std::getline ( istream, temp );
    return std::make_any< bool > ( true );
}

std::any parseInputFullName ( std::istream &istream )
{
    defines::ChrString *name = new defines::ChrString [ 2 ];

    istream >> name [ 0 ] >> name [ 1 ];

    return std::make_any< defines::ChrString [ 2 ] > ( name );
}

InputGetter chooseInputFunction ( ux::console::InputMode const &mode )
{
    using ux::console::InputMode;
    switch ( mode )
    {
        case InputMode::FULL_NAME: return parseInputFullName;
        case InputMode::NONE: return parseInputNone;
        default: return parseInputNone;
    }
}

ux::console::IOThing::impl_s::impl_s ( ) noexcept
{
    // set the palette to console's own defaults.
    for ( std::size_t i = 0; i < 8; i++ )
    {
        palette.emplace ( i,
                          std::make_shared< RGBAColor > (
                                  defines::defaultConsoleColors [ i ][ 0 ],
                                  defines::defaultConsoleColors [ i ][ 1 ],
                                  defines::defaultConsoleColors [ i ][ 2 ],
                                  0 ) );
    }
    // ensure that the next options are minimally not empty.
    nextOptions.push_back ( nullptr );
}
