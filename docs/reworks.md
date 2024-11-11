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
