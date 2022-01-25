/**
 * @file rememberer.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Remembers items that have already beeen seen by the program
 * @version 1
 * @date 2022-01-22
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#pragma once

#include <cstdint>
#include <memory>

namespace util::memory {
    template < class T >
    class Rememberer {
        struct impl_s;
        std::unique_ptr < impl_s > pimpl;
    public:
        Rememberer ( );
        ~Rememberer ( );

        void setPeriod ( std::uint64_t const msecs );
        std::uint64_t const getPeriod ( void ) const;

        std::shared_ptr < T > get ( T & );

    };

}
#define IN_SOURCE_UTIL_MEMORY_REMEMBERER_HPP
#include "./rememberer.c++"
#undef IN_SOURCE_UTIL_MEMORY_REMEMBERER_HPP