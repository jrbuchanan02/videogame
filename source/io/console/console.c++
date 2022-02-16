/**
 * @file console.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implementation of the console class
 * @version 1
 * @date 2022-02-13
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <io/console/console.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <io/base/syncstream.h++>
#include <io/console/colors/color.h++>
#include <io/console/colors/direct.h++>
#include <io/console/colors/indirect.h++>
#include <io/console/internal/channel.h++>
#include <io/console/manip/stringfunctions.h++>
#include <io/unicode/character.h++>

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <thread>

#ifdef WINDOWS
#    include "windows.h"
#endif // ifdef windows

struct io::console::Console::impl_s
{
    // our implementation of splitting by code point opened th door
    // to a race condition where text may be interleaved if sent to
    // the text channel.
    std::mutex sending;
    std::atomic_bool mutable readySignal = false;
    internal::TextChannel txt {
            internal::TextChannel::SharedFlag ( &readySignal ) };
    internal::TextChannel cmd {
            internal::TextChannel::SharedFlag ( &readySignal ) };

    // data for managing the two channels.
    std::chrono::milliseconds maxDelay ( ) const noexcept;
    void                      ensureStopped ( ) const noexcept;
    // data for managing the text channel
    struct ConsolePoint
    {
        std::uint32_t row;
        std::uint32_t col;
    };

    using ConsolePosition = ConsolePoint;
    using ConsoleSize     = ConsolePoint;
    using CursorPosition  = ConsolePoint;

    std::stack< CursorPosition >      positionStack;
    void                              pushCursorPosition ( );
    void                              pullCursorPosition ( );
    // data for managing the command channel
    // mutex to prevent reading invalid colors
    // std::mutex                        changingColors;
    // colors onscreen.
    std::shared_ptr< colors::IColor > screen [ 8 ];
    // colors used for calculating those onscreen.
    // this value is a map to allow random access and fast addition / removal
    // without reallocating the entire system.
    std::map< std::size_t, std::shared_ptr< colors::IColor > > colors;
    // "now" so-to-speak.
    double                                                     time = 0;
    // thread which feeds the cmd channel with the commands.
    std::jthread                                               commands;
    // boolean to tell the jthread that it's time to shut down when
    // the console is destructed
    std::atomic_bool mutable stopSignal = false;
    // the function which performs the text-feeding. Internally uses the same
    // delay between ticks as the cmd channel
    void commandGenerator ( );

    // just some general data
    ConsoleSize consoleSize = { 25, 80 };

    // internal flag to block a thread until the text channel has caught up.
    bool waitOnTextChannel = false;

    impl_s ( ) noexcept
    {
        // txt.setReady ( std::shared_ptr< std::atomic_bool > ( &readySignal )
        // ); std::cout << "here\n"; cmd.setReady ( std::shared_ptr<
        // std::atomic_bool > ( &readySignal ) ); std::cout << "here\n";
        for ( std::size_t i = 0; i < 8; i++ )
        {
            screen [ i ] = std::shared_ptr< colors::RGBAColor > (
                    new colors::RGBAColor (
                            defines::defaultConsoleColors [ i ][ 0 ],
                            defines::defaultConsoleColors [ i ][ 1 ],
                            defines::defaultConsoleColors [ i ][ 2 ],
                            0xFF ) );
        }

        for ( std::size_t i = 0; i < 8; i++ )
        {
            for ( std::size_t j = 0; j < 3; j++ )
            {
                screen [ i ]->setBasicComponent (
                        j,
                        defines::defaultConsoleColors [ i ][ j ] );
            }
        }

        commands = std::jthread ( [ & ] ( ) { commandGenerator ( ); } );
        commands.detach ( );
        // TODO: insert console initialization routine.
        // TODO: reset console
        readySignal.store ( true );
    }

    ~impl_s ( )
    {
        readySignal.store ( false );
        stopSignal.store ( true );
        ensureStopped ( );
        while ( commands.joinable ( ) ) { commands.join ( ); }
    }
};

std::chrono::milliseconds
        io::console::Console::impl_s::maxDelay ( ) const noexcept
{
    if ( txt.getDelay ( ) > cmd.getDelay ( ) )
    {
        return std::chrono::milliseconds ( txt.getDelay ( ) );
    } else
        return std::chrono::milliseconds ( cmd.getDelay ( ) );
}

void io::console::Console::impl_s::ensureStopped ( ) const noexcept
{
    readySignal.store ( true );
    std::this_thread::sleep_for ( maxDelay ( ) );
}

void io::console::Console::impl_s::pushCursorPosition ( )
{
    ensureStopped ( );
    std::cout << "\u001b[6n";
    std::cout.flush ( );
    // maximum size of the command response is actually
    // 2+10+1+10+1 bytes
    // or 24 bytes. However, we want to account for a maximum row and
    // column size of 140 4 - 2 / (RPN) or 136 / 2 or 68 digit number.
    char buffer [ 140 ];
    std::cin.get ( buffer, 140, 'R' );
    char              esc, openBracket, semicolon;
    std::stringstream temp ( buffer );
    CursorPosition    current;
    temp >> esc >> openBracket >> current.row >> semicolon >> current.col;
    positionStack.push ( current );
    readySignal.store ( true );
}

void io::console::Console::impl_s::pullCursorPosition ( )
{
    ensureStopped ( );
    std::cout << "\u001b[" << positionStack.top ( ).row << ";"
              << positionStack.top ( ).col << "H";
    std::cout.flush ( );
    positionStack.pop ( );
    readySignal.store ( true );
}

void io::console::Console::impl_s::commandGenerator ( )
{
    using namespace std::chrono_literals;
    while ( !this->stopSignal.load ( ) )
    {
        this->time += 0.1;

        std::stringstream command;
        auto generateCommand = [ & ] ( std::size_t color ) -> std::string {
            std::stringstream result;
            this->screen [ color ]->refresh ( time );
            defines::UnboundColor const *rawColor =
                    this->screen [ color ]->rgba ( time );
            defines::BoundColor bound [ 4 ] = {
                    colors::bind ( rawColor [ 0 ] ),
                    colors::bind ( rawColor [ 1 ] ),
                    colors::bind ( rawColor [ 2 ] ),
                    colors::bind ( rawColor [ 3 ] ),
            };

            defines::SentColor sent [ 4 ] = {
                    defines::SentColor ( bound [ 0 ] ),
                    defines::SentColor ( bound [ 1 ] ),
                    defines::SentColor ( bound [ 2 ] ),
                    defines::SentColor ( bound [ 3 ] ),
            };

            result << "\u001b]" << defines::paletteChangePrefix << std::hex;
            result << color << defines::paletteChangeSpecif;
            for ( std::size_t i = 0; i < 2; i++ )
            {
                result << sent [ i ] << defines::paletteChangeDelimt;
            }
            result << sent [ 2 ] << "\u001b\\";
            delete [] rawColor;
            return result.str ( );
        };

        for ( std::size_t i = 0; i < 8; i++ )
        {
            command << generateCommand ( i );
        }
        this->cmd.pushString ( command.str ( ) );

        auto now   = [ & ] ( ) { return std::chrono::steady_clock::now ( ); };
        auto start = now ( );
        while ( now ( ) - start < operator""ms ( this->cmd.getDelay ( ) ) )
        {
            if ( this->stopSignal.load ( ) )
            {
                return;
            } else
            {
                std::this_thread::sleep_for ( 1ms );
            }
        }
        // wait again, but this time for the ready signal, if that is low
        // choosing to wait here ensures that we have a buffer of around one
        // cycle, which gives us some wiggle room.
        while ( !this->readySignal.load ( ) )
        {
            auto now = [ & ] ( ) { return std::chrono::steady_clock::now ( ); };
            auto start = now ( );
            while ( now ( ) - start < operator""ms ( this->cmd.getDelay ( ) ) )
            {
                if ( this->stopSignal.load ( ) )
                {
                    return;
                } else
                {
                    std::this_thread::sleep_for ( 1ms );
                }
            }
        }
    }
}

io::console::Console::Console ( ) : pimpl ( new impl_s ( ) ) { }
io::console::Console::~Console ( ) = default;

std::uint32_t io::console::Console::getCols ( ) const noexcept
{
    return pimpl->consoleSize.col;
}

void io::console::Console::setCols ( std::uint32_t const &value ) noexcept
{
    pimpl->consoleSize.col = value;
}

std::uint32_t io::console::Console::getRows ( ) const noexcept
{
    return pimpl->consoleSize.row;
}

void io::console::Console::setRows ( std::uint32_t const &value ) noexcept
{
    pimpl->consoleSize.row = value;
}

void io::console::Console::send ( std::string const &str ) noexcept
{
    std::shared_ptr< bool > lastToken;
    {
        for ( auto &cp : manip::splitByCodePoint ( str ) )
        {
            // check for emoji. Their graphical representation is two columns
            // wide on windows-systems, the cursor only moves one column across.
            std::string temp = cp;
            char32_t    c    = manip::widen ( temp.c_str ( ) );
            if ( unicode::characterProperties ( ).at ( c ).emoji )
            {
                temp += "\u001b[C";
            }

            lastToken = pimpl->txt.pushString ( temp );
        }
    }
    if ( pimpl->waitOnTextChannel )
    {
        while ( !*lastToken )
        {
            std::this_thread::sleep_for ( std::chrono::milliseconds ( 1 ) );
        }
    }
}

std::shared_ptr< io::console::colors::IColor >
        io::console::Console::getScreenColor ( std::uint8_t const &index )
{
    if ( index > 7 )
    {
        RUNTIME_ERROR ( "Index for screen color out of bounds: ",
                        index,
                        " when max is 7!" )
    } else
    {
        return pimpl->screen [ index ];
    }
}

std::shared_ptr< io::console::colors::IColor >
        io::console::Console::getCalculationColor (
                std::size_t const    &at,
                colors::IColor const &deflt )
{
    if ( pimpl->colors.contains ( at ) )
    {
        return pimpl->colors.at ( at );
    } else
    {
        pimpl->colors.insert ( std::pair { at, ( colors::IColor * ) &deflt } );
        return pimpl->colors.at ( at );
    }
}

void io::console::Console::setScreenColor (
        std::uint8_t const                      &index,
        std::shared_ptr< colors::IColor > const &color )
{
    pimpl->screen [ index & 7 ] = color;
}

void io::console::Console::setCalculationColor (
        std::size_t const                       &index,
        std::shared_ptr< colors::IColor > const &color )
{
    pimpl->colors.insert_or_assign ( index, color );
}

std::uint64_t io::console::Console::getTxtRate ( ) const noexcept
{
    return pimpl->txt.getDelay ( );
}
void io::console::Console::setTxtRate ( std::uint64_t const &value ) noexcept
{
    pimpl->txt.setDelay ( value );
}
std::uint64_t io::console::Console::getCmdRate ( ) const noexcept
{
    return pimpl->cmd.getDelay ( );
}
void io::console::Console::setCmdRate ( std::uint64_t const &value ) noexcept
{
    pimpl->cmd.setDelay ( value );
}

void io::console::Console::setWaitOnText ( bool const &value ) noexcept
{
    pimpl->waitOnTextChannel = value;
}