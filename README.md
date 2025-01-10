# BEAM2MID
Beam Software (GB/GBC) to MIDI converter

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex).

Note, usually (always?) the first track is empty. This is normal.

Examples:
* BEAM2MID "Bill & Ted's Excellent Gameboy Adventure (UE) [!].gb" 8
* BEAM2MID "4-in-1 Funpak (UE) [!].gb" 7
* BEAM2MID "We're Back! - A Dinosaur's Story (U) [!].gb" 6

This tool was based on my own reverse-engineering of the sound engine, partially based on minor disassembly. As usual, a "prototype" program, BEAM2TXT, is also included.

Supported games:
  * 4-in-1 FunPak
  * 4-in-1 FunPak Vol. II
  * Beavis and Butt-Head
  * Bill & Ted's Excellent Game Boy Adventure
  * Boulder Dash
  * Casino FunPak
  * Choplifter II: Rescue & Survive
  * Choplifter III
  * College Slam
  * DragonHeart
  * Fire Fighter
  * George Foreman's KO Boxing
  * Hello Kitty's Cube Frenzy
  * The Hunt for Red October
  * Itchy & Scratchy: Miniature Golf Madness
  * The Lost World: Jurassic Park
  * NBA All-Star Challenge
  * NBA All-Star Challenge 2
  * NBA Jam
  * NBA Jam: Tournament Edition
  * NBA Jam '99
  * NFL Quarterback Club
  * Out to Lunch
  * The Punisher: The Ultimate Payback
  * Solitaire FunPak
  * Stargate
  * T2: The Arcade Game
  * Taz-Mania
  * Tom and Jerry
  * Tom and Jerry: Frantic Antics
  * WCW: The Main Event
  * We're Back!: A Dinosaur's Story

## To do:
  * Support for other versions of the sound engine
  * GBS file support
