/**
 * @file channel.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Text channel implementation
 * @version 1
 * @date 2022-02-13
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <io/console/internal/channel.h++>

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <io/base/syncstream.h++>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>

using namespace std::chrono_literals;

struct io::console::internal::TextChannel::impl_s
{
    io::base::osyncstream                    stream { std::cout.rdbuf ( ) };
    std::queue< std::string >                queue { };
    std::atomic< std::chrono::milliseconds > delay = 100ms;
    std::jthread                             thread;
    std::atomic_bool                         stop;
    SharedFlag                               ready;

    // internal functions
    void spin ( SharedFlag const &watch, bool const &checkStop = true );
    void wait ( bool const &checkStop = true );

    void send ( );
    void loop ( );

    impl_s ( SharedFlag const & );
    virtual ~impl_s ( );
};

io::console::internal::TextChannel::TextChannel ( ) noexcept :
        TextChannel ( SharedFlag ( &defaultReady ) )
{ }
io::console::internal::TextChannel::TextChannel (
        SharedFlag const &ready ) noexcept :
        pimpl ( new impl_s ( ready ) )
{ }

io::console::internal::TextChannel::~TextChannel ( ) { pimpl.release ( ); }

void io::console::internal::TextChannel::setDelay (
        std::uint64_t const &delay ) noexcept
{
    this->pimpl->delay.store ( std::chrono::milliseconds ( delay ) );
}

std::uint64_t const
        io::console::internal::TextChannel::getDelay ( ) const noexcept
{
    return this->pimpl->delay.load ( ).count ( );
}

void io::console::internal::TextChannel::pushString (
        std::string const &string ) noexcept
{
    this->pimpl->queue.push ( string );
}

void io::console::internal::TextChannel::setReady (
        SharedFlag const &ready ) noexcept
{
    this->pimpl->ready = ready;
}

void io::console::internal::TextChannel::impl_s::spin ( SharedFlag const &watch,
                                                        bool const &checkStop )
{
    while ( !watch->load ( ) ) { wait ( checkStop ); }
}

void io::console::internal::TextChannel::impl_s::wait ( bool const &checkStop )
{
    auto now   = [ & ] ( ) { return std::chrono::steady_clock::now ( ); };
    auto start = now ( );
    while ( now ( ) - start > delay.load ( ) )
    {
        if ( checkStop && stop.load ( ) )
        {
            return;
        } else
        {
            std::this_thread::sleep_for ( 1ms );
        }
    }
}

void io::console::internal::TextChannel::impl_s::send ( )
{
    spin ( ready );
    if ( stop.load ( ) || queue.empty ( ) )
    {
        return;
    }
    stream << queue.front ( );
    stream.emit ( );
    queue.pop ( );
}

void io::console::internal::TextChannel::impl_s::loop ( )
{
    spin ( ready );
    while ( !stop.load ( ) )
    {
        send ( );
        wait ( );
    }
}

io::console::internal::TextChannel::impl_s::impl_s ( SharedFlag const &ready ) :
        thread ( [ & ] ( ) { loop ( ); } ), stop ( false ), ready ( ready )
{
    thread.detach ( );
}

io::console::internal::TextChannel::impl_s::~impl_s ( )
{
    stop.store ( true );
    delay.store ( 0ms );
    while ( thread.joinable ( ) ) thread.join ( );
}