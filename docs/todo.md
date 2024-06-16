- add water

# Manipulation

- add runtime dot manipulation interface
- remove manual tomfoolery

# Niceties

- add arguments for
	- gfxScale
	- tickrate
	- `-h --help`

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
- add ground friction
- add dot collision
- check if gravity calculation makes sense

- set version to 0.1.


-----
