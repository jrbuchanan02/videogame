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

#include <atomic>
#include <chrono>
#include <queue>
#include <stack>
#include <string>
#include <thread>

#ifdef WINDOWS
#    include "windows.h"
#endif // ifdef windows

struct io::console::Console::impl_s
{
    internal::TextChannel txt;
    internal::TextChannel cmd;

    // data for managing the two channels.
    std::atomic_bool mutable readySignal = false;
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

    std::stack< CursorPosition > positionStack;
    void                         pushCursorPosition ( );
    void                         pullCursorPosition ( );
    // data for managing the command channel

    // colors onscreen.
    std::shared_ptr< colors::IColor >              screen [ 8 ];
    // colors used for calculating those onscreen.
    std::list< std::shared_ptr< colors::IColor > > colors;
    // "now" so-to-speak.
    double                                         time;
    // thread which feeds the cmd channel with the commands.
    std::jthread                                   commands;
    // boolean to tell the jthread that it's time to shut down when
    // the console is destructed
    std::atomic_bool                               stopSignal = false;
    // the function which performs the text-feeding. Internally uses the same
    // delay between ticks as the cmd channel
    void                                           commandGenerator ( );
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
            std::stringstream            result;
            defines::UnboundColor const *rawColor =
                    this->screen [ color ]->rgba ( time );
            defines::BoundColor bound [ 4 ] = {
                    colors::bind ( rawColor [ 0 ] ),
                    colors::bind ( rawColor [ 1 ] ),
                    colors::bind ( rawColor [ 2 ] ),
                    colors::bind ( rawColor [ 3 ] ),
            };
            delete [] rawColor;
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
            return result.str ( );
        };

        for ( std::size_t i = 0; i < 8; i++ )
        {
            command << generateCommand ( i );
        }
        this->cmd.pushString ( command.str ( ) );

        auto now   = [ & ] ( ) { return std::chrono::steady_clock::now ( ); };
        auto start = now ( );
        while ( now ( ) - start > operator""ms ( this->cmd.getDelay ( ) ) )
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
        // choosing to wait here ensures that we have a buffer of around one cycle, which gives us some wiggle room.
        while ( !this->readySignal.load ( ) )
        {
            auto now = [ & ] ( ) { return std::chrono::steady_clock::now ( ); };
            auto start = now ( );
            while ( now ( ) - start > operator""ms ( this->cmd.getDelay ( ) ) )
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