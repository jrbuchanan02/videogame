/**
 * @file macros.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief generic macros for general use
 * @version 1
 * @date 2022-02-09
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once
#include <defines/types.h++>
#ifndef SOURCE_DEFINES_MACROS
#    define SOURCE_DEFINES_MACROS

#    define CATCH_AND_GIVE_LINE( Exception, ... )                              \
        try                                                                    \
        {                                                                      \
            __VA_ARGS__                                                        \
        } catch ( Exception & except )                                         \
        {                                                                      \
            defines::ChrStringStream stream;                                   \
            stream << "Threw error on line " << __LINE__ << " of file "        \
                   << __FILE__ << "\n";                                        \
            throw std::runtime_error ( stream.str ( ) );                       \
        }

#    ifdef WINDOWS
#        define NEWLINE "\r\n"
#    else
#        define NEWLINE "\n"
#    endif
#endif // ifndef SOURCE_DEFINES_MACROS