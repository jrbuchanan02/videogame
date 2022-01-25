/**
 * @file syncbuf.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Rudimentary implementation of C++20's std::syncbuf
 * @version 1
 * @date 2022-01-22
 * 
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 * 
 */
#pragma once

#include <mutex>
#include <streambuf>

namespace game::io {
    template < class CharT , class Traits = std::char_traits < CharT > >
    class basic_syncstreambuf : public std::basic_streambuf < CharT , Traits > {
        std::mutex mutex;
        std::basic_streambuf < CharT , Traits > *around;
    public:
        basic_syncstreambuf ( std::basic_streambuf < CharT , Traits > *const &buffer ) {
            around = buffer;
        }

        virtual ~basic_syncstreambuf ( ) {
            std::scoped_lock < std::mutex > ( this->mutex );
            around = nullptr;
        }

        /**
         * @brief Indicates whether the buffer this wraps is the same exact buffer that
         * is in buffer
         *
         * @param buffer the buffer
         * @return true they are the same
         * @return false they are not the same
         */
        bool const sameAs ( std::basic_streambuf < CharT , Traits > *const buffer ) {
            return around == buffer;
        }

    protected:
        void imbue ( const std::locale &locale ) override {
            std::scoped_lock < std::mutex > ( this->mutex );
            around->pubimbue ( locale );
        }



        std::basic_streambuf<CharT , Traits>::pos_type seekoff ( std::basic_streambuf<CharT , Traits>::off_type off , std::ios_base::seekdir dir , std::ios_base::openmode which ) override {
            std::scoped_lock < std::mutex > ( this->mutex );
            return around->pubseekoff ( off , dir , which );
        }

        std::basic_streambuf<CharT , Traits>::pos_type seekpos ( std::basic_streambuf<CharT , Traits>::pos_type pos , std::ios_base::openmode which ) override {
            std::scoped_lock < std::mutex > ( this->mutex );
            return around->pubseekpos ( pos , which );
        }

        int sync ( ) override {
            std::scoped_lock < std::mutex > ( this->mutex );
            return around->pubsync ( );
        }

        std::streamsize xsgetn ( std::basic_streambuf<CharT , Traits>::char_type *s , std::streamsize count ) override {
            std::scoped_lock < std::mutex > ( this->mutex );
            return around->sgetn ( s , count );
        }

        std::streamsize xsputn ( const std::basic_streambuf<CharT , Traits>::char_type *s , std::streamsize count ) override {
            std::scoped_lock < std::mutex > ( this->mutex );
            return around->sputn ( s , count );
        }
    };

    using ssbuf = basic_syncstreambuf < char >;
    using wssbuf = basic_syncstreambuf < wchar_t>;
}