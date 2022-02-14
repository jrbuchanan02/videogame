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
#include <io/console/colors/color.h++>
#include <io/console/colors/direct.h++>
#include <io/console/colors/indirect.h++>
#include <io/console/internal/channel.h++>
#include <io/console/manip/stringfunctions.h++>

#include <memory>
#include <sstream>
#include <string>

namespace io::console
{

    class Console
    {
        struct impl_s;
        std::unique_ptr< impl_s > pimpl;

        void send ( std::string const &str ) noexcept;
    public:
        Console ( );
        Console ( Console const & );
        Console ( Console && );
        virtual ~Console ( );

        std::uint32_t getCols ( ) const noexcept;
        void          setCols ( std::uint32_t const &value ) noexcept;
        std::uint32_t getRows ( ) const noexcept;
        void          setRows ( std::uint32_t const &value ) noexcept;

        template < class T >
        // clang-format off
        Console &operator<< ( T const &t ) requires (
                !std::is_same_v< T, std::u8string > 
             && !std::is_same_v< T, std::u16string >
             && !std::is_same_v< T, std::u32string > 
             && !std::is_same_v< T, char8_t > 
             && !std::is_same_v< T, char16_t > 
             && !std::is_same_v< T, char32_t > )
        {
            // clang-format on
            std::stringstream temp;
            temp << t;
            send ( temp.str ( ) );
            return *this;
        }
    };
} // namespace io::console