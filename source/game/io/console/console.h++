/**
 * @file console.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief An interface to a thread-safe console.
 * @version 1
 * @date 2022-01-22
 * 
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 * 
 */
#pragma once

#include <functional>
#include <iostream>
#include <memory>

namespace game::io::console {
    class Console;

    using ConsoleManipulator = std::function < Console & ( Console & ) >;

    class Console {
        struct impl_s;
        std::unique_ptr < impl_s > pimpl;
    public:
        Console ( std::ostream *const = &std::cout , std::istream *const = &std::cin );
        
        template < class T > Console &operator << ( T t );
    };


}