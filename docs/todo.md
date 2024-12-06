- [ ] test android build
`sudo apt install google-android-ndk-r26d-installer or newer version`

- [ ] fix liquids running past grains ?

# UI update

- [x] remove world size args
- [x] make all args single hyphen

- [x] make fullscreen default
- [x] add arg "-window" "-windowed"
- [x] add arg "-noborder"
Also fix "-window" flag stopping arg parsing.
- [x] add arg "-width" "-W"
- [x] add arg "-height" "-H"
- [x] update help text to show which arg needs a parameter

- [x] add UI base
- [x] refine UI base and add basic data structs
- [x] add basic tile drawing

- [x] nuke everything, begin again
- [x] rewrite world
Also I feel very dumb for how I used to think `go:generate` works.
Also also the chemical reactions in the C version must have been bugged,
without me knowing it.
The indexing was wrong, but it didn't trigger fsanitize... Too bad.

- [x] add tool images
- [x] add UI base
	- [x] add ui tileset
	- [x] add basic ui layout
- [x] precalculate geoMs of Tool- and Matbox
- [x] properly fit Matbox
- [x] add material images
- [x] add basic mat image generation
- [x] add color swapping to mat image generation

- [ ] add tool box functionality
- [ ] add mat box functionality
- [ ] add tall UI layout and automatic layout determination
- [ ] add arg "-wide_layout"
- [ ] add arg "-tall_layout"
- [ ] add automatic world size, fitting remaining space

- [ ] add UI key binds
	- [ ] add arrow left/right for tool switching
	- [ ] add arrow up/down for mat switching
- [ ] add "hold ALT key" to display what UI element uses which key bind

- [ ] set version to 0.4

-----

# Gasy Update

- [x] add a none mat state for the none mat

- [x] add server client split
Server calculates dots.
Default client draws them to screen via SDL,
and is listens to localhost.
Other clients may draw them to a console or print them or whatever.
I won't tell you what to do.
	- [x] add actual networking

- [x] improve memory layout of world and TCP read/write of world

- [x] add argument for setting ip address on client
- [x] add argument for setting port on client and server

- [x] add iron and solid dots

- [x] change license to LGPL-2.1-only

- [x] add oxygen, hydrogen and gas behavior

- [x] add reaction to oxygen meeting hydrogen

- [x] fix arg --tickrate missing
I forgot to bring it from server.c.

- [x] set version to 0.3

# Liquid Update

- [x] add water

- [x] fix visual flickering

- [x] rework movement and collision to be MUCH simpler
No more floats. No velocities. Just a simple matrix.
A dot always tries to move every tick.

- [x] add material displacement

- [x] add more readable way of declaring material properties

- [x] fix sdl.Surface "frame" not being freed

- [x] add pause button

- [x] add stack collapsing via gravity
	- [x] fix liquid collapsing incorrectly iterating loops
	- [x] fix grain collapsing making grains jump up
	- [x] fix basic liquid collapsing
	- [x] fix grain collapsing being blocked by something
	- [x] fix grain collapsing letting dots disappear

- [x] add arguments for
	- [x] tickrate
	- [x] dotscale
	- [x] world_width
	- [x] world_height
	- [x] help print
	- [x] version info print

- [x] change dotscale to world_scale

- [x] set version to 0.2

# Base (done)

+ remove Godot implementation, see docs/reworks.md

Also add basic data models in Go.

+ add graphics

+ add dot velocity and movement

This also caused the implementation of a fixed tick- and framerate.
Velocity pretty much only makes sense as a float,
which caused the obsolescence of the world data type,
which was initially meant as a way to quickly index via position.

+ fix dot draw ignoring gfxScale in regards to pos
+ add gravity
+ add timescale
+ add bounds collision
+ add ground friction
+ check if gravity calculation makes sense

+ enforce new formatting for function declarations

Putting the first parameter already on a new line,
and the closing parenthesis with return type and opening bracket on a
new line fixes everything I disliked so far.
There is a clear cut between params and function start,
which often is var declarations.
No more weird alignment because of first param vs the other.

+ add dot collision

+ add about print argument

Also add quit keybind (ESC), and
set version to 0.1
