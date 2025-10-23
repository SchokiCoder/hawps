- [ ] add shader that uses the temperature glow to emit light around the glowing object

- [ ] test android build
`sudo apt install google-android-ndk-r26d-installer or newer version`

- [ ] add "hold ALT key" to display what UI element uses which key bind

- [ ] unify approach to flag error prints
Some are panics, despite a graceful shutdown being implemented

- [ ] find out how to make ebiten not use keyboard scancodes
- [ ] add non-numpad plus/minus to tickrate keybinds

- [ ] consider oxygen/air to be always there (not None) ?
But only for chemical reactions,
because otherwise displacement and thermal conduction would have to permanently run.
Add a roomtemperature?

# Material Me Update

- [x] add some missing phonies to Makefile
- [x] unify critical errors into panics

- [x] move glow colors to a new extra package
- [x] rename mat to core
- [x] rename main to cross_platform and move ui into there

- [x] replace manual values with generation for glow color in-betweens

- [x] add mock up of desktop UI
This is a glade project file, which is for GTK3,
but if a switch to a different toolkit is desired,
this should still be used as reference then.

- [x] add desktop client base
- [x] add proper window close

- [x] rewrite desktop in C
Why C again? See docs/reworks.md.

- [x] rewrite core and extra in C
Why C again? See docs/reworks.md.

- [x] change signals being connected using glade handlers, not manually

- [x] add basic world draw to desktop client
This also undid glade handlers.
`gtk_builder_connect_signals` doesn't pass custom data, which I need for draw,
AND overrides all custom connections, which I did for draw.
Therefore it needs to go again.

- [x] add world_free and use it

- [x] add world simulation loop
- [x] add glowcolor draw to desktop
- [x] add brush hover draw

- [x] add content of tool- and materiallist
- [x] add materiallist entries update, based on selected tool
- [x] add materiallist entries update for spawner, based on temperature

- [x] change simulation ui
Change play, pause, and thermoview buttons into switches.
Change speed up/down buttons into a single slider.
This simplifies programming and use.

- [x] add default values being loaded into the widgets
- [x] add tool radius change upon scroll
- [x] add tool zone draw snapping to tiles
Also fix the tool hover size.

- [x] hide mousecursor when hovering on worldbox
This also causes toolhover to only draw when actually hovering on the worldbox.
This relies on the deprecated Gdk function `gdk_cursor_new`.
I heard GTK just eventually breaks deprecated stuff when convenient (for them),
but hopefully this doesn't apply to Gdk too.
- [x] fix type warning about tick return value

- [x] add mouse click handling for worldbox
- [ ] refine all that shit yo
- [ ] hook up buttons
- [ ] use statusbar
- [ ] add world scale slider
- [ ] hook up menubar
- [ ] save selected mat of each tool

- [ ] add per spawner temperature
- [ ] add temperature setting for new spawners

- [ ] update README build dependencies

- [ ] fix temperature glow (see black body radiation)
The visible result maybe different due to the colors mixing,
becoming a white eventually
- [ ] increase the resolution of available glow colors ?

- [ ] bring back the app args

- [ ] add core test against gravity
- [ ] add core test against grain stack collapse
- [ ] add core test against liquid stack collapse
- [ ] add core test against gas stack collapse
- [ ] add core test against thermal conduction
- [ ] add core test against chemical reactions
- [ ] add core test against loss of mass upon heat up
- [ ] add core test against spawners

- [ ] add Dirt
- [ ] add Ironoxide
- [ ] add Aluminum
- [ ] add Aluminumoxide
- [ ] add some Rocks lol
- [ ] add Coal
- [ ] add Sulfur
- [ ] add Gunpowder
- [ ] add Thermite

- [ ] add Sulfur and Coal collision, creating Gunpowder
- [ ] add Oxygen and Iron collision, creating Ironoxide
- [ ] add Oxygen and Aluminum collision, creating Aluminumoxide
- [ ] add Ironoxide and Aluminumoxide collision, creating Thermite

- [ ] set version to 0.7

-----

# Pyro Update

- replace current arg handling with flag std lib package ?
No. Makes having shortcuts very weird. Doesn't decrease complexity. Too stiff.  

- [x] add scrollwheel to scroll ui.TileSet
- [x] add scrollwheel to increase/decrease tool radius

- [x] decouple world updates from general app updates
I want the ui to have at least 120 updates. This way brushe usages are nicer.
This also comes with a speed up of the default world update rate from 24 to 30.
This is done to keep the world update a fraction with the factor of 2.
Otherwise things aren't properly timed.

- [x] split World.Tick, into one for essential updates, one for simulation
Also put the pause sys back into main.
This also resolves some minor visual lag on low tickrates,
since World's color updates now match the ui updates.

- [x] add further mat weight loss for gases being heated up
Huzzah! Another fictional formula.
This one can even cause gases with negative weight,
if the temperature reaches the 5000C region.
It doesn't cause anti-gravity. So clearly it's fine.

- [x] add experimental fire
Also fix an issue with one-way chemical reactions.
Also add methane and carbon dioxide.
Fire still lacks visuals and reliability.

- [x] flatten conduction values
This mostly affects what is a gas under room temperature and pressure.
This brings them into the
"I can see heat exchange happen before I need to go to bed" realm,
and brings a much more realistic looking burning speed and consistency.

- [x] add glow via temperature system
Replace solid, liquid, and gas RGBs with a generic glow by temperature system
Draper point = 798 K = temperature at which things start to glow
Wiens displacement law = 2898000 / Kelvin = function to determine wavelength in nm
violet 380 - 440 nm | 7626.32 - 6586.36 K
blue   440 - 485 nm | 6586.36 - 5975.26 K
cyan   485 - 510 nm | 5975.26 - 5682.35 K
green  510 - 565 nm | 5682.35 - 5129.20 K
yellow 565 - 590 nm | 5129.20 - 4911.86 K
orange 590 - 625 nm | 4911.86 - 4636.80 K
red    625 - 740 nm | 4636.80 - 3916.22 K
Instead of doing expensive HSV stuff for temperature based glowing,
we prepare an array of glow colors,
which we then iterate through based on dot temperature.

- [x] fix glow far below draper point
Technically it's just working around ebitengine,
blending colors that have 0 alpha.

- [x] use Kelvin instead of Celsius in mat math
- [x] add check to cool brush against negative values
This replaces a check against negative glow colors.

- [x] update copyright

- [x] seperate tool hover drawing into own layer
Fixes tool hover zone being drawn over by glow.
Also adding transparency to tool hover draw.

- [x] fix world simulation zone size

- [x] optimize the row direction switch of world simulation

- [x] fix chemical reaction overreach

- [x] fix negative temperature given via flag
This also changes the interpreted temperature unit to Kelvin.

- [x] replace mat.None drawing with an actual background

- [x] decrease max alpha of glow
This greatly helps with visual distinction.

- [x] adjust blend factor for world image layers
To demonstrate and increase visuals the colors have been adjusted here.
Most gases had been a hard coded dark blue so far, assuming a black background.
Now that the background could change so should the gases look.

- [x] add dot alpha loss for aggregate state change
Plus adjusting dot colors once more.

- [x] fix some wrong melt- and boilpoints

- [x] fix panic when scrolling down on an empty ui.TileSet

- [x] generalize mat's oxidation reaction code
This allows for oxidation to be a declarative property.
This also removes Hydrogen's incorrect oxidation for now.

- CANCELLED: fix stack collapse bleeding through diagonal surfaces
Changing the stack collapse to move sideways then fall,
with more ifs solve the problem, but decrease performance.
The fix also differs quite a bit per aggregate state. This is annoying.
Just canonically accept diagonal surfaces to be open.

- [x] add proper hydrogen oxidation

- CANCELLED: use actual values for oxidation speed
Couldn't find data.

- [x] update demo.gif and description in README

- [x] set version to 0.6

# Thermo Update

- [x] make liquid and gas stack collapsing not ignore grains anymore
- [x] fix phys updates going from left to right instead of bottom to top
This showed most obviously in grain stack collapsing,
thus teleporting grains to the bottom right of the stack.

- [x] add + and - keybinds to change tickrate

- [x] add temperature to live dots

- [x] fix grain stack collapsing bias
It was following the horizontal "applyGravity" loop, which it still does,
but now the direction of which gets shuffled.

- [x] add aggregate state variation to live dots

- [x] add basic thermal conduction

- [x] fix improper dot clear on chemical reactions and erasing

- [x] add thermal conductivity values to material properties and calculation
- [x] add thermal vision

- [x] add world bg sys and specific thermal vision bg

- [x] fix non-gravity on every 2nd row of left border

- [x] add preview for tool hoverzone

- [x] fix pause skipping dot color updates

- [x] fix clicks on world's first col and row not doing anything

- [x] fix pause not effecting thermal conduction

- [x] add None to spawner mats
This also lead to the removal of the MsNone state,
which since dynamic temperatures got too error prone anyway.

- [x] add weight loss for warmer materials, upon state change
This comes with generalized displacement checks.

- [x] unify dot updates into world.Tick
This causes a reduction of individual loops, thus reducing None checks,
theoretically improving performance.

- [x] add tool to manipulate temperature of dots

- [x] add arg "-temperature"

- [x] add tool tile bg

- [x] set version to 0.5
Also add a new demo image.

# Solid Update

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

- [x] add click handling to TileSet
- [x] make game data mutable to its Update function
- [x] fix TileSet cursor drawing
- [x] fix TileSet click not applying an offset for Tile determination
- [x] fix TileSet click going past visible tiles
- [x] add a world instance back into the whole thing
- [x] add brush tool function
- [x] add eraser tool function
- [x] change mouse input to be instant and perpetual
Also increase eraser tool radius.

- [x] add Matbox filter (depending on current tool)
- [x] improve handling of visible tiles and mats
- [x] add spawner tool function

- [x] add arrow keys to control tool and mat switching

- [x] add arg "-wideui" and "-tallui" and update help
Also add missing help text for arrow keys.

- [x] fix args "-v" "-version" not cancelling further execution

- [x] set version to 0.4

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
