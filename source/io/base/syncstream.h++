/**
 * @file syncstream.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Synchronized, Thread Safe, FIFO, output streams
 * @version 1
 * @date 2022-01-25
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <streambuf>

#include <io/base/unistring.h++>

namespace io::base
{
    template < class CharT , class Traits , class Allocator >
    class basic_syncstreambuf;

    template < class CharT , class Traits , class Allocator >
    class basic_osyncstream;

    template < class CharT , class Traits , class Allocator >
    class SynchronizedStreamBufferImplementation
    {
        friend class basic_syncstreambuf < CharT , Traits , Allocator >;

        std::map < std::basic_streambuf < CharT , Traits > * , std::unique_ptr < std::mutex > > locks;
        SynchronizedStreamBufferImplementation ( ) = default;

        /**
         * @brief Registers a streambuf
         *
         * @param buf the streambuf to register.
         */
        void doRegister ( std::basic_streambuf < CharT , Traits > *const &buf )
        {
            if ( !locks.contains ( buf ) )
            {
                locks.emplace ( buf , new std::mutex ( ) );
            }
        }

        /**
         * @brief We do something to our buffer atomically across all threads and all basic_syncstreambuf's directed
         * at that buffer.
         * @note If the action fails, it is considered to have completed.
         * @param buf the buffer
         * @param action the action
         */
        void doAtomically ( std::basic_streambuf < CharT , Traits > *const &buf , std::function < void ( ) > const &action )
        {
            std::scoped_lock < std::mutex > lock ( *locks [ buf ] );
            action ( );
        }
    };

    template < class CharT , class Traits , class Allocator >
    std::unique_ptr < SynchronizedStreamBufferImplementation < CharT , Traits , Allocator > > container = nullptr;

    template < class CharT , class Traits = std::char_traits < CharT > , class Allocator = std::allocator < CharT > >
    class basic_syncstreambuf : public std::basic_streambuf < CharT , Traits >
    {
        std::mutex bufferMutex;
        std::mutex streamMutex;
        std::basic_streambuf < CharT , Traits > *stream;
        std::basic_string < CharT , Traits , Allocator > buffer;
        std::atomic_bool emitOnSync = false;

        void move ( basic_syncstreambuf &&that )
        {
            that.emit ( );


            stream = std::move ( that.stream );
            buffer = std::move ( that.buffer );
            emitOnSync = std::move ( that.emitOnSync );

            that.stream = nullptr;
            that.emitOnSync.store ( false );
        }

        void destruct ( )
        {
            emit ( );
            stream = nullptr;
            buffer = std::basic_string < CharT , Traits , Allocator > ( );
            emitOnSync.store ( false );
        }

    public:
        basic_syncstreambuf ( ) : basic_syncstreambuf ( nullptr ) {
            if ( !container < CharT , Traits , Allocator > )
            {
                container < CharT , Traits , Allocator > = std::make_unique < SynchronizedStreamBufferImplementation < CharT , Traits , Allocator > > ( SynchronizedStreamBufferImplementation < CharT , Traits , Allocator > ( ) );
            }
        }
        explicit basic_syncstreambuf ( std::basic_streambuf < CharT , Traits > *obuf ) : stream ( obuf )
        {
            if ( !container < CharT , Traits , Allocator > )
            {
                container < CharT , Traits , Allocator > = std::make_unique < SynchronizedStreamBufferImplementation < CharT , Traits , Allocator > > ( SynchronizedStreamBufferImplementation < CharT , Traits , Allocator > ( ) );
            }
            container < CharT , Traits , Allocator >->doRegister ( obuf );
        }
        basic_syncstreambuf ( std::basic_streambuf < CharT , Traits > *obuf , Allocator const &a ) : basic_syncstreambuf ( obuf ) {}
        basic_syncstreambuf ( basic_syncstreambuf &&that )
        {
            move ( that );
        }

        basic_syncstreambuf &operator = ( basic_syncstreambuf &&that )
        {
            destruct ( );
            move ( that );
            return *this;
        }

        void swap ( basic_syncstreambuf &other )
        {
            emit ( );
            other.emit ( );
            std::scoped_lock < std::mutex > thisBufferLock ( bufferMutex );
            std::scoped_lock < std::mutex > thisStreamLock ( streamMutex );

            std::scoped_lock < std::mutex > thatBufferLock ( other.bufferMutex );
            std::scoped_lock < std::mutex > thatStreamLock ( other.streamMutex );
            std::swap ( buffer , other.buffer );
            std::swap ( stream , other.stream );
            bool temp = emitOnSync.load ( );
            emitOnSync.store ( other.emitOnSync.load ( ) );
            other.emitOnSync.store ( temp );
        }

        ~basic_syncstreambuf ( )
        {
            // as required by standard.
            // we must catch and ignore any exception thrown
            // by emit
            try { emit ( ); } catch ( ... ) { }
        }

        bool emit ( )
        {
            bool result;
            auto emission = [ & ] ( )
            {
                std::scoped_lock < std::mutex > streamLock ( streamMutex );
                std::scoped_lock < std::mutex > bufferLock ( bufferMutex );
                if ( stream )
                {
                    stream->sputn ( buffer.c_str ( ) , buffer.size ( ) );
                    buffer = emptyString < CharT , Traits , Allocator > ( );
                    result = true;
                    if ( !stream->pubsync ( ) ) {
                        result &= true;
                    } else result = false;
                    return;
                }
                result = false;
            };
            container < CharT , Traits , Allocator >->doAtomically ( stream , [ & ] ( ) {emission ( ); } );
            return result;
        }

        std::basic_streambuf < CharT , Traits > *get_wrapped ( ) const noexcept
        {
            return stream;
        }

        Allocator get_allocator ( ) const noexcept
        {
            return Allocator ( );
        }

        void set_emit_on_sync ( bool b ) noexcept
        {
            emitOnSync.store ( b );
        }
    protected:
        int sync ( ) override
        {
            if ( emitOnSync.load ( ) )
            {
                return emit ( ) ? 0 : -1;
            }
            return 0;
        }

        void imbue ( std::locale const &loc ) override
        {
            auto imbuement = [ & ] ( )
            {
                std::scoped_lock < std::mutex > streamLock ( streamMutex );
                stream->pubimbue ( loc );
            };

            container< CharT , Traits , Allocator >->doAtomically ( stream , [ & ] ( ) { imbuement ( ); } );
        }

        std::basic_streambuf < CharT , Traits > *setbuf ( CharT *s , std::streamsize n ) override
        {
            std::basic_streambuf < CharT , Traits > *result;
            auto bufferset = [ & ] ( )
            {
                std::scoped_lock < std::mutex > streamLock ( streamMutex );
                result = stream->pubsetbuf ( s , n );
            };

            container< CharT , Traits , Allocator >->doAtomically ( stream , [ & ] ( ) { bufferset ( ); } );
            return result;
        }

        Traits::pos_type seekoff ( Traits::off_type off , std::ios_base::seekdir dir , std::ios_base::openmode which = std::ios_base::in | std::ios_base::out ) override
        {
            typename Traits::pos_type result;
            auto offseek = [ & ] ( )
            {
                std::scoped_lock < std::mutex > streamLock ( streamMutex );
                result = stream->pubseekoff ( off , dir , which );
            };
            container < CharT , Traits , Allocator>->doAtomically ( stream , [ & ] ( ) { offseek ( ); } );
            return result;
        }

        Traits::pos_type seekpos ( Traits::pos_type pos , std::ios_base::openmode which = std::ios_base::in | std::ios_base::out ) override
        {
            typename Traits::pos_type result;
            auto posseek = [ & ] ( )
            {
                std::scoped_lock < std::mutex > streamLock ( streamMutex );
                result = stream->pubseekpos ( pos , which );
            };
            container < CharT , Traits , Allocator>->doAtomically ( stream , [ & ] ( ) { posseek ( ); } );
            return result;
        }

        // returns 0 since this stream can only output data.
        std::streamsize showmanyc ( ) override
        {
            return -1;
        }

        Traits::int_type underflow ( ) override
        {
            return Traits::eof ( );
        }

        Traits::int_type uflow ( ) override
        {
            return underflow ( );
        }

        std::streamsize xsgetn ( Traits::char_type *s , std::streamsize count ) override
        {
            for ( std::streamsize i = 0; i < count; i++ ) s [ i ] = uflow ( );
            return count;
        }

        std::streamsize xsputn ( Traits::char_type const *s , std::streamsize count ) override
        {
            std::basic_string < CharT , Traits , Allocator > str = emptyString < CharT , Traits , Allocator > ( );
            for ( std::streamsize i = 0; i < count; i++ )
            {
                str += s [ i ];
            }
            std::scoped_lock < std::mutex > bufferLock ( bufferMutex );
            buffer += str;
            return count;
        }
    };

    template < class CharT , class Traits = std::char_traits < CharT > , class Allocator = std::allocator < CharT > >
    class basic_osyncstream : public std::basic_ostream < CharT , Traits >
    {
        // locks when accessing buffer to prevent us from
        // modifying during a write.
        std::mutex modifyBuffer;
        basic_syncstreambuf < CharT , Traits , Allocator > *buffer;

        void move ( basic_osyncstream &&that )
        {
            std::scoped_lock < std::mutex > modifyLock ( modifyBuffer );
            buffer = std::move ( that.buffer );
            that.buffer = nullptr;
        }
    public:
        basic_osyncstream ( std::basic_streambuf < CharT , Traits > *buf , Allocator const &a ) : basic_osyncstream ( buf ) {}
        explicit basic_osyncstream ( std::basic_streambuf < CharT , Traits > *buf ) : buffer ( new basic_syncstreambuf ( buf ) ) {}
        basic_osyncstream ( std::basic_ostream < CharT , Traits > &os , Allocator const &a ) : basic_osyncstream ( os.rdbuf ( ) , a ) {}
        basic_osyncstream ( basic_osyncstream &&that ) noexcept
        {
            move ( that );
        }

        basic_osyncstream &operator = ( basic_osyncstream &&that ) noexcept
        {
            move ( that );
            return *this;
        }

        ~basic_osyncstream ( )
        {
            emit ( );
        }

        basic_syncstreambuf < CharT , Traits > *rdbuf ( ) const noexcept
        {
            return buffer;
        }

        std::basic_streambuf < CharT , Traits > *get_wrapped ( ) const noexcept
        {
            std::scoped_lock < std::mutex > modifyLock ( modifyBuffer );
            if ( buffer )
            {
                return buffer->get_wrapped ( );
            } else return nullptr;
        }

        void emit ( )
        {
            std::scoped_lock < std::mutex > modifyLock ( modifyBuffer );
            if ( buffer ) buffer->emit ( );
        }
    };

    using syncbuf = basic_syncstreambuf < char >;
    using wsyncbuf = basic_syncstreambuf < wchar_t >;
    using osyncstream = basic_osyncstream < char >;
    using wosyncstream = basic_osyncstream < wchar_t >;
}