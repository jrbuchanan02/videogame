# Videogame
 Terminal Based RPG with Good Graphics

 Videogame has the following goals:
 1. [Generate Demo-Style Effects](#generating-demo-style-effects-on-a-stock-terminal-emulator) on a "stock" Linux / Windows based terminal 
 emulator.
    - Note: these include the Windows Terminal, Terminix, ux-term, etc.
    - That is: use only ANSI Terminal sequences and Linux / Windows Operating
    System Commands.
 2. Create a [highly-randomized](#randomness-in-our-engine) RPG experience that 
 lasts at least one hour from starting a new file to beating the game (minimum %
 complete) and at least ten hours to fully 100% complete a file from start to 
 finish (through say, all items, all quests, etc.)
 3. Use no external libraries.
    - That is: the only external parts of our code should be either the Windows 
    API or the C++ standard library.

## Generating Demo Style Effects on a Stock Terminal Emulator
Starting with the Windows 10/11 WSL 2 update, the builtin Windows terminal 
emualtor supports ANSI Terminal escape sequences. While on current versions of 
Windows the program needs to make [some API Calls](#the-required-api-calls), 
these API calls enable the amazing-looking [visual effects](#the-visual-effects)
 that Videogame will use.

### The Visual Effects
First, colors. In the terminal we're using, we can split the current color of 
the background / foreground into four different categories:
1. Eight indexed colors
    - These colors default to:
        + black
        + red
        + green
        + orange / yellow (implementation-defined)
        + blue
        + magenta
        + cyan
        + white
    - Notably, on the terminals we're targeting, there is an operating system 
    command which allows us to set these eight colors to any 24-bit RGB value
    we so choose. While this code varies between Windows 
    (`OSC;4;n;rgb:rr/gg/bbST`) and Linux (`OSCPnrrggbb`) (here, n is the number 
    of the color to change, OSC is the operating system command, `\u001b]`, 
    `rr`, `gg`, and `bb` are the components of the new color in hexadecimal, and 
    ST is the string terminator, `\u001b\`), the basic format is the same 
    between the two operating systems.
2. One color out of a selection of 256 colors.
    - The first eight are the same as the 
    [eight indexed colors](#the-visual-effects), however, I have not researched
    the standard enough to know if these eight colors are "deep" copies of the 
    eight CGA-style colors or if they refer to the same color.
3. One color out of the entire 24-bit color space.
    - We can choose any color.
    - Notably, this command requires the color in decimal format.
4. The default color.
    - The ANSI Terminal Sequence standard (that's not the actual name of the
    standard) requires that this color is implementation-defined. In fact, this
    color, on say the Windows Terminal, could be multiple colors as the Windows
    Terminal supports the window being partially / fully transparent and for the
    terminal background to be the desktop background.

Since we have the ability to change our colors dynamically without having an 
object actively keep track of what colors are where on the screen (the terminal
emulator does that for us), we can constantly cycle the eight indexed colors 
according to software command to achieve 
[**advanced blinking**](advanced-blinking-and-color-cycling) and 
[**color-cycling**](advanced-blinking-and-color-cycling) effects. 
These effects are controlled entirely in software, giving us complete control of
the exact curve that this color math uses and the near-exact time period for 
updating this text.

#### Advanced Blinking and Color Cycling
In order to achieve this effect, we need to potentially output text from multiple
threads to the console without interleaving or garbling the text. Fortunately, 
the C++20 standard (which Videogame compiles for) implements `std::basic_syncbuf`
and `std::basic_osyncstream`. Unfortunately, the version of g++ that I have, 10.3,
does not have standard library support for these classes. The solution is to 
implement them myself. I have already done so and the work can be found in 
[source/io/base/syncstream.h++](https://github.com/Natara1939344/videogame/blob/main/source/io/base/syncstream.h%2B%2B),
(the link points to the corresponding file in the repository). While this code 
is not exactly standard-library worthy, it works, almost exactly matches the 
standard (notably, we ignore any `Allocator` given to us), and has the same 
thread-safety guarantees that the standard library version of these classes grant.

### The Required API Calls
Example Code that Generates a shared environment between Windows and Linux is 
shown below. Note that this code assumes windows support for the virtual 
terminal processing.

```C++
#include <ostream>

constexpr auto windowsUTF8CodePage = 65001;

#ifdef WINDOWS
#ifndef UNICODE
#define UNICODE
#endif // ifndef unicode
#include "windows.h"
#endif // ifdef windows

void initConsoleEnvironment ( )
{
#ifdef WINDOWS
    HANDLE outputHandle = GetStdHandle ( STD_OUTPUT_HANDLE );
    DWORD mode;
    GetConsoleOutputMode ( outputHandle , &mode );
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleOutputMode ( outputHandle , mode );
    SetConsoleOutputCP ( windowsUTF8CodePage );
#endif // ifdef WINDOWS
    // private sequence recognized by some terminals that requires a reset
    // to the initial terminal state. This term may be ignored. if our 
    // terminal does not recognize the command.
    std::cout << "\u001bc";
    // attempt to clear screen & scrollback.
    // not all versions of windows / linux 
    // support this operation, so it might
    // be ignored or output garbage to the screen.
    std::cout << "\u001b[3J";
    // clear the screen but not the scrollback.
    // if the previous output cleared the screen,
    // then this command has no effect, if it did not
    // clear the screen, then this command does the next
    // closest thing to what we meant to do in the first
    // command. 
    std::cout << "\u001b[2J";
    // move the cursor to the top left part of the screen. If the 
    // reset to initial state does not do so already, we now ensure that
    // the cursor is in the top left corner of the screen.
    std::cout << "\u001b[H";
}
```
As you can see, we only have four Windows API calls:
1. `GetStdhandle` where we get the device used for stdout.
2. `GetConsoleOutputMode` where we get the console mode. We will modify the 
    current mode. Technically, this call is optional, but some of the bits in 
    the mode are reserved and we don't want to modify them.
3. `SetConsoleOutputMode` Enforce the new console output mode. The mode we choose
    is the original / default mode, but definitely with the virtual terminal 
    sequence processing enabled. In current versions of windows, this bit defaults
    to 0, meaning no processing (but we obviously want the processing).
4. `SetConsoleOutputCP` Tell windows that when it sees a character byte beyond 
    `0x7f`, that we're talking in Unicode and not in ... whatever the default 
    Windows character encoding is.

## Randomness in our Engine
We want most every aspect of the engine to have some element of randomness to it
in order to keep the player on their toes. Even "certain" events should have a 
minute chance of behaving unexpectedly (for example, if we have a "spell 
absorption" effect, there should be a very tiny chance of someone with that 
effect *not* absorbing a spell and vice-versa). One method to generate pass-fail
checks is to use a normal model. The engine refers to these checks as `sigmaChecks`.
Each sigma check takes a hypothetical z-score (i.e., sigma-value) and generates
a nummber on the normal model N(0,1) to compare against the z-score. If the 
generated z-score is greater than the sigma-value, the check passes.

There are two rules with Sigma Checks:
1. The game must hint when the player fails a sigma check, in case the strange 
event is not an immediate indicator.
2. If the rolled number has an absolute value >= 10, then pass anyways since 
generating a value 10 standard deviations outside the mean or more is simply 
**__that__** unlikely.
3. All skill-checks and "certain" events should be determined with a sigma check.
    - For example, if a hit will kill the player, a "certain-to-fail" sigma 
    check should occur where if the player fails, they die.
    - For example, if the player has a spell-absorption ability, and they get 
    hit by a spell, roll a "certain-to-pass" sigma check to see if they *really*
    absorb the spell.