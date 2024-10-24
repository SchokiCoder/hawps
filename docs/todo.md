- [ ] add runtime dot manipulation interface
- [ ] remove manual tomfoolery
- [ ] would disabling friction calculations while gravity is 0, save cpu time?

# Gases Update

- [ ] add dot friction?
upon colliding with a grounded dot,
get grounded and use friction average of own and below dot mul by grav and delta

- [ ] add dot bounciness?

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

- [ ] towers of sand must collapse

- [ ] add arguments for
	- [ ] gfxScale
	- [ ] gravity
	- [ ] groundFriction
	- [ ] tickrate
	- [ ] timescale
	- [ ] worldWidth
	- [ ] worldHeight
	- [ ] `-h --help`

- [ ] set version to 0.2

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
