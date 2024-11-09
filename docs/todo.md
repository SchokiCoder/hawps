- [ ] test android build
`sudo apt install google-android-ndk-r26d-installer or newer version`

- [ ] add runtime dot manipulation interface
- [ ] remove manual tomfoolery
- [ ] fix liquids running past grains ?

# Gasy Update

- [x] add a none mat state for the none mat

- [x] add server client split
Send via UDP.
Server calculates dots.
Default client draws them to screen via SDL,
and is listens to localhost.
Other clients may draw them to a console or print them or whatever.
I won't tell you what to do.
	- [x] add actual networking

- [ ] add argument ip address on client
- [ ] add argument port on client and server

- [ ] change license to LGPL-2.1-only ?

- [ ] add solid dots

- [ ] add oxygen and gas behavior

- [ ] set version to 0.3

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

-----

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
