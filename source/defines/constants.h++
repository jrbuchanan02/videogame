/**
 * @file constants.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Constants
 * @version 1
 * @date 2022-02-13
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/macros.h++>
#include <defines/types.h++>

namespace defines
{
    constexpr ChrPString ucdDataFile = "ucd.all.grouped.xml";
    constexpr ChrPString ucdDataPath = "./data/unicode/";
    constexpr ChrPString ucdDataName = "./data/unicode/ucd.all.grouped.xml";

    constexpr ChrPString forwardSlash  = "/";
    constexpr ChrPString backwardSlash = "\\";
    constexpr ChrPString yamlExtension = ".yaml";

    constexpr ChrPString dataFolderName   = "data";
    constexpr ChrPString textFolderName   = "text";
    constexpr ChrPString screenFolderName = "screen";

    // highest control character
    constexpr defines::ChrChar maximumControlCharacter = ( char ) 0x1F;

    // highest ASCII byte
    constexpr defines::ChrChar maximumASCII = ( char ) 0x7F;

    // lowest UTF-8 following byte
    constexpr defines::ChrChar minimumFollowing = ( char ) 0x80;
    // highest UTF-8 following byte
    constexpr defines::ChrChar maximumFollowing = ( char ) 0xBF;

    // first byte indicating the start of a two byte sequence. Any sequence
    // starting with this byte or the next one is invalid, however.
    constexpr defines::ChrChar firstTwoByte   = ( char ) 0xC0;
    // first valid 2-byte UTF-8 sequence introducer (preceeding
    // ones give an overlong sequence)
    constexpr defines::ChrChar minimumTwoByte = ( char ) 0xC2;
    constexpr defines::ChrChar maximumTwoByte = ( char ) 0xDF;

    // first byte which introduces a 3-byte UTF-8 sequence
    constexpr defines::ChrChar minimumThreeByte = ( char ) 0xE0;
    constexpr defines::ChrChar maximumThreeByte = ( char ) 0xEF;

    // first byte which introduces a 4-byte UTF-8 sequence
    constexpr defines::ChrChar minimumFourByte  = ( char ) 0xF0;
    // maximum valid leading UTF-8 byte
    constexpr defines::ChrChar maximumFirstByte = ( char ) 0xF4;
    constexpr defines::ChrChar maximumFourByte  = ( char ) 0xF7;

    // mask used for four-byte UTF-8 sequences
    constexpr defines::ChrChar fourByteMask = ( char ) 0xF8;

    // maximum 2-byte UTF-8 encoded sequence + 1
    constexpr defines::U32Char maximumTwoByteEncoded   = 0x800;
    // maximum 3-byte UTF-8 encoded sequence + 1
    constexpr defines::U32Char maximumThreeByteEncoded = 0x10000;

    // UTF-16 (aka UCS-2) has a deadzone where it cannot encode characters. Each
    // potential code point in this deadzone also "happens" to be part of a
    // surrogate pair
    constexpr defines::U32Char ucs2Deadzone [ 2 ]          = { 0xD800, 0xDFFF };
    // maximum valid unicode code point
    constexpr defines::U32Char maxUnicode                  = 0x10FFFF;
    // the space character in UTF-8. It's more useful than you think.
    constexpr defines::ChrChar space                       = ' ';
    // single-width ASCII start and end
    constexpr defines::U32Char singleWidthASCIIRange [ 2 ] = { U'!', U'~' };
    // double-width ASCII start and end
    constexpr defines::U32Char doubleWidthASCIIRange [ 2 ] = { U'！', U'～' };

    // how many tests to do against the sigmaCheck function
    constexpr std::uint64_t sigmaTestSamples = 1000000;
    // the standard-deviation values in order from "always passes" to "never
    // passes"
    constexpr std::int64_t  sigmaCheckValues [] =
            { -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };

#ifdef WINDOWS
    // the string immediately following OSC for the windows palette
    // modification command
    constexpr ChrPString paletteChangePrefix = "4;";
    // the string immediately following the hex digit specifying the color to
    // modify on the windows palette modification command
    constexpr ChrPString paletteChangeSpecif = ";rgb:";
    // the string between hex values for the color of the new palette entry on
    // the windows color palette modifying command
    constexpr ChrPString paletteChangeDelimt = "/";
#else
    // the string immediately following OSC for the linux palette
    // modification command
    constexpr ChrPString paletteChangePrefix = "P";
    // the string immediately following the hex digit specifying the color to
    // modify on the linux palette modification command
    constexpr ChrPString paletteChangeSpecif = "";
    // the string between hex values for the color of the new palette entry on
    // the linux color palette modifying command
    constexpr ChrPString paletteChangeDelimt = "";
#endif
    // number of entries in the color palette.
    constexpr std::size_t consolePaletteLength = 8;

    // default console colors. Sort of a "major fallback".
    constexpr std::uint8_t defaultConsoleColors [ consolePaletteLength ][ 3 ] =
            {
                    { 0x00, 0x00, 0x00 },
                    { 0x7F, 0x00, 0x00 },
                    { 0x00, 0x7F, 0x00 },
                    { 0x7F, 0x7F, 0x00 },
                    { 0x00, 0x00, 0x7F },
                    { 0x7F, 0x00, 0x7F },
                    { 0x00, 0x7F, 0x7F },
                    { 0x7F, 0x7F, 0x7F },
    };

} // namespace defines