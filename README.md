[![C/C++ CI](https://github.com/jrbuchanan02/videogame/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/jrbuchanan02/videogame/actions/workflows/c-cpp.yml)
[![CodeQL](https://github.com/jrbuchanan02/videogame/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/jrbuchanan02/videogame/actions/workflows/codeql-analysis.yml)
# Videogame
You can find the [license](./LICENSE) [here](./LICENSE): [link](./LICENSE)
(all three links lead to the license). Videogame is published under the GNU GPL
version 3.0 with the hopes that it will be entertaining, but comes "as-is" with
no warranty, neither express nor implied. Videogame has open-source
dependencies and databases. These are published under the Boost Software License, 
the Unicode License, and the MIT License. All dependencies and databases also 
come "as-is" and with no warranty, neither express nor implied.

It may seem implied, but Videogame is a working title. Since the game is not 
currently playable, I am not concerned about figuring out a plot, but about
creating the most versatile engine for whatever that plot will be.

 Terminal Based RPG with Good Graphics. Videogame is an ambitious project with 
 the following goals:

 1. Create unbelievably intricate graphics using only the terminal emulator 
 built into the operating system.
 2. Create an entertaining RPG experience that takes a theoretical minimum of 
 one hour to complete (similar to how drag-race for the Atari 2600 cannot be
 completed in less than 5.51 seconds).
 3. Achieve the above two goals using minimal external dependencies.
 4. Generate a truly portable experience: if someone removes videogame, the 
 system is left in the exact state that it was in prior to the install
 (excluding any changes other than those Videogame may have made).

 As of the time of writing, Videogame is nowhere near reaching either goal 1 or 2.
 The game is not even currently playable. However, you can run the unit-tests for 
 the engine. Videogame compiles against g++ version 10.3 and uses the C++ 20 
 standard. However, aside from those two requirements, Videogame should build
 and function properly on most / all Windows / Linux systems (64-bit systems).


## System Requirements:
### Base
1. Around ~~50~~ 200 MB RAM free[^1]. This requirement will change as Videogame gets more
complete. If your computer cannot meet this requirement, the advice is: get a 
better computer. There is a good chance a more skilled programmer than me could 
save some RAM, as most of the requirement consists of loading in an
approximately 40 MB database for processing.
2. Any version of Linux or Windows 10 20H1 or later (20H2 or any version of 
Windows 11).[^2]
   - If it becomes possible, I would take interest in porting Videogame to retro
   systems which support the necessary terminal commands. If you know such a 
   retro system well enough, feel free to make a contribution!
   - Unfortunately, because of how Videogame will manipulate some colors on the 
   screen, it may not ever work on Mac OS. Both Windows and Linux use (similar)
   proprietary commands for this specific color manipulation. Mac OS, as far as
   I know, does not support this command. This does not mean that Videogame will
   *never* run on a Darwin-based system, but that Videogame would lose a fundamental
   component of its engine when running on a Darwin-based system.
3. A computer capable of approximately 5,000 floating point operations per second
 and capable of multithreading.
4. A display capable of rendering CGA graphics (or equivalent) or later.
   - Almost every modern device passes this requirement. It is just here for 
   completion / humor purposes.
5. Keeping the [license](./LICENSE) file next to the executable. While Videogame
does not currently check for this license, Videogame is licensed under the GPL
version 3.0, and my understanding of that license is that it must remain with the
work.
6. Around 60 MB of storage free.[^6]
### Recommended
1. If on Windows, having Windows Terminal installed. Windows Terminal is not 
required to play Videogame, but it allows you to perform cool graphical tricks
with Videogame such as setting the font, setting the starting-window size, and
more. In general, I would recommend Windows Terminal for normal use.
2. A display capable of housing 120 columns onscreen at once.

[^1]: This measurement comes from connecting videogame.exe to the Microsoft
Visual Studio 2022 Community Edition Preview running on Windows 11 Home for
an ACPI x64 PC. Your results will probably vary since I was debugging something
and probably had some of the compilation arguments different. When loaded, however
Videogame uses around 3 MB of RAM. The reason for this massive difference is 
loading in the Unicode character database. (in fact, 200 MB RAM utilization 
implies that each byte in the file is copied less than five times -- impressive).
This amount could be lowered by splitting the unicode character database into
multiple files, something not really worth the effort of working with a file
containing more than 150k lines!

[^2]: If you are on the most recent version of windows available for your PC and
you have Windows 10 or Windows 11, then you meet this requirement. This requirement
is for the commands that Videogame sends to the console. While these commands 
should function properly on every version of Linux, only the more recent
feature-updates to windows provde support for these commands.

## Roadmap:
Here are the steps in our roadmap. As Videogame gets more complete, the roadmap
will become more fleshed out. However, the basic todo-list has taken shape. Items 
checked off are fully complete. Items checked off but also crossed out might be 
complete but have not been thouroughly (however you spell that word) tested.

1. [ ] The Console Window
   - [x] The ability to output text at a specified rate with the following 
   control over it in addition to the control provided by the Terminal Sequences:
      + [x] Exact color of the eight indexed colors and their movements over 
      a specified time.
         * Movements must allow multiple sweeps to both directly specified (rgb-value)
         colors and to indirectly specified (other colors we're potentially changing
         through time) colors. These sweeps should include a sinusoidal function, 
         linear sweep, and more.
      + [ ] The text justification
   - [x] The ability to output text that the user sees while modifying the 
   attributes of all text onscreen.
   - [x] The ability to programmatically determine the console window's operating
   environment and to properly set the Console Window into a known, default state.
2. [ ] The Serialization / Deserialization
   - [ ] The ability to programmatically detect which data files are available for
   loading, including the ability to ignore or repair corrupted data files.
   - [ ] The ability to load all localization data from data-files, correctly 
   determine back-up locales (ex, en-US for en-UK), and correctly "internalize"
   strings.
   - [ ] The ability to both load and save all data about what objects are in 
   the game, including their current states, interactions, and locations.
3. [ ] The Engine Functionality
   - [ ] The ability to interpret the commands from the player and determine 
   what to do with them.
   - [x] Intricate Pseudo-random number generation which takes two flavors: one 
   flavor where the player, if so skilled, may manipulate it, and another which
   brings randomness into seemingly completely certain game-events.
   - [ ] Multiple engine states including a menu, cutscene, tutorial, and in-game
   state.

## Version Numbering
Videogame uses a modified version of symver. Videogame uses the shortest version number
possible while giving each version a unique number which always counts up. 
So, v0 is the first version, then, if there is bug fix, the version would increment to
v0.1 (never using v0.0). This is to keep the version number as short as reasonable. 
Videogame's source files uses a simple counter numbering-scheme where a version 
increment occurs whenever a major rewrite occurs. (none have happened yet, so all
of videogame's source files are version 1).

## Dependencies
Videogame intends on having minimal dependencies. All external code is currently
contained within the `extern` folder. The complete list of all program contents
which are not the intellectual property of Videogame's creators are down below:

- The Unicode Character Database, copyright 2021[^3] Unicode Inc.[^4].
 Videogame uses the UCD version 14.0, which is licensed under the open-source
Unicode License. You can read the unicode license [here](https://www.unicode.org/license.txt),
or, at https://www.unicode.org/license.txt (the link points there).
- Rapidxml, copyright 2006, 2007 Marcin Kalicinski. Videogame uses Rapidxml 
version 1.13, which is copyright under the open-source Boost Software License. 
You can read the Boost Software license [here](./extern/rapidxml-1.13/license.txt),
or in the file ./extern/rapidxml-1.13/license.txt (the link points there).
- YAML-CPP, copyright 2008-2015 Jesse Beder under the MIT license. You can find 
the license [here](./extern/yaml-cpp/LICENSE)
[^3]: I may have the year off on the UCD 14.0's copyright date. However, we can
infer from the age of Unicode version 14.0 to know that the copyright year listed
is around that time.

[^4]: Unicode Inc. is the name of the organization as listed in the license.

[^6]: The videogame executable occupies between 1 and 0.75 MiB, the unicode 
character database requires around 40MiB. The remaining 9 MiB accounts for 
anticipated growth in the size of the executable and for anticipated binary
files. If you have a modern computer and your storage is low enough check your
disk stats, I would recommend upgrading your storage.
