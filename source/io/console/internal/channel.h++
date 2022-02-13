/**
 * @file channel.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A text channel
 * @version 1
 * @date 2022-02-13
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <io/console/console.h++>

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

namespace io::console::internal
{
    class TextChannel
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        using SharedFlag = std::shared_ptr< std::atomic_bool >;
        static inline std::atomic_bool defaultReady = false;

        TextChannel ( ) noexcept;
        TextChannel ( SharedFlag const & ) noexcept;
        virtual ~TextChannel ( );

        void                setDelay ( std::uint64_t const &delay ) noexcept;
        std::uint64_t const getDelay ( ) const noexcept;

        void pushString ( std::string const &string ) noexcept;

        void setReady ( SharedFlag const &ready ) noexcept;
    };
} // namespace io::console::internal