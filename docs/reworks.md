# 5th rework

Still Go, don't worry.  
However... ebitengine, I am afraid, needs to go.  
While it worked mostly just fine, UI is kind of a concern,
and that is currently very... barebones.  
Some might say, "Just grab ebitenui!"
There are some issues with it. Firstly it is currently v0.7.0,
aka not production ready.  
Secondly it seems to be an unofficial thing,
which is not an actual part of the engine.  
I got a bit wary of 3rd party dependencies.  
So I will be playing with actual UI libs.
This might cause very real, serious issues for Android compatibility though.
This may lead to the omission of that goal,
because I will not maintain multiple different clients. No sir.  
Ebiten, so long, partner.  

## Go goes too.

Since GTK, the programming is mostly just simulation math,
and gluing that to a client. Anything CGO has performance hits,
and it's simply not worth that.
Drawing the world will need a new dependency.
Using C, I can cut gogtk and whatever will be needed for that.
You were good, son, real good, maybe even the best.

# 4th rework

Ah shit here we go again...  
Golang.  

C is fun, except when it's not.  
That is what I think of networking and UI.  

Networking has been removed by now. One of the reasons being security, but also
development speed. I looked at a Go example, and it's so easy, but still,
only after everything else has been done.  

UI work gets increasingly unfun, and now that "UIBox" needs a dict for tiles
I saw the final reason to switch back... kind of.  

I can use both actually, thanks to CGo.  
This will help avoid allocations later, keep GC low, and performance high.  
Plus, I can finally `fmt.Printf` any data type easily again if I need to...  
I should finish that book and learn Delve huh...  

> Dipshittery is dead
Naive me  

...One more thing...  
I'll try ebitengine.

# 3rd rework

So I did it in Go, but now I redid it in C anyway.  
I have fun with C. Can you blame me?  

Before that, I should also mention the rejection of float-based physics.  
No more velocity, friction, or hitboxes at all.  
Now a dot is just a single blip in a matrix of data.  

Also, I added networking, which caused a bunch of splits,
but I am very pleased with how it turned out.  
The server doesn't need SDL to be compiled,
can be statically compiled, and can be copied to another system in the LAN,
to be tested.  
I put it on my Debian 12 server and it just worked.  

# 2nd rework

Godot or any other game engine seems to not really be a good fit.
I wanted this initially for the collision physics, but it turned out that this
is far too complex in the first place.
Reject polygons, return to pixels...
and with that to just programming languages.
This should be more fun anyways.
Now, which langauge?

Rust: Meh, too annyoing.
Zig:  No tabs... dumb.
C:    Memory and pointer fun, but janky.
Go:   Maybe too easy.
Odin: Maybe, but didn't compile last time. Plus need to learn.

Originally RNG told me to do C:

```C
// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

#define WORLD_W  80
#define WORLD_H  60
#define MAX_DOTS (WORLD_W * WORLD_H)

enum DotMaterial {
	D_NONE,
	D_SAND
};

typedef int DotId;

typedef struct _wld {
	DotId n[WORLD_W][WORLD_H];
} World;

typedef struct _dmat {
	enum DotMaterial n[MAX_DOTS];
} DotsMaterial;

DotsMaterial new_DotsMaterial()
{
	int          i;
	DotsMaterial ret;

	for (i = 0; i < MAX_DOTS; i++) {
		ret.n[i] = D_NONE;
	}

	return ret;
}

World new_World()
{
	World ret;
	int   x, y;

	for (x = 0; x < WORLD_W; x++) {
		for (y = 0; y < WORLD_H; y++) {
			ret.n[x][y] = -1;
		}
	}

	return ret;
}

void manually_spawn_some_sand(DotsMaterial *d_mat, World *world)
{
	d_mat->n[0] = D_SAND;
	world->n[2][2] = 0;
}

int main(int argc, char *argv[])
{	
	DotsMaterial d_mat;
	World        world;

	d_mat = new_DotsMaterial();
	world = new_World();

	manually_spawn_some_sand(&d_mat, &world);
}

```

But... eh.

# 1st rework

## Why a rework (to another one of my repos)

"physics_box" originally started out as a simple "physics" sandbox game but at
some point i started thinking something like "What if i took this whole thing
way too serious?".  
A "half-assed wannabee physics simulation", that makes physicists and chemists
want to strangle me.  
That is my honest evaluation of the project after i left it rot for more than a
year...  
And that's how "hawps" is born.  

## Now

Now, "hawps" is the ironic name of the show instead of the painful truth.  
So strap yourselves in.  
We got a bumpy new roadmap ahead of us.  

## Kicking the dead horse

I don't know why i started to think that it would be a good idea to try a
serious simulation of physics but i am sure it killed all my fun back then.  
This pile of +#!% also suffered from my good old habit of jumping into a new
thing without ever thinking it through.  
Dipshittery is dead, long live [docs/goals.md][goals.md].  

[goals.md]: <https://github.com/SchokiCoder/hawps/blob/main/docs/goals.md>
