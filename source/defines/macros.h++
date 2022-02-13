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

/**
 * @brief A loop that the compiler needs assurance that it can vectorize.
 * @note for example:
 * void foo ( int n, int *a, int *b, int *c)
 * {
 *      // case 1. no overlap
 *      // case 2. A and B overlap but C does not
 *      // case 3. A and C overlap but B does not
 *      // case 4. A, B, and C all overlap
 *      // case 4a. in the same direction (say the lower or upper half of A)
 *      // case 4b. in opposite directions
 *      if ([b and c do not overlap or the bottom of a is closer to the bottoms
 *           of both b and c than the tops of both b and c])
 *      {
 *          TRICKY_LOOP_TO_VECTORIZE
 *          for (int i = 0; i < n; i++ ) a[i] = b[i] + c[i];
 *      } else if ([the bottoms of b and c are both closer to the top of a than
 * the bottom])
 *      {
 *          TRICKY_LOOP_TO_VECTORIZE
 *          for (int i = n; i; i--) a[i - 1] = b[i - 1] + c [ i - 1];
 *      } else // b and c overlap a in different directions
 *      {
 *          // this is actually hard to figure out, lmao,
 *          // but you get the point. The compiler doesn't
 *          // like the idea of messing up our code if it
 *          // tries to optimize it (with you-know, as-if and
 *          // all that). However, we sometimes can tell that
 *          // we either wouldn't care about the end result in
 *          // some of those cases (such as when the effect of
 *          // the function is undefined anyways) or that vectorizing
 *          // should be ok (as in the three cases above.)
 *      }
 * }
 */
#    define TRICKY_LOOP_TO_VECTORIZE _Prgama ( "GCC ivdep" )

#    define RUNTIME_ERROR( WHAT, ... )                                         \
        {                                                                      \
            std::stringstream message ( "" );                                  \
            message << "On LINE " << __LINE__ << " in FILE " << __FILE__       \
                    << ": " << WHAT __VA_OPT__ ( << ) __VA_ARGS__;             \
            throw std::runtime_error ( message.str ( ) );                      \
        }

#    define CATCH_AND_GIVE_LINE( Exception, ... )                              \
        try                                                                    \
        {                                                                      \
            __VA_ARGS__                                                        \
        } catch ( Exception & except )                                         \
        {                                                                      \
            defines::ChrStringStream stream;                                   \
            RUNTIME_ERROR ( "Threw " #Exception ": ", except.what ( ) )        \
        }

#    ifdef WINDOWS
#        define NEWLINE "\r\n"
#    else
#        define NEWLINE "\n"
#    endif
#    define FOREACH( IN, COLLECTION ) for ( auto &IN : COLLECTION )

#    define STREAM_AS_HEX( C, ... )                                            \
        std::hex __VA_ARGS__ std::uint64_t ( C ) __VA_ARGS__ std::dec
#    define STREAM_AS_HEX_COMMA( C )  std::hex, std::uint64_t ( C ), std::dec
#    define BEGIN_UNIT_FAIL( S, MSG ) S << "Invalid Result: " << MSG << "=> ";
#    define CHAR_UNITTEST_FAIL( S, WHY, ... )                                  \
        _Pragma ( "GCC diagnostic push" ) _Pragma (                            \
                "GCC diagnostic ignored \"-Wunused-value\"" )                  \
                        BEGIN_UNIT_FAIL ( S, WHY ) S                           \
                << std::hex __VA_OPT__ ( << ) __VA_ARGS__;                     \
        S << std::dec;                                                         \
        _Pragma ( "GCC diagnostic pop" )
#    define SYNC_UNITTEST_READON( S, WHY, ID, ... )                            \
        S << WHY << ID << __VA_OPT__ ( << ) __VA_ARGS__;
#    define END_UNIT_FAIL( S )                                                 \
        S << std::endl;                                                        \
        return false;
#    define BASIC_UNIT_FAIL( S, WHY )                                          \
        BEGIN_UNIT_FAIL ( S, "" ) S << WHY;                                    \
        END_UNIT_FAIL ( S )

#endif // ifndef SOURCE_DEFINES_MACROS