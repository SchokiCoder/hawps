# Gases Update

- add dot friction?
- add dot bounciness?

- add oxygen and gas behavior

- set version to 0.3

# Manipulation Update

- add runtime dot manipulation interface
- remove manual tomfoolery

- consider line v circle segment algorithm for collision

- add water and liquid behavior
  rgb(0, 253, 255)?

- add arguments for
	- gfxScale
	- gravity
	- groundFriction
	- tickrate
	- timescale
	- worldWidth
	- worldHeight
	- `-h --help`

- remove debug color manipulation (has a TODO mark)

- set version to 0.2

# Base

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


-----
