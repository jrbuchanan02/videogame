# Videogame
You can find the [license](./LICENSE) [here](./LICENSE): [link](./LICENSE)
(all three links lead to the license). Videogame is published under the GNU GPL
version 3.0 with the hopes that it will be entertaining, but comes "as-is" with
no warranty, neither express nor implied, of any kind.

 Terminal Based RPG with Good Graphics. Videogame is an ambitious project with 
 the following goals:

 1. Create unbelievably intricate graphics using only the terminal emulator 
 built into the operating system.
 2. Create an entertaining RPG experience that takes a theoretical minimum of 
 one hour to complete.
 3. Achieve the above two goals using only the standard library and, where 
 necessary, the Windows API.

 As of the time of writing, Videogame is nowhere near reaching either goal. The
 game is not even currently playable. However, you can run the unit-tests for 
 the engine. Videogame compiles against g++ version 10.3 and uses the C++ 20 
 standard. However, aside from those two requirements, Videogame should build
 and function properly on most / all Windows / Linux systems (64-bit systems).


## System Requirements:
### Base
1. Around 7 MB RAM free. This requirement will change as Videogame gets more
complete. I apologize to those still attempting to game on a 286.
2. Any version of Linux or Windows 10 20H1 or later (20H2 or any version of 
Windows 11).
   - In other words, any version / flavor of linux where Videogame will build
   supports running Videogame.
   - On unsupported versions of Windows, Videogame will put garbage on the screen. 
   This is because unsupported versions of Windows do not speak UTF-8. Perhaps
   that will be fixed in the future, but there are currently no plans to do so.
3. A display capable of rendering CGA graphics (or equivalent) or later.
   - Almost every modern device passes this requirement. It is just here for 
   completion / humor purposes.
4. Keeping the [license](./LICENSE) file next to the executable. While Videogame
does not currently check for this license, Videogame is licensed under the GPL
version 3.0, and my understanding of that license is that it must remain with the
work.
### Recommended
1. If on Windows, having Windows Terminal installed. Windows Terminal is not 
required to play Videogame, but it allows you to perform cool graphical tricks
with Videogame such as setting the font, setting the starting-window size, and
more.
2. A display capable of housing 120 columns onscreen at once. I just like the
widescreen effect.

## Roadmap:
Here are the steps in our roadmap. As Videogame gets more complete, the roadmap
will become more fleshed out. However, the basic todo-list has taken shape. Items
marked with [x] are incomplete, items marked with a [~] are in progress, and items
without an [x] or [~] are complete.

1. [~] The Console Window
   - [~] The ability to output text at a specified rate with the following 
   control over it in addition to the control provided by the Terminal Sequences:
      + [x] Exact color of the eight indexed colors and their movements over 
      a specified time.
         * Movements must allow multiple sweeps to both directly specified (rgb-value)
         colors and to indirectly specified (other colors we're potentially changing
         through time) colors. These sweeps should include a sinusoidal function, 
         linear sweep, and more.
      + [~] The text justification
   - [~] The ability to output text that the user sees while modifying the 
   attributes of all text onscreen.
   - [x] The ability to programmatically determine the console window's operating
   environment and to properly set the Console Window into a known, default state.
2. [x] The Serialization / Deserialization
   - [x] The ability to programmatically detect which data files are available for
   loading, including the ability to ignore or repair corrupted data files.
   - [x] The ability to load all localization data from data-files, correctly 
   determine back-up locales (ex, en-US for en-UK), and correctly "internalize"
   strings.
   - [x] The ability to both load and save all data about what objects are in 
   the game, including their current states, interactions, and locations.
3. [~] The Engine Functionality
   - [x] The ability to interpret the commands from the player and determine 
   what to do with them.
   - [~] Intricate Pseudo-random number generation which takes two flavors: one 
   flavor where the player, if so skilled, may manipulate it, and another which
   brings randomness into seemingly completely certain game-events.
   - [x] Multiple engine states including a menu, cutscene, tutorial, and in-game
   state.