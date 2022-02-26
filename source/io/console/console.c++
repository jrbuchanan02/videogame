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
#include <vector>

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
    ConsoleSize                              consoleSize = { 25, 80 };
    // thread which holds a function to regularly update the size of the
    // console.
    std::jthread                             sizeUpdater;
    // time between updates.
    std::atomic< std::chrono::milliseconds > updateRate =
            std::chrono::milliseconds ( 500 );

    void sizeUpdateFunction ( );

    // internal flag to block a thread until the text channel has caught up.
    bool                waitOnTextChannel = false;
    // internal flag to (attempt to) wrap text.
    bool                wrapText          = false;
    // internal flag to (attempt to) center text.
    bool                centerText        = false;
    // internal flags corresponding to the SGR attributes
    std::vector< bool > sgrMap;
    // internal colors corresponding to the current color
    // for the foreground / background. Handled separately
    // from the CGA colors, but since the console asserts
    // these foreground / background colors after setting
    // the CGA colors, these override the CGA values when
    // they indicate a 256 or true color
    std::uint32_t       foreground = 0;
    std::uint32_t       background = 0;

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
        for ( std::size_t i = 0; i < ( std::size_t ) SGRCommand::_MAX; i++ )
        {
            sgrMap.push_back ( false );
        }
        sgrMap.shrink_to_fit ( );

        commands = std::jthread ( [ & ] ( ) { commandGenerator ( ); } );
        commands.detach ( );
#ifdef WINDOWS
        SetConsoleOutputCP ( 65001 );
        HANDLE hcout = GetStdHandle ( STD_OUTPUT_HANDLE );
        DWORD  mode  = 0;
        GetConsoleMode ( hcout, &mode );
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode ( hcout, mode );
        HANDLE hcin = GetStdHandle ( STD_INPUT_HANDLE );
        GetConsoleMode ( hcin, &mode );
        mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        SetConsoleMode ( hcout, mode );
#endif
        std::cout << "\u001b[3J\u001b[2J\u001b[0m\u001b[H";
        std::cout.flush ( );
        readySignal.store ( true );
        sizeUpdater = std::jthread ( [ & ] ( ) { sizeUpdateFunction ( ); } );
        sizeUpdater.detach ( );
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

void io::console::Console::impl_s::sizeUpdateFunction ( )
{
    using namespace std::chrono_literals;
    while ( !this->stopSignal.load ( ) )
    {
#ifdef WINDOWS
        HANDLE                     hcout = GetStdHandle ( STD_OUTPUT_HANDLE );
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo ( hcout, &info );
        consoleSize.row = info.dwSize.Y;
        consoleSize.col = info.dwSize.X;
#else
        // use the linux environment variables
        defines::ChrChar *_rows = std::getenv ( "ROWS" );
        defines::ChrChar *_cols = std::getenv ( "COLUMNS" );

        defines::ChrStringStream rows { _rows };
        defines::ChrStringStream cols { _cols };
        rows >> consoleSize.row;
        cols >> consoleSize.col;
#endif

        auto now   = [ & ] ( ) { return std::chrono::steady_clock::now ( ); };
        auto start = now ( );
        while ( now ( ) - start < this->updateRate.load ( ) )
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
    std::string             line = str;
    if ( pimpl->wrapText )
    {
        std::vector< std::string > joinables =
                manip::generateTextInseperables ( line );
        std::vector< std::string > lines           = { "" };
        std::string                joined          = "";
        std::uint32_t              currentPosition = 0;

        auto lengthOf = [] ( std::string text ) -> std::uint32_t {
            std::uint32_t result = 0;
            for ( auto &cp : manip::splitByCodePoint ( text ) )
            {
                auto props = unicode::characterProperties ( ).at (
                        manip::widen ( cp.c_str ( ) ) );
                if ( !props.control )
                {
                    result += 1 + props.columns;
                }
            }
            return result;
        };
        // for each joinable string in joinables:
        // 1. if it's a hard line break, reset the current position since a
        // line break will occur automatically.
        // 2. if adding the next joinable would cause a screen wrap and we are
        // not at the beginning of a line, insert a newline then add the
        // joinable.
        // 3. if we uncontrollably wrapped the previous line and we have another
        // joinable to add, unconditionally break between the two.
        for ( auto &joinable : joinables )
        {
            std::uint32_t itsLength = lengthOf ( joinable );
            // if we would uncontrollably wrap the screen by adding the sequence
            if ( currentPosition && itsLength + currentPosition > getCols ( ) )
            {
                lines.back ( ).append ( "\n" );
                lines.push_back ( joinable );
                currentPosition = itsLength;
                // no need to check again for the uncontrolled screen wrap
                // since currentPosition != 0.
            } else
            {
                lines.back ( ) += joinable;
                currentPosition += itsLength;
                // check if joinable ends in a hard line break
                char32_t lastCodePoint =
                        manip::widen ( manip::splitByCodePoint ( joinable )
                                               .back ( )
                                               .c_str ( ) );
                auto properties =
                        unicode::characterProperties ( ).at ( lastCodePoint );
                unicode::BreakingProperties breaking =
                        ( unicode::BreakingProperties ) properties.lineBreaking;
                switch ( breaking )
                {
                    case unicode::BreakingProperties::BK:
                    case unicode::BreakingProperties::CR:
                    case unicode::BreakingProperties::LF:
                    case unicode::BreakingProperties::NL:
                        // we have a hard line break
                        currentPosition = 0;
                        break;
                    default: break;
                }
            }
        }
        for ( auto &aLine : lines )
        {
            if ( pimpl->centerText )
            {
                // each line has a newline already, so we don't need to check
                // for that.
                joined += manip::centerTextOn ( aLine, getCols ( ) );
            } else
            {
                joined += aLine;
            }
        }
        // final step: set line to joined
        line = joined;
    }
    // assert our SGR attributes for this line
    // send all attributes.
    std::string command = "\u001b[m";
    for ( std::size_t i = 0; i < pimpl->sgrMap.size ( ); i++ )
    {
        if ( pimpl->sgrMap.at ( i ) )
        {
            command += "\u001b[" + std::to_string ( i ) + "m";
        }
    }
    if ( ( pimpl->foreground & 0xff ) == 9 )
    {
        command += "\u001b[38;5;" + std::to_string ( pimpl->foreground >> 8 )
                 + "m";
    } else if ( ( pimpl->foreground & 0xff ) == 10 )
    {
        command += "\u001b[38;2;"
                 + std::to_string ( ( 0xff & pimpl->foreground ) >> 24 ) + ";"
                 + std::to_string ( ( 0xff & pimpl->foreground ) >> 16 ) + ";"
                 + std::to_string ( ( 0xff & pimpl->foreground ) >> 8 ) + "m";
    }

    if ( ( pimpl->background & 0xff ) == 9 )
    {
        command += "\u001b[48;5;" + std::to_string ( pimpl->background >> 8 )
                 + "m";
    } else if ( ( pimpl->background & 0xff ) == 10 )
    {
        command += "\u001b[48;2;"
                 + std::to_string ( ( 0xff & pimpl->background ) >> 24 ) + ";"
                 + std::to_string ( ( 0xff & pimpl->background ) >> 16 ) + ";"
                 + std::to_string ( ( 0xff & pimpl->background ) >> 8 ) + "m";
    }
    line = command + line;
    {
        // iterate through the SGR attributes and assert the appropriate ones.
        std::scoped_lock< std::mutex > lock ( pimpl->sending );
        for ( auto &cp : manip::splitByCodePoint ( line ) )
        {
            // check for emoji. Their graphical representation is two
            // columns wide on windows-systems, the cursor only moves one
            // column across.
            std::string temp = cp;
            char32_t    c    = manip::widen ( temp.c_str ( ) );
            if ( unicode::characterProperties ( ).at ( c ).emoji )
            {
                // command that moves the cursor one unit forwards.
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

void io::console::Console::setWrapping ( bool const &value ) noexcept
{
    pimpl->wrapText = value;
}

void io::console::Console::setCentering ( bool const &value ) noexcept
{
    pimpl->centerText = value;
}

void io::console::Console::sgrCommand ( SGRCommand const &command,
                                        bool const        value ) noexcept
{
    // if we're setting a color, clear all other color attributes
    if ( value
         && ( std::size_t ) command
                    >= ( std::size_t ) SGRCommand::CGA_FOREGROUND_0
         && ( std::size_t ) command
                    <= ( std::size_t ) SGRCommand::BACKGROUND_DEFAULT )
    {
        for ( std::size_t i = 0; i < 8; i++ )
        {
            pimpl->sgrMap.at ( std::size_t ( SGRCommand::CGA_BACKGROUND_0 )
                               + i ) = false;
            pimpl->sgrMap.at ( std::size_t ( SGRCommand::CGA_FOREGROUND_0 )
                               + i ) = false;
        }
        pimpl->sgrMap.at ( std::size_t ( SGRCommand::FOREGROUND_DEFAULT ) ) =
                false;
        pimpl->sgrMap.at ( std::size_t ( SGRCommand::BACKGROUND_DEFAULT ) ) =
                false;
    }

    pimpl->sgrMap.at ( std::size_t ( command ) ) = value;
}

void io::console::Console::setForeground ( std::uint32_t const &color ) noexcept
{
    if ( ( color & 0xff ) < 9 )
    {
        // set sgr color accordingly
        for ( std::size_t i = 0; i < 8; i++ )
        {
            pimpl->sgrMap.at ( std::size_t ( SGRCommand::CGA_FOREGROUND_0 )
                               + i ) = false;
        }
        pimpl->sgrMap.at ( std::size_t ( SGRCommand::FOREGROUND_DEFAULT ) ) =
                false;
        switch ( color & 0x7 )
        {
            case 7:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_FOREGROUND_7 ) ) = true;
                break;
            case 6:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_FOREGROUND_6 ) ) = true;
                break;
            case 5:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_FOREGROUND_5 ) ) = true;
                break;
            case 4:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_FOREGROUND_4 ) ) = true;
                break;
            case 3:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_FOREGROUND_3 ) ) = true;
                break;
            case 2:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_FOREGROUND_2 ) ) = true;
                break;
            case 1:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_FOREGROUND_1 ) ) = true;
                break;
            case 0:
                if ( color & 8 )
                {
                    pimpl->sgrMap.at ( std::size_t (
                            SGRCommand::FOREGROUND_DEFAULT ) ) = true;
                } else
                {
                    pimpl->sgrMap.at ( std::size_t (
                            SGRCommand::CGA_FOREGROUND_0 ) ) = true;
                }
                break;
            default:
                if ( color & 8 )
                {
                    pimpl->sgrMap.at ( std::size_t (
                            SGRCommand::FOREGROUND_DEFAULT ) ) = true;
                } else
                {
                    pimpl->sgrMap.at ( std::size_t (
                            SGRCommand::CGA_FOREGROUND_0 ) ) = true;
                }
        }
    } else
    {
        // set internal color accordingly.
        pimpl->foreground = color;
    }
}
void io::console::Console::setBackground ( std::uint32_t const &color ) noexcept
{
    if ( ( color & 0xff ) < 9 )
    {
        // set sgr color accordingly
        for ( std::size_t i = 0; i < 8; i++ )
        {
            pimpl->sgrMap.at ( std::size_t ( SGRCommand::CGA_BACKGROUND_0 )
                               + i ) = false;
        }
        pimpl->sgrMap.at ( std::size_t ( SGRCommand::BACKGROUND_DEFAULT ) ) =
                false;
        switch ( color & 0x7 )
        {
            case 7:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_BACKGROUND_7 ) ) = true;
                break;
            case 6:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_BACKGROUND_6 ) ) = true;
                break;
            case 5:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_BACKGROUND_5 ) ) = true;
                break;
            case 4:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_BACKGROUND_4 ) ) = true;
                break;
            case 3:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_BACKGROUND_3 ) ) = true;
                break;
            case 2:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_BACKGROUND_2 ) ) = true;
                break;
            case 1:
                pimpl->sgrMap.at (
                        std::size_t ( SGRCommand::CGA_BACKGROUND_1 ) ) = true;
                break;
            case 0:
                if ( color & 8 )
                {
                    pimpl->sgrMap.at ( std::size_t (
                            SGRCommand::FOREGROUND_DEFAULT ) ) = true;
                } else
                {
                    pimpl->sgrMap.at ( std::size_t (
                            SGRCommand::CGA_FOREGROUND_0 ) ) = true;
                }
                break;
            default:
                if ( color & 8 )
                {
                    pimpl->sgrMap.at ( std::size_t (
                            SGRCommand::BACKGROUND_DEFAULT ) ) = true;
                } else
                {
                    pimpl->sgrMap.at ( std::size_t (
                            SGRCommand::CGA_BACKGROUND_0 ) ) = true;
                }
        }
    } else
    {
        // set internal color accordingly.
        pimpl->background = color;
    }
}