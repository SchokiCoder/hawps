- [ ] ebiten client: hook up to c libs

- [ ] ebiten client: scroll TileSet when cursor goes below or above visible
- [ ] ebiten client: change TileSet to use mouse on release
This fixes accidentally changing mat or tool

- [ ] enable grains to absorb water, or add a water can wash away clay function?
Clay might be boring. Do something with it.

- [ ] add "Hammer" tool, which changes statics into grains?
- [ ] increase the resolution of available glow colors ?

- [ ] add per spawner temperature
- [ ] add temperature setting for new spawners

- [ ] test android build
`sudo apt install google-android-ndk-r26d-installer or newer version`

- [ ] add "hold ALT key" to display what UI element uses which key bind

- [ ] find out how to make ebiten not use keyboard scancodes
- [ ] add non-numpad plus/minus to tickrate keybinds

- [ ] consider oxygen/air to be always there (not None) ?
But only for chemical reactions,
because otherwise displacement and thermal conduction would have to permanently run.
Add a roomtemperature?

# Beauty Update

- [ ] fix temperature glow (see black body radiation)
The visible result is different due to the colors mixing,
becoming a white eventually. Otherwise our sun would be green. It isn't...
which is kinda sad.

- [ ] change conduction so that the conductivity describes the rate at which a mat **loses** temperature?
A metal rod (high cond.) can ignite a gas (low cond.) quickly this way.
Thus redo all the conductivity values to remove the artifical flattening?

- [ ] load image files instead of embedding them
- [ ] move mat property table to json or csv and provide an init function for package mat

- [ ] add make rule for optimized release binary
- [ ] ensure packaging and installation aligns with flatpaks
- [ ] add shader that uses the temperature glow to emit light around the glowing object

- [ ] unify approach to flag error prints
Some are panics, despite a graceful shutdown being implemented

# Web Update

- [ ] libcore: add saving world to file functionality
Serialization stuff. Mind endianness.

- [ ] libcore: add loading world from file functionality

- [ ] terminal client: add cmd for saving world to file
- [ ] terminal client: add flag for loading world from file
Remember to update the world name display to reflect this.

- [ ] terminal client: add auto-saving world to file and flag to disable it
Saves to "default" and by default loads it at startup?

- [ ] terminal client: add cmd for loading world from file

- [ ] terminal client: add networking
How? Look at ca22d7451c87b6d090e35328ae6994459117e86c,
the commit before networking got removed.
This should be able to piggyback off the saving/loading serialization.
Remember to update the ip address display to reflect this.

- [ ] terminal client: add flag to enable hosting a server
- [ ] terminal client: add flag to enable hosting a dedicated server
- [ ] terminal client: add flag for connecting to an ip address
- [ ] terminal client: add cmd for connecting to an ip address

- [ ] terminal client: update help text
- [ ] update README

- [ ] set version to 0.8

# "Waiting for an update"-Anniversary Update

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
- [x] fix retrieval of selected mat
- [x] add thermotool rate
This fixes them working as fast as your CPU allows.
- [x] add spawner drawing
- [x] add colorchange upon aggregate state change

- [x] add proper worldloop thread termination
Also fix mistimed thread creation.
This fixes many Gdk-Critical messages happening at start,
regarding an invalid window being used.

- [x] add callback for thermoview switch and thermal vision
- [x] add callback for pause switch
Also minor var name fix: "win -> "mainwin".
- [x] add callback for sim speed slider
- [x] decouple worldbox update from sim speed
This fixes the laggy appearance with low simspeed.
Oddly enough this simple change had no performance impact.
Also added an important chapter in docs/reworks.md.

- [x] revive ebiten client and port libs to Go
There are also some architectural differences.
mat is now a submodule of core,
to bring back the proper namespace use,
which is nicer to read.
The ebiten client also got renamed,
and changed to fit the new libs.
Also fix an old Makefile mistake from prior renaming attempt,
that messed with ldflags.

- [x] port over tick-sim-subsample system from tk to ebiten
With this I also reassessed what should be the default tick- and simrate.
I thought about getting back to the old 24 simrate,
but there are problems with that.
I wanted a user to easily increase and decrease the rate with 2 keys,
with which the simrate is doubled and halved.
24 as a base is very odd.
The tickrate would have to be (simrate * (factor of 2)).
The tickrate should be high for comfortable UI use.
That leaves a default tickrate of 192, which feels a bit high,
or 96, which is just too low.
I tested the performance of 120-30 vs 192-24, tickrate-simrate,
and the latter also used more CPU.
More updates, less simulation was heavier in that case.
I suppose base 30 aligns better with most screens update rate anyway.
In another thing, the cap for slow down also doesn't perfectly align with
full numbers. This is fine. Don't "fix" it.

- [x] add saving of selected mat of each tool

- [x] add tiled png-font and replace hard drawn text of mat symbols

- [x] add Coal and a oxidation product chance system
- [x] add Ironoxide, melt product system, and applied it to Sand/Glass too
- [x] add Aluminum and change Iron a bit
Iron 's thermal conductivity had a minor mistake,
and Iron's color got touched up to make it more distinct from Aluminum.
- [x] add Aluminumoxide
Also bring back the possibility for oxidation to have fixed products.
This has been applied to all oxidations except Coal.
- [x] add Thermite and touch reaction system
- [x] add Magnesium and Magnesiumoxide
- [x] add Sulfur, Sulfur trioxide, and Gunpowder
- [x] add Sulfuric acid and acidity system
- [x] ~~add Dirt~~
- [x] add Clay and Ceramic
Dirt is too impure and would be wonky to implement,
which is rich given that clay is implemented even.
Dirt is just even worse than clay.
Ceramic probably wouldn't melt and then eventually go back to being solid
ceramic. It apparently gets destroyed by heat. It seems to be mainly silicon,
therefore I just decided it becomes glass. Currently there is no
"random melting product" system,
and I don't feel adding it for just one edge case.

- [x] add Limestone and Cement

- [x] add world/dot scaling
Default scaling is 4.
- [x] add overall scaling
This removes the old magic numbers for scaling with flags and proper defaults.
- [x] add better labels for materials
This needed multiple things.
Changing the win- and worldscale, adding a scale for pngs (constant),
updating the TileSets in use, and updating the mat table.
Sadly "Al2Si2O5" and "Al2Si2O7" still don't fully fit,
but at least now one can see how things may transform.
Seeing "CaCO3" and then "CaO", obviously a "CO2" is missing,
which is released when burning it.
This may given an "aha" moment to the user.
Also thermite is not one molecule.
It's a mix and should technically be labelled "2 Al+Fe2O3",
which is obviously still too long as well.

- [x] create basic terminal client
What? Why?
I used to be a frontend dev like you, but then I took a GTK to the knee.
GUI has been the bane of my existence for the past year.
Qt is C++ = no.
Tk is sadly unfit due to performance issues and an unclear future,
regarding X11's impending death.
Hacking together widgets, whether in my own lib (C/Tk) or in Ebiten,
is just stinky.
The idea of TUI for hawps I mocked up turned out better than I expected,
and it even took less time than coming up with my GUI design.
Best of all, new features like save-load, networking, etc. will be easy...
Just add a flag to arg-parsing, done.
Now losing mouse input isn't trivial, I know, but so is losing my sanity.
"Proper" GUI was supposed to be a quick slam dunk and win for desktop users,
but it became a full on main quest with C/Tk.
While fun (most of the time) I lost 5 months for hawps.
No commits, no work.

- [x] terminal client: port to C
HUH?!
Yeah, let me squeeze that in real quick. Why?
Ebiten is just one client, next to Tk and potentially someday C/Tk (who knows?)
and it's only Ebiten that is written in Go.
The core and extra lib of hawps already have C versions that need to be dug out
and some features need to be backported.
Now if I'll ever use Ebiten client again, I'll just bind the C libs with CGo.

- [x] bring back C versions of core and extra and tie to terminal client
- [x] terminal client: add tick-sim-subsample system

- [x] backport to C libs: all new Materials and the property table
This is easier than doing it on a per commit basis.

- [x] backport to C libs: melt product system
From 723183c680ff94c65aa3a778d8a5b02bbc06ed83
and  325651e1b387cd7f3b3864bfc35039bbceee29ad

- [x] backport to C libs: oxidation product system
From a2a4e514a428a8bf8ca31a2611be57b1a90e2295

- [x] backport to C libs: touch reaction system
From 4c81602e3d641fcb7ba4b2873e33d997ac562867

- [x] backport to C libs: acidity system
From 79102fb2d936f8fa98411a8be4c79f73d8f54d17
Also update copyright.

- [x] remove plural from world array names
This has already been the case in the Go libs.

- [x] replace truth ints with stdbool
- [x] core: fix formatting and alphabetical ordering of functions
- [x] fix `CSI_CLEAR` leaving empty lines behind

- [x] terminal client: add input and mainloop
- [x] terminal client: fix input blocking

- [x] terminal client: fix not updating the world
Also add minor chapter to `docs/performance.md`.

- [x] terminal client: buf drawing output
This fixes a flickery appearance.

- [x] terminal client: add bind for vision switch

- [x] terminal client: performance: replace most sprintf calls

- [x] terminal client: fix last row not being cleared properly
- [x] terminal client: fix flag parsing
- [x] terminal client: fix flag int arg parsing

- [x] ~terminal client: consider removing tickrate (for a as fast as possible loop)~
No, the tickrate is need for device battery life.
I considered setting tickrate to 60 and sim-subsample to 2 to compensate,
so that on first glance nothing changes,
but this didn't really lower the power usage.
Also `cfsetispeed` seems to not affect keyboard input speed at all.

- [x] terminal client: add mouse input support
POV ebiten client:
Anything you can do, I can do better!
I can do anything better than you!

- [x] terminal client: add tool radius change via mouse wheel
Also buff thermo tools to roughly match ebiten thermo rate,
since input isn't as fast as the tickrate itself.

- [x] terminal client: fix SIGINT and SIGTSTP handling
- [x] terminal client: add bg coloration of thermoview
Also fix draw display having no null termination.
This can make the display fuller than intended,
and then output would be shifted upward permanently.

- [x] terminal client: add tool use when mouse button held down
- [x] terminal client: add spawner drawing
- [x] terminal client: improve color csi gen function robustness
- [x] terminal client: fix mouse release not being caught during lag
- [x] terminal client: fix laggy input
This decouples input from the tickrate.

- [x] terminal client: add dynamically sized display string
This HAS to be done.
Colored dots are quickly using up anything the stack can provide at all.

- [x] terminal client: add spawner color
Actually this was rather a giant rabbit hole of debugging.
Just look at the commit and what kind of stuff I wrote.

- [x] add "lib" prefix to core and extra and move binaries to bin
No, this doesn't give these libs proper namespaces, because I don't want to.

- [x] terminal client: add thermoview

- [x] ~replace world size `int` vars with `size_t`~
No, this can't be done because it adds `if`s.

- [x] move `struct Rgba` from libcore to libextra
- [x] tk client: update to current lib and update Makefile
- [x] terminal client: use libs' `enum Tool` and `struct Rgba`
- [x] terminal client: use `size_t` only for memory things anymore

- [x] fix: re-enable most melt conversions

- [x] terminal client: add dot color
This removes an entire alpha blend step for each dot in terminal client,
since we use a different char for aggregate states, not alpha loss.
Graphic clients such as ebiten can just assume full alpha.
Why haven't the RGB values changed to compensate?
I wanted to do that,
and by figuring out how the colors finally look in the ebiten client,
I found out that the color blending there never really worked as intended,
so the assumptions of the values haven't been met anyway.

- [x] terminal client: add dots with glow color to draw

- [x] terminal client: fixmouse cursor reporting
The terminal report is offset, starting at 1, not 0.
This also fixes a crash when the mouse moves o lower right corner.

- [x] terminal client: replace cursor draw with actual tool hover

- [x] ~terminal client: add tool hover color~
Not too sure about it anymore.
In the times of single point cursor, it was hard to see,
but now it's an actual zone, and visibility is not too bad.
Plus, the tool hover draw doesn't change the underlying colors,
so it is kind of transparent in a way.

- [x] terminal client: add binds for tool radius change
- [x] terminal client: fix radius change on spawner crash
- [x] terminal client: fix tool hover draw of spawner

- [x] terminal client: add arrow keys as alias for tool navigation
Also clean up csi.h quite a bit.

- [x] terminal client: replace own tool names with libextra's for draw
- [x] terminal client: add pgup, pgdn, home, end keys for tool navigation
- [x] terminal client: add uppercase vim nav keys for extreme tool navigation

- [x] terminal client: fix input buffer not getting null terminated
`read` doesn't care, so I have to.
This fixes certain nav methods disabling other nav methods
For example, mouse nav disabled everything else, except og vim nav.

- [x] libcore: change oxidation base condition from released heat to speed
I didn't even notice that. You may think I did,
because Iron and Alu oxidation have a small value given,
but that was just an easteregg, I swear.

- [x] terminal client: fix input buffer overflow crash
Thought it would stay theoretical but it didn't.
The `input_len` can practically be as long as the size.
Now it cannot fall onto my feet anymore.

- [x] terminal client: add binds for simspeed change
- [x] terminal client: add plus and minus keys as alias for simspeed change
...Remembering this as struggle from Ebiten... that was easy.

- [x] terminal client: add basic cmdline
- [x] terminal client: add mat switch binds
- [x] terminal client: fix missing comma in mat table, behind "Iron Oxide"
This caused a crash when switching to last mat, and name mismatches.

- [x] terminal client: add small snippet about static config in help text
This comes with a dev reminder in `config.h`.
If it is removed, compilation fails.

- [x] terminal client: add cmdline simple functionality and feedback
- [x] terminal client: add cmdline backspace
- [x] terminal client: add many simple cmds
Also make libcore's `world_clear_dot` public.

- [x] terminal client: add simspeed display to status bar
This bindings for runtime help texts from st bar display,
because the feature will also be scrapped.
Why bother when `-h` exists?

- [x] change license to MPL-2.0  and update copyright

- [x] terminal client: add a ':' to cmdline display
- [x] terminal client: fix cmdline feedback generating too many clear spaces
- [x] terminal client: fix old cmdline feedback persisting through new command
- [x] terminal client: add cursor during cmdline display
This is not using the "real" cursor,
because restoring that one is a non-trivial amount of work.

- [x] update license url in Makefile
- [x] add "clearall" command to also remove spawners
- [x] make cmdline ':' and '<' configurable
- [x] make status bar separator configurable
- [x] add more extreme / uppercase keybind versions

- [x] libcore: add Quicklime and Slaked Lime
This is much easier than actual Cement.

- [x] libcore: add random chance for alternative touch reaction product 2
This is so that the Slaked Lime + Carbon Dioxide -> Limestone reaction
doesn't always consume the Carbon Dioxide.
The creation of Quicklime also doesn't always create Carbon Dioxide.
The chance for both CO2 creation and consumption are now 5%.

- [x] ~libcore: remove chemical sim touch's parenting `if`~
It's NOT unnecessary. Without it, things react to `MAT_NONE`.

- [x] terminal client: add cmd for changing brush material
- [x] terminal client: add cmd for changing spawner material
- [x] terminal client: add toolswitch upon matswitch via command
- [x] terminal client: add cmd for changing material of current tool

- [x] terminal client: add cmd for setting the temperature of new dots
- [x] terminal client: rename temperature flag and variable to spawntemperature
- [x] terminal client: add app flags as constants / defines
- [x] terminal client: add cmd for setting the temperature of all current dots
- [x] terminal client: add cmd for setting the tickrate
- [x] terminal client: add cmd for setting the thermodelta

- [x] terminal client: add flags for various settings
- [x] terminal client: change short for thermo commands to contain "th"
This is to avoid incoming name collisions.

- [x] terminal client: add commands for tool radii
- [x] terminal client: add float parsing for various flags
- [x] terminal client: add newline char at end of error messages

- [x] move color blending from terminal client to libextra

- [x] terminal client: add flag to disable glowcolor
- [x] terminal client: add flag to disable all csi coloring

- [x] terminal client: add struct for tool options
- [x] terminal client: microoptimize an if

- [x] remove obsolete go workspace files

- [x] terminal client: add new flags to help text

- [x] terminal client: add cmds to set glowcolor

- [ ] terminal client: add commands to help text
- [ ] terminal client: update keybinds in help text

- [ ] terminal client: add a Tool overview to help text
- [ ] terminal client: add a Mat overview to help text

- [ ] melt decomposition is skipped when spawn temperature is meddled with
- [ ] fix odd displacement of grain when they fall into a warmer gas or liquid
- [ ] terminal client: how to handle extremely small terminals? are there crashes?
- [ ] terminal client: performance: add lookup table for int to str conversion
(removing last sprintf calls, and don't forget reaplacing scanf with strtol)
- [ ] performance review of both clients

- [ ] add proper profiling procedure to Makefile with `gcc -pg`
- [ ] performance: function pointer vs if; test
Color options could benefit from this.

- [ ] libcore: add test against gravity
- [ ] libcore: add test against grain stack collapse
- [ ] libcore: add test against liquid stack collapse
- [ ] libcore: add test against gas stack collapse
- [ ] libcore: add test against thermal conduction
- [ ] libcore: add test against chemical reactions
- [ ] libcore: add test against loss of mass upon heat up
- [ ] libcore: add test against spawners
- [ ] libcore: add test against mat table property array length miscount

- [ ] update README
- [ ] update copyright?

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
