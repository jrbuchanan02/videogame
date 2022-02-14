/**
 * @file console.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief A console
 * @version 1
 * @date 2022-02-12
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>
#include <io/console/internal/channel.h++>
#include <io/console/manip/stringfunctions.h++>

#include <memory>

namespace io::console
{

    class Console
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;
    public:
        Console ( );
        Console ( Console const & );
        Console ( Console && );
        virtual ~Console ( );

        std::uint32_t getCols ( ) const noexcept;
        void          setCols ( std::uint32_t const &value ) noexcept;
        std::uint32_t getRows ( ) const noexcept;
        void          setRows ( std::uint32_t const &value ) noexcept;

    };
} // namespace io::console