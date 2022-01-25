/**
 * @file rememberer.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implementation for rememberrer
 * @version 1
 * @date 2022-01-22
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#include "rememberer.h++"

#include <atomic>
#include <chrono>
#include <list>
#include <memory>
#include <thread>

#ifdef IN_SOURCE_UTIL_MEMORY_REMEMBERER_HPP

template < class T >
struct util::memory::Rememberer < T >::impl_s {
    std::list < std::shared_ptr < T > > items;
    std::atomic < std::chrono::milliseconds > delay;
    std::atomic_bool started , stopped , pruning , valid;

    bool itemUnused ( std::shared_ptr < T > sp ) {
        return sp.use_count ( ) <= 1;
    }

    void prune ( ) {
        items.remove_if ( [ & ] ( auto p ) { return this->itemUnused ( p ); } );
    }

    void actions ( ) {
        started.store ( true );
        while ( valid.load ( ) ) {
            pruning.store ( true );
            prune ( );
            pruning.store ( false );
            std::this_thread::sleep_for ( delay.load ( ) );
        }
        stopped.store ( true );
    }

    impl_s ( ) {
        delay.store ( std::chrono::milliseconds ( 1000 ) );
        std::thread ( [ & ] ( ) {this->action ( ); } ).detach ( );
    }
};

template < class T >
util::memory::Rememberer < T >::Rememberer ( ) : pimpl ( new impl_s ( ) ) {}

template < class T >
util::memory::Rememberer < T >::~Rememberer ( ) {
    pimpl->valid.store ( false );
    pimpl->delay.store ( std::chrono::milliseconds ( 0 ) );
    while ( !pimpl->stopped.load ( ) ) std::this_thread::sleep_for ( std::chrono::microseconds ( 1 ) );
}


template < class T > void util::memory::Rememberer < T >::setPeriod ( std::uint64_t const msecs ) {
    pimpl->delay.store ( std::chrono::milliseconds ( msecs ) );
}
template < class T > std::uint64_t const util::memory::Rememberer < T >::getPeriod ( void ) const {
    return pimpl->delay.load ( ).count ( );
}
template < class T > std::shared_ptr < T > util::memory::Rememberer < T >::get ( T &t ) {
    for ( auto buffer : pimpl->items ) {
        if ( buffer.get() == &t ) {
            return buffer;
        }
    }
    std::shared_ptr < T > pointer = std::make_shared < T > ( &t );
    pimpl->items.push_back ( pointer );
    return pointer;
}

#endif