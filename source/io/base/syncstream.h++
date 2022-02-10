/**
 * @file syncstream.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Synchronized, Thread Safe, FIFO, output streams
 * @version 1
 * @date 2022-01-25
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <atomic>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <functional>
#include <io/base/unistring.h++>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <streambuf>
#include <test/unittester.h++>
#include <thread>
namespace io::base
{
    /**
     * @brief An implementation for basic_syncstreambuf
     * @note while this class is internal, it is documented so that
     * people reading the code can better understand what it does.
     * @tparam CharT the character type
     * @tparam Traits the traits of the character type
     * @tparam Allocator the allocator
     */
    template <class CharT, class Traits, class Allocator>
    class SynchronizedStreamBufferImplementation
    {
        friend class basic_syncstreambuf<CharT, Traits, Allocator>;

        static inline bool unittest ( std::ostream &stream )
        {
            stream << "Beginning unittest for "
                      "SynchronizedStreamBufferImplementation where "
                      "sizeof(CharT) = "
                   << sizeof ( CharT ) << ".\n";
            stream << "Ensuring that registration works...\n";
            {
                SynchronizedStreamBufferImplementation<CharT, Traits, Allocator>
                        test;
                test.doRegister ( nullptr );
                try
                {
                    std::scoped_lock<std::mutex> testLock (
                            *test.locks.at ( nullptr ) );
                } catch ( std::range_error &error )
                {
                    stream << "Threw range error, which indicates that the "
                              "registration process did not register.\n";
                    return false;
                }
            }
            {
                stream << "Testing race conditions...\n";
                SynchronizedStreamBufferImplementation<CharT, Traits, Allocator>
                        test;
                test.locks.emplace ( nullptr, new std::mutex ( ) );
                std::size_t        counter      = 0;
                std::atomic_size_t latch        = 1 << 10;
                auto               raceFunction = [ & ] ( ) {
                    test.doAtomically ( nullptr, [ & ] ( ) { counter++; } );
                    latch.fetch_sub ( 1 );
                };

                for ( int i = 0; i < 1 << 10; i++ )
                {
                    std::thread ( raceFunction ).detach ( );
                }
                while ( latch.load ( ) )
                { }
                if ( counter != 1 << 10 )
                {
                    stream << "Race condition failed, and the function is not "
                              "atomic.\n";
                    return false;
                } else
                {
                    return true;
                }
            }
        }

        static inline test::Unittest test = { &unittest };

        std::map<std::basic_streambuf<CharT, Traits> *,
                 std::unique_ptr<std::mutex>>
                locks;
        SynchronizedStreamBufferImplementation ( ) = default;

        /**
         * @brief Registers a streambuf
         *
         * @param buf the streambuf to register.
         */
        void doRegister ( std::basic_streambuf<CharT, Traits> *const &buf )
        {
            if ( !locks.contains ( buf ) )
            {
                locks.emplace ( buf, new std::mutex ( ) );
            }
        }

        /**
         * @brief We do something to our buffer atomically across all threads
         * and all basic_syncstreambuf's directed at that buffer.
         * @note If the action fails, it is considered to have completed.
         * @param buf the buffer
         * @param action the action
         */
        void doAtomically ( std::basic_streambuf<CharT, Traits> *const &buf,
                            std::function<void ( )> const              &action )
        {
            std::scoped_lock<std::mutex> lock ( *locks [ buf ] );
            action ( );
        }
    };
    /**
     * @brief Synchronized stream buffer.
     * @note basic_syncstreambuf does not technically comply with the C++20
     * standard. For one, it does not use the allocator given to it. However,
     * off the top of my head, that is the only nonstandard aspect (other than
     * the name being basic_syncstreambuf instead of basic_syncbuf).
     * @note Some terms:
     *  - the "one-at-a-time" guarantee means that this class guarantees that
     * only one thread at a time will modify some shared data.
     *  - near-pure passthrough means that a function behaves as if it were a
     * member of a wrapped object with only a few hints that there is anything
     * other than a pure passthrough happening (such as blocking a thread for
     * longer than usual)
     * @tparam CharT the character type
     * @tparam Traits the traits type
     * @tparam Allocator the allocator type.
     */
    template <class CharT, class Traits, class Allocator>
    class basic_syncstreambuf : public std::basic_streambuf<CharT, Traits>
    {
        // todo: switch to using *this* container rather than the one in the
        // namespace. Since I'm committing this code directly to main, I'm not
        // willing to make that change just now.
        static inline SynchronizedStreamBufferImplementation<CharT,
                                                             Traits,
                                                             Allocator>
                                                    container;
        std::mutex                                  bufferMutex;
        std::mutex                                  streamMutex;
        std::basic_streambuf<CharT, Traits>        *stream;
        std::basic_string<CharT, Traits, Allocator> buffer;
        std::atomic_bool                            emitOnSync = false;

        static inline bool selfTest ( std::ostream &stream )
        {
            stream << "Beginning test for basic_syncstreambuf with "
                      "sizeof(CharT) = "
                   << sizeof ( CharT ) << "\n";
            stream << "Ensuring that giving two syncbuf's the same output "
                      "stream gives them the same underlying buffer...\n";
            std::basic_stringstream<CharT, Traits, Allocator> testStream;
            basic_syncstreambuf<CharT, Traits, Allocator>     test1, test2;
            test1 = basic_syncstreambuf<CharT, Traits, Allocator> (
                    testStream.rdbuf ( ) );
            test2 = basic_syncstreambuf<CharT, Traits, Allocator> (
                    testStream.rdbuf ( ) );
            if ( test1.stream != test2.stream )
            {
                stream << "The two syncbufs ended up with different streams!\n";
                return false;
            }
            stream << "Ensuring that outputting to the two syncbufs do not go "
                      "through until calls to emit...\n";
            std::basic_ostream<CharT, Traits> stream1 ( &test1 );
            std::basic_ostream<CharT, Traits> stream2 ( &test2 );

            static CharT text [] = {
                    'S',
                    'o',
                    'm',
                    'e',
                    ' ',
                    't',
                    'e',
                    'x',
                    't',
                    '!',
                    '\n',
                    '\0',
            };
            static CharT text2 [] = {
                    'S', 'o', 'm',  'e', ' ', 't',  'e',  'x',
                    't', '!', '\n', 'S', 'o', 'm',  'e',  ' ',
                    't', 'e', 'x',  't', '!', '\n', '\0',
            };
            stream1 << text;
            stream2 << text;
            if ( !testStream.str ( ).empty ( ) )
            {
                stream << "The stringstream received text (or already had "
                          "it)!\n";
            }
            stream << "Ensuring that emitting at around the same time will not "
                      "garble output...\n";
            std::atomic_size_t ready = 2;
            std::atomic_size_t done  = 0;
            std::atomic_bool   go    = false;

            auto sendInformationOn =
                    [ & ] ( basic_syncstreambuf<CharT, Traits, Allocator>
                                    *alloc ) {
                        ready.fetch_sub ( 1 );
                        while ( !go.load ( ) )
                        { }
                        alloc->emit ( );
                        done.fetch_add ( 1 );
                    };
            std::thread ( std::bind_front ( sendInformationOn, &test1 ) )
                    .detach ( );
            std::thread ( std::bind_front ( sendInformationOn, &test2 ) )
                    .detach ( );
            while ( ready.load ( ) )
            { }
            go.store ( true );
            while ( done.load ( ) < 2 )
            { }
            if ( testStream.str ( ).find ( text2 ) == std::string::npos )
            {
                stream << "Could not find the text within the stream. "
                          "Indicates a failure.\n";
                return false;
            }
            return true;
        }

        static inline test::Unittest unittest = { &selfTest };

        /**
         * @brief Generic move operation
         * @note Since this routine is meant to be called during the move
         * constructor, it does not emit *this*.
         * @param that the basic_syncstreambuf to move
         */
        void move ( basic_syncstreambuf &&that )
        {
            that.emit ( );

            stream = std::move ( that.stream );
            buffer = std::move ( that.buffer );
            emitOnSync.store ( that.emitOnSync.load ( ) );

            that.stream = nullptr;
            that.emitOnSync.store ( false );
        }
        /**
         * @brief Basic destruction routine
         *
         */
        void destruct ( )
        {
            emit ( );
            stream = nullptr;
            buffer = std::basic_string<CharT, Traits, Allocator> ( );
            emitOnSync.store ( false );
        }
    public:
        basic_syncstreambuf ( ) : basic_syncstreambuf ( nullptr ) { }
        explicit basic_syncstreambuf (
                std::basic_streambuf<CharT, Traits> *obuf )
                : stream ( obuf )
        {
            container.doRegister ( obuf );
        }
        basic_syncstreambuf ( std::basic_streambuf<CharT, Traits> *obuf,
                              Allocator const                     &a )
                : basic_syncstreambuf ( obuf )
        { }
        basic_syncstreambuf ( basic_syncstreambuf &&that )
        {
            move ( std::move ( that ) );
        }

        basic_syncstreambuf &operator= ( basic_syncstreambuf &&that )
        {
            destruct ( );
            move ( std::move ( that ) );
            return *this;
        }
        /**
         * @brief Swaps two basic_syncstreambufs
         * @note Per the standard, both this and other emit the contents of
         * their respective buffers before swapping.
         * @note this function prevents both this syncstreambuf and other from
         * accessing any information about themselves until they finish the
         * swapping routine.
         * @param other the basic_syncstreambuf to swap with.
         */
        void swap ( basic_syncstreambuf &other )
        {
            emit ( );
            other.emit ( );
            std::scoped_lock<std::mutex> thisBufferLock ( bufferMutex );
            std::scoped_lock<std::mutex> thisStreamLock ( streamMutex );

            std::scoped_lock<std::mutex> thatBufferLock ( other.bufferMutex );
            std::scoped_lock<std::mutex> thatStreamLock ( other.streamMutex );
            std::swap ( buffer, other.buffer );
            std::swap ( stream, other.stream );
            bool temp = emitOnSync.load ( );
            emitOnSync.store ( other.emitOnSync.load ( ) );
            other.emitOnSync.store ( temp );
        }

        ~basic_syncstreambuf ( )
        {
            // as required by standard.
            // we must catch and ignore any exception thrown
            // by emit
            try
            {
                emit ( );
            } catch ( ... )
            { }
        }

        /**
         * @brief Sends the contents of an internal buffer out to the globally
         * maintained, thread-shared buffer.
         * @note this command will effect all basic_syncstreambuf's attached to
         * the same basic_streambuf (as in, it will prevent them from modifying
         * the underlying buffer). This function is considered part of the
         * "one-at-a-time" guarantee.
         * @return true we successfully sent all characters.
         * @return false either there were no characters to send or we did not
         * successfully send all characters.
         */
        bool emit ( )
        {
            bool result;
            auto emission = [ & ] ( ) {
                std::scoped_lock<std::mutex> streamLock ( streamMutex );
                std::scoped_lock<std::mutex> bufferLock ( bufferMutex );
                if ( stream )
                {
                    stream->sputn ( buffer.c_str ( ), buffer.size ( ) );
                    buffer = emptyString<CharT, Traits, Allocator> ( );
                    result = true;
                    if ( !stream->pubsync ( ) )
                    {
                        result &= true;
                    } else
                        result = false;
                    return;
                }
                result = false;
            };
            container.doAtomically ( stream, [ & ] ( ) { emission ( ); } );
            return result;
        }

        /**
         * @brief Obtains a raw pointer to the wrapped basic_streambuf
         * @note This pointer does **not** have the thread safety guarantees
         * of this class.
         *
         * @return std::basic_streambuf<CharT, Traits>* the wrapped buffer
         */
        std::basic_streambuf<CharT, Traits> *get_wrapped ( ) const noexcept
        {
            return stream;
        }

        /**
         * @brief Get a default-constructed allocator of the same type as the
         * allocator we were told to use.
         * @note This function is nonstandard as we are actually supposed to use
         * the allocator given to us.
         * @return Allocator the allocator.
         */
        Allocator get_allocator ( ) const noexcept { return Allocator ( ); }

        /**
         * @brief Enables or disables the setting to emit when we receive the
         * sync command.
         * @note Since the flag is internally implemented as a std::atomic_bool,
         * this function is not part of the "one-at-a-time" guarantee, since
         * std::atomic_bool makes that guarantee.
         * @param b the new state. True says to emit when we receive a call to
         * sync.
         */
        void set_emit_on_sync ( bool b ) noexcept { emitOnSync.store ( b ); }
    protected:
        /**
         * @brief Synchronize with the internal buffer. In reality, this only
         * does anything if we are set to emit on sync, since we use the
         * presence of text in the buffer as an indication that a sync is
         * pending. (Another instance of nonstandard behavior)
         * @note This function is not considered part of the "one-at-a-time"
         * guarantee since it does nothing when we do not emit on sync, and
         * emit() handles that guarantee when we do emit on sync.
         * @return 0 the call succeedeed.
         * @return -1 the call failed.
         */
        int sync ( ) override
        {
            if ( emitOnSync.load ( ) )
            {
                return emit ( ) ? 0 : -1;
            }
            return 0;
        }

        /**
         * @brief Imbues with the given locale.
         * @note This command affects *all* basic_syncstreambuf's attached to
         * the same internal basic_streambuf. This command is considered part of
         * the "one-at-a-time" guarantee.
         * @param loc the new locale.
         */
        void imbue ( std::locale const &loc ) override
        {
            auto imbuement = [ & ] ( ) {
                std::scoped_lock<std::mutex> streamLock ( streamMutex );
                stream->pubimbue ( loc );
            };

            container.doAtomically ( stream, [ & ] ( ) { imbuement ( ); } );
        }
        /**
         * @brief Changes the underlying buffer to a user defined array
         * @note This function acts as a near-pure passthrough. The only hint
         * is that it will block until it obtains exclusive access to the
         * underlying buffer.
         * @note This function is considered part of the "one-at-a-time"
         * guarantee since it modifies the underlying streambuf.
         * @param s the user defined array
         * @param n the size of the user defined array
         * @return std::basic_streambuf<CharT, Traits>* the value returned from
         * the underlying streambuf
         */
        std::basic_streambuf<CharT, Traits> *
                setbuf ( CharT *s, std::streamsize n ) override
        {
            std::basic_streambuf<CharT, Traits> *result;
            auto                                 bufferset = [ & ] ( ) {
                std::scoped_lock<std::mutex> streamLock ( streamMutex );
                result = stream->pubsetbuf ( s, n );
            };

            container.doAtomically ( stream, [ & ] ( ) { bufferset ( ); } );
            return result;
        }

        /**
         * @brief Seeks to a specific offset.
         * @note this function acts as a near-pure passthrough. The only hint is
         * that it will block until it obtains exclusive access to  the
         * underlying buffer.
         * @note This function is part of the "one-at-a-time" guarantee.
         * @param off the offset
         * @param dir the direction
         * @param which which part of the buffer to move. (by default, all
         * streambuf's are in and out).
         * @return Traits::pos_type the value returnede from the underlying
         * buffer.
         */
        Traits::pos_type seekoff (
                Traits::off_type        off,
                std::ios_base::seekdir  dir,
                std::ios_base::openmode which = std::ios_base::in
                                                | std::ios_base::out ) override
        {
            typename Traits::pos_type result;
            auto                      offseek = [ & ] ( ) {
                std::scoped_lock<std::mutex> streamLock ( streamMutex );
                result = stream->pubseekoff ( off, dir, which );
            };
            container.doAtomically ( stream, [ & ] ( ) { offseek ( ); } );
            return result;
        }

        Traits::pos_type seekpos (
                Traits::pos_type        pos,
                std::ios_base::openmode which = std::ios_base::in
                                                | std::ios_base::out ) override
        {
            typename Traits::pos_type result;
            auto                      posseek = [ & ] ( ) {
                std::scoped_lock<std::mutex> streamLock ( streamMutex );
                result = stream->pubseekpos ( pos, which );
            };
            container.doAtomically ( stream, [ & ] ( ) { posseek ( ); } );
            return result;
        }

        std::streamsize showmanyc ( ) override { return -1; }

        Traits::int_type underflow ( ) override { return Traits::eof ( ); }

        Traits::int_type uflow ( ) override { return underflow ( ); }

        std::streamsize xsgetn ( Traits::char_type *s,
                                 std::streamsize    count ) override
        {
            for ( std::streamsize i = 0; i < count; i++ ) s [ i ] = uflow ( );
            return count;
        }

        std::streamsize xsputn ( Traits::char_type const *s,
                                 std::streamsize          count ) override
        {
            std::basic_string<CharT, Traits, Allocator> str =
                    emptyString<CharT, Traits, Allocator> ( );
            for ( std::streamsize i = 0; i < count; i++ )
            {
                str += s [ i ];
            }
            std::scoped_lock<std::mutex> bufferLock ( bufferMutex );
            buffer += str;
            return count;
        }
    };

    /**
     * @brief basic_osyncstream
     * @bug Does not work, we know that the bug likely resides within the stream
     * since basic_syncstreambuf works normally with a std::basic_ostream.
     * @tparam CharT
     * @tparam Traits
     * @tparam Allocator
     */
    template <class CharT, class Traits, class Allocator>
    class basic_osyncstream : public std::basic_ostream<CharT, Traits>
    {
        // locks when accessing buffer to prevent us from
        // modifying during a write.
        std::mutex modifyBuffer;

        basic_syncstreambuf<CharT, Traits, Allocator> *buffer;

        static inline bool selfTest ( std::ostream &stream )
        {
            stream << "Beginning test for basic_osyncstream for sizeof(CharT) "
                      "= "
                   << sizeof ( CharT ) << "\n";
            stream << "Ensuring that osyncstream ... well ... works.\n";
            std::basic_stringstream<CharT, Traits, Allocator> sstream;
            static CharT text [] = { 'A', 'B', 'C', 'D', 'E', 'F', '\0' };
            basic_syncstreambuf<CharT, Traits, Allocator> *buffers =
                    new basic_syncstreambuf<
                            CharT,
                            Traits,
                            Allocator>[ std::thread::hardware_concurrency ( ) ];
            for ( unsigned i = 0; i < std::thread::hardware_concurrency ( );
                  i++ )
            {
                buffers [ i ] = basic_syncstreambuf<CharT, Traits, Allocator> (
                        sstream.rdbuf ( ) );
            }
            basic_osyncstream<CharT, Traits, Allocator> *streams =
                    new basic_osyncstream<
                            CharT,
                            Traits,
                            Allocator>[ std::thread::hardware_concurrency ( ) ];
            for ( unsigned i = 0; i < std::thread::hardware_concurrency ( );
                  i++ )
            {
                streams [ i ] = basic_osyncstream ( &buffers [ i ] );
            }

            std::atomic_size_t latch = std::thread::hardware_concurrency ( );
            auto               send  = [ & ] ( unsigned id ) {
                for ( int i = 0; i < 2; i++ )
                {
                    streams [ i ] << text;
                }
                latch.fetch_sub ( 1 );
            };
            std::thread *threads =
                    new std::thread [ std::thread::hardware_concurrency ( ) ];
            for ( unsigned i = 0; i < std::thread::hardware_concurrency ( );
                  i++ )
            {
                threads [ i ] = std::thread ( std::bind_front ( send, i ) );
            }
            for ( unsigned i = 0; i < std::thread::hardware_concurrency ( );
                  i++ )
            {
                threads [ i ].detach ( );
            }

            while ( latch.load ( ) )
            { }

            // since we haven't destructed anything, we should still have a
            // clear string
            if ( !sstream.str ( ).empty ( ) )
            {
                stream << "Some of the streams sent output when emit_on_sync "
                          "was false and they were not destroyed.\n";
                delete [] streams;
                delete [] buffers;
                delete [] threads;
                return false;
            }
            stream << "Testing what happens when the streams are now "
                      "destroyed...\n";
            delete [] streams;
            delete [] buffers;
            std::basic_string<CharT, Traits, Allocator> testString = text;
            for ( unsigned i = 1; i < std::thread::hardware_concurrency ( );
                  i++ )
            {
                testString += text;
            }
            // since we sent out all information twice, double the length of
            // testString
            testString = testString + testString;
            if ( sstream.str ( ).find ( testString ) == std::string::npos )
            {
                stream << "The values sent to the string were not the values "
                          "expected!\n";
                if ( sstream.str ( ).empty ( ) )
                {
                    stream << "In fact! Nothing was sent!!\n";
                }
                delete [] threads;
                return false;
            } else
            {
                delete [] threads;
            }

            return true;
        }

        static inline test::Unittest unittest = { &selfTest };

        void move ( basic_osyncstream &&that )
        {
            std::scoped_lock<std::mutex> modifyLock ( modifyBuffer );
            buffer      = std::move ( that.buffer );
            that.buffer = nullptr;
        }

        // default constructor, may make public some day
        basic_osyncstream ( )
                : basic_osyncstream (
                        new basic_syncstreambuf<CharT, Traits, Allocator> ( ) )
        { }
    public:
        basic_osyncstream ( std::basic_streambuf<CharT, Traits> *buf,
                            Allocator const                     &a )
                : basic_osyncstream ( buf )
        { }
        explicit basic_osyncstream ( std::basic_streambuf<CharT, Traits> *buf )
                : buffer ( new basic_syncstreambuf ( buf ) )
        { }
        basic_osyncstream ( std::basic_ostream<CharT, Traits> &os,
                            Allocator const                   &a )
                : basic_osyncstream ( os.rdbuf ( ), a )
        { }
        basic_osyncstream ( basic_osyncstream &&that ) noexcept
        {
            move ( std::move ( that ) );
        }

        basic_osyncstream &operator= ( basic_osyncstream &&that ) noexcept
        {
            move ( std::move ( that ) );
            return *this;
        }

        ~basic_osyncstream ( ) { emit ( ); }

        basic_syncstreambuf<CharT, Traits> *rdbuf ( ) const noexcept
        {
            return buffer;
        }

        std::basic_streambuf<CharT, Traits> *get_wrapped ( ) const noexcept
        {
            // std::scoped_lock<std::mutex> modifyLock ( modifyBuffer );
            if ( buffer )
            {
                return buffer->get_wrapped ( );
            } else
                return nullptr;
        }

        void emit ( )
        {
            std::scoped_lock<std::mutex> modifyLock ( modifyBuffer );
            if ( buffer )
            {
                if ( !buffer->emit ( ) )
                {
                    std::cout << "emission failed.\n";
                }
            }
        }
    };

    template class basic_syncstreambuf<defines::ChrChar>;
    template class basic_syncstreambuf<defines::U08Char>;
    template class basic_syncstreambuf<defines::U16Char>;
    template class basic_syncstreambuf<defines::U32Char>;

    template class basic_osyncstream<defines::ChrChar>;
    template class basic_osyncstream<defines::U08Char>;
    template class basic_osyncstream<defines::U16Char>;
    template class basic_osyncstream<defines::U32Char>;

    template class SynchronizedStreamBufferImplementation<
            defines::ChrChar,
            std::char_traits<defines::ChrChar>,
            std::allocator<defines::ChrChar>>;
    template class SynchronizedStreamBufferImplementation<
            defines::U08Char,
            std::char_traits<defines::U08Char>,
            std::allocator<defines::U08Char>>;
    template class SynchronizedStreamBufferImplementation<
            defines::U16Char,
            std::char_traits<defines::U16Char>,
            std::allocator<defines::U16Char>>;
    template class SynchronizedStreamBufferImplementation<
            defines::U32Char,
            std::char_traits<defines::U32Char>,
            std::allocator<defines::U32Char>>;

    using syncbuf      = basic_syncstreambuf<char>;
    using wsyncbuf     = basic_syncstreambuf<wchar_t>;
    using osyncstream  = basic_osyncstream<char>;
    using wosyncstream = basic_osyncstream<wchar_t>;
} // namespace io::base