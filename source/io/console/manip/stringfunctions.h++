/**
 * @file stringfunctions.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Functions for string manipulation
 * @version 1
 * @date 2022-01-25
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#pragma once

#include <functional>
#include <string>

namespace io::console::manip
{
    using ANSISequence = std::string;
#define PARAM_FREE_CODE(X) ANSISequence const X() noexcept


#define C0Code(X) PARAM_FREE_CODE ( X )
    C0Code ( nul );
    C0Code ( soh );
    C0Code ( stx );
    C0Code ( etx );
    C0Code ( eot );
    C0Code ( enq );
    C0Code ( ack );
    C0Code ( bel );
    C0Code ( bs );
    C0Code ( ht );
    C0Code ( lf );
    C0Code ( vt );
    C0Code ( ff );
    C0Code ( cr );
    C0Code ( so );
    C0Code ( si );
    C0Code ( dle );
    C0Code ( dc1 );
    C0Code ( dc2 );
    C0Code ( dc3 );
    C0Code ( dc4 );
    C0Code ( nak );
    C0Code ( syn );
    C0Code ( etb );
    C0Code ( can );
    C0Code ( em );
    C0Code ( sub );
    C0Code ( esc );
    C0Code ( fs );
    C0Code ( gs );
    C0Code ( rs );
    C0Code ( us );
    C0Code ( del );
#undef C0Code
#define C1Code(X) PARAM_FREE_CODE ( X )
    // C1 Codes are ESC followed by the code
    C1Code ( bph ); // a 0-width space
    C1Code ( nbh ); // do not insert line break here. We'll try to adhere to that.
    C1Code ( ind ); // index
    C1Code ( nel ); // identical to CR LF
    C1Code ( ssa ); // useful for block-orientated terminals
    C1Code ( esa ); // useful for block-orientated terminals
    C1Code ( hts ); // sets tab stop at current position
    C1Code ( htj ); // similar to hts, but like one character before?
    C1Code ( vts ); // like hts, but vertical?
    C1Code ( pld ); // partial line downward (we don't expect support)
    C1Code ( plu ); // partual line upward (we don't expect support)
    C1Code ( ri ); // reverse index. Like LF, but the opposite
    C1Code ( ss2 ); // has one column width
    C1Code ( ss3 ); // has one column width
    C1Code ( dcs ); // terminated via ST
    C1Code ( pu1 ); // private use 1. We'll kindly use that?
    C1Code ( pu2 ); // private use 2. We'll kindly use that?
    C1Code ( sts ); // Wikipedia has no info on this one.
    C1Code ( cch ); // Like Backspace, but assertively destructive.
    C1Code ( mw ); // message waiting. Is that a question or statement?
    C1Code ( spa ); // block-orientated terminals
    C1Code ( epa ); // block-orientated terminals
    C1Code ( sos ); // start of string. Ends with ST, but contains any character
                 // other than SOS or ST
    C1Code ( sgc ); // single graphic chracter introducer. Doesn't actually do anything.
    C1Code ( sci ); // single character introducer. No standard on what it does.
    C1Code ( csi ); // DOES SO MUCH!!!
    C1Code ( st );
    C1Code ( osc ); // also does so much
    C1Code ( pm );
    C1Code ( apc );
#undef C1Code
    // any code that uses CSI
    // these are defined in the file because, unlike the above commands, they 
    // either take parameters or have a more complicated string.

    // cursor position instructions that take one argument
    ANSISequence const cuu ( std::uint32_t const &n = 0 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "A";
        return result;
    }
    ANSISequence const cud ( std::uint32_t const &n = 0 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "B";
        return result;
    }
    ANSISequence const cuf ( std::uint32_t const &n = 0 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "C";
        return result;
    }
    ANSISequence const cub ( std::uint32_t const &n = 0 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "D";
        return result;
    }
    ANSISequence const cnl ( std::uint32_t const &n = 0 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "E";
        return result;
    }
    ANSISequence const cpl ( std::uint32_t const &n = 0 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "F";
        return result;
    }
    ANSISequence const cha ( std::uint32_t const &n = 0 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "G";
        return result;
    }

    // cursor position instruction that takes two arguments
    ANSISequence const cup ( std::uint32_t const &n = 1 , std::uint32_t const &m = 1 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += ";";
        result += std::to_string ( !m ? m + 1 : m );
        result += "H";
        return result;
    }

    ANSISequence const hvp ( std::uint32_t const &n = 1 , std::uint32_t const &m = 1 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += ";";
        result += std::to_string ( !m ? m + 1 : m );
        result += "f";
        return result;
    }

    enum EraseInDisplayOptions
    {
        ED_CURSOR_TO_END = 0 , // erases from cursor to end of screen.
        ED_TOP_TO_CURSOR = 1 , // erases from beginning of screen to cursor
        ED_ENTIRE_SCREEN = 2 , // erases entire screen and may move cursor to top left
        ED_ENTIRE_BUFFER = 3 , // erases entire screen and scrollback.
    };

    ANSISequence const ed ( EraseInDisplayOptions const &n = ED_CURSOR_TO_END ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( n );
        result += "J";
        return result;
    }


    enum EraseInLineOptions
    {
        EL_CURSOR_TO_END = 0 , // erases from cursor to end of line
        EL_BEG_TO_CURSOR = 1 , // erases from beginning of line to cursor
        EL_ENTIRE_LINE = 2 , // erase entire line
    };

    ANSISequence const el ( EraseInLineOptions const &n = EL_CURSOR_TO_END ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( n );
        result += "K";
        return result;
    }

    ANSISequence const su ( std::uint32_t const &n = 1 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "S";
        return result;
    }
    ANSISequence const sd ( std::uint32_t const &n = 1 ) noexcept
    {
        ANSISequence result = csi ( );
        result += std::to_string ( !n ? n + 1 : n );
        result += "T";
        return result;
    }

    // SGR intentionally omitted since we are implementing those functions directly
    // DSR intentionally omitted since we will use that internally to find the 
    // point where the screen wraps

    namespace sgr
    {
        ANSISequence const normal ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "0";
            result += "m";
            return result;
        }
        ANSISequence const bolded ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "1";
            result += "m";
            return result;
        }
        ANSISequence const dimmed ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "2";
            result += "m";
            return result;
        }
        ANSISequence const italic ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "3";
            result += "m";
            return result;
        }
        ANSISequence const ulined ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "4";
            result += "m";
            return result;
        }
        // "slow" blink, what's commonly supported ( < 150 x per minute)
        ANSISequence const sblink ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "5";
            result += "m";
            return result;
        }
        // "fast" blink, rarely supported. ( > 150 x per minute)
        ANSISequence const fblink ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "6";
            result += "m";
            return result;
        }
        ANSISequence const invert ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "7";
            result += "m";
            return result;
        }
        // not widely supported.
        ANSISequence const hidden ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "8";
            result += "m";
            return result;
        }
        // legible but marked as if for deletion,
        // apparently not supported on Mac OS
        ANSISequence const struck ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "9";
            result += "m";
            return result;
        }

#define SET_FONT(N) ANSISequence const font##N ( ) noexcept                     \
        {                                                                       \
            ANSISequence result = csi();                                        \
            result += std::to_string ( 10 + N );                                \
            result += "m";                                                      \
            return result;                                                      \
        }

        SET_FONT ( 0 )
            SET_FONT ( 1 )
            SET_FONT ( 2 )
            SET_FONT ( 3 )
            SET_FONT ( 4 )
            SET_FONT ( 5 )
            SET_FONT ( 6 )
            SET_FONT ( 7 )
            SET_FONT ( 8 )
            SET_FONT ( 9 )
#undef SET_FONT
            // visual studio code is ... good at formatting...?
            ANSISequence const gothic ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "20";
            result += "m";
            return result;
        }

        // Warning: on some systems this means "not bold"
        ANSISequence const dlined ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "21";
            result += "m";
            return result;
        }

        // returns text to normal intensity
        // normal was taken.
        ANSISequence const unbold ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "22";
            result += "m";
            return result;
        }
        // not italic
        ANSISequence const nitalc ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "23";
            result += "m";
            return result;
        }
        // not underlined
        ANSISequence const nlined ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "24";
            result += "m";
            return result;
        }

        ANSISequence const nblink ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "25";
            result += "m";
            return result;
        }
        // not invert
        ANSISequence const nnvert ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "26";
            result += "m";
            return result;
        }
        // opposite of hidden
        ANSISequence const reveal ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "27";
            result += "m";
            return result;
        }
        // not struck through
        ANSISequence const nstruk ( ) noexcept
        {
            ANSISequence result = csi ( );
            result += "28";
            result += "m";
            return result;
        }

    }
}