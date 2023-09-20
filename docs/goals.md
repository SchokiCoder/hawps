# Explanation: Limitations and goals 

There are limitations, primary goals and secondary goals.  

Limitations describe what is __not allowed__ in the project/program.
Examples for that could be:  

- max. 3000 sloc, C99, Linux-style
- libc only allowed dependency
- no incompatibility with FreeBSD, OpenBSD or Linux

All __primary goals__ are to be implemented __before the final release__, unless
the implementation of a single goal proves to be too difficult or would come
with a too high cost such as a high increase of the sloc count.
The update implementing the last primary goal, becomes the final release update,
which in semantic versioning would be 1.0.0.  
Optimally the primary goals are documented as a roadmap.  

All __secondary goals__ are to be assigned __after the final release__.  
Optimally the secondary goals are documented as a roadmap,
starting where the primary goal-roadmap ended.  

# Limitations

- try Zig, only Zig, just Zig, raw Zig, no Makefile, no C, shell for install
  allowed
- data-oriented programming
- configuration via Zig source code file
- max. 5000 sloc in Zig (not including config or build files)
- max. 500 sloc in Zig for build files
- must be compatible with Linux, FreeBSD and Android
- fun is more important (in programming and in use)

# Roadmap (done)

# Roadmap (planned)

## v0.1.0 basic features

Only a mock-up sort of proof of concept thing for now.  
No user interaction yet, aside from window close of course.  

- open window, window close event
- draw pixels (take first that succeeds: try OpenGL ES, then Open GL, then SDL)
- add material "Sand"
- gravity and basic collision
- no more brain-dead stack of materials upon fallig on top of each other
  (see following snippet, where O represents a dot of sand)

```
   O
   O       ---\
   O       ---/    O
   O              OOO

brain-dead ---> nice pile
```

## v0.2.0 liquids

- add material "Water"
- make the water behave like water (and remember data-oriented programming, so
  that once this runs on android, my battery doesn't implode)
- make sure sand displaces water

## 0.3.0 gases

- add material "Oxygen"
- add material "Hydrogen"
- make sure that the lighter gas gets displaced by the heavier one
- add chemical reaction (oxygen and hydrogen collide)

## v0.4.0 user interaction

- create hud mock-ups
- hud for selecting tools
- brush for setting materials at a spot to selected
- add key binds
- add "hold ALT key" to display on hud what uses which key bind

## v0.5.0 temperatures and aggregate states

- add temperature value
- add tool to manipulate temperature of target mass
- add aggregate states to materials
- add a thermo-vision switch

## v0.6.0 fire

- what is fire exactly? but don't make it too serios ;)
- it consumes oxygen

## v0.7.0 a bunch of fun materials

Add materials:

- Dirt
- Iron
- Ironoxide
- Aluminum
- Aluminumoxide
- Glass
- some Rocks lol
- Coal
- Sulfur
- Gunpowder
- Thermite

Add chemical reactions:

- Sulfur and Coal collision, creating Gunpowder
- Oxygen and Iron collision, creating Ironoxide
- Oxygen and Aluminum collision, creating Aluminumoxide
- Ironoxide and Aluminumoxide collision, creating Thermite

## v.1.0.0 polish

- make sure it runs on Linux, FreeBSD and Android
- make sure all other limitations are respected

## v1.1.0 beatiful specks of dirt

So far we only use flat colors for all the materials.  
A pile of the same material looks pretty boring.  
  
Use a shader?  
