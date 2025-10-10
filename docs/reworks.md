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

## Unlike GTK (Moving to tkinter now)

> Additionally, there are known bugs in GTK3
> where the delta_x and delta_y fields in the GdkEventScroll structure
> are consistently zero,
> rendering them unusable for determining scroll direction.
> This issue has persisted even in newer GTK versions, including GTK4,
> making it impossible to reliably detect scroll events using these fields.
> As a workaround, developers have been advised to use
> the GDK_SCROLL_UP and GDK_SCROLL_DOWN event types
> instead of relying on the delta values

Leo (Brave Search AI)  

BUT WAIT, THERE IS MORE!
Apparently <https://docs.gtk.org/gtk3/signal.Widget.scroll-event.html>
lists the event parameter as type `GdkEventScroll`.
This is putrid trash, as it is actually a pointer of that.
If you try to follow the documentation, **you're fucked**.
PLUS none of the deltas will work,
if you haven't added `GDK_SMOOTH_SCROLL_MASK`,
while the docs say that you just need `GDK_SCROLL_MASK`.

Accessing ComboBox in scroll event randomly did not work.  
Thats why AppData.cur_tool got added actually.  

I am filled with tremendous amounts of piss...
Now I will try Python and tkinter.
Compared to C/GTK there is **no loss of type safety**,
because GTK, impressively, fucked that up too.
In order to be OOP, it's just a huge mess of pointers,
which you have to cast to other types of pointers.
This practice is also known as "deleting all type safety".
"But muh inheritance nyeeeeh!"
If you want to eat crayons- I mean have inheritance, just use C++.

I considered Go and Fyne, but mnemonics seem to not exist there,
and it had some performance issues.
By readjusting a slider, the thread would 100% and lag out.
That on a rather good CPU is no good sign.
It would eat a multitude of RAM compared to tkinter,
which is also true for GTK 3.

With Tkinter I will have to do more homework,
but I prefer that over bugs that never get fixed,
weird ass, forced OOP shoehorning,
and simply just GTK's general abusiveness with developers.
I was already wary of that fact before, but figured that since GTK 3 is still
"maintained" (on paper as it turns out),
I could just wait for GTK 3 to be dropped by GNOME,
and then freeload off the inevitable fork of GTK 3, right?
Cinnamon, XFCE, MATE, etc will surely not switch GTK 4... right?
The future of these desktops is foggy, and I can't depend on fortune.

Also Glade seemed neat...  
GNOME doesn't even pretend that it's maintained.
This would be fine, if it didn't crash every two seconds.

Hawps can be added to the list of
<https://en.wikipedia.org/wiki/GTK#Criticism>.

## No to python and tkinter, just cut out the middle-man (raw TCL)

Python and tkinter have some weird stuff.  
I `app.bind_all("<Alt-f>", lambda e: mnemonic_f(filemenu))`,
which just `filemenu.post()`.  
In English, upon hitting Alt and F, open the filemenu of the menubar.  
This doesn't work:
`TypeError: Menu.post() missing 2 required positional arguments: 'x' and 'y'`
Ok, so there is some data related error. I didn't pass something correctly?  
No. I merely bound it to `<Alt-f>` instead of `<Alt-F>`.  
The event **does fire**, but it just doesn't work? Why?  
Technically this just differs in that now I would need to press Alt + Shift + F,
but for some reason Shift is not needed.  
It seems as if Alt uppercases all keypresses in this stack!  
`<Alt-f>` works with Alt + Shift + F while, `<Alt-F>` works with Alt + F.  
I could accept if the event merely would not reach the callback, but it does,
just without the needed data.  

Since I left GTK to avoid such jank, the decision is consistent.  
Thing is, in TCL this does not happen at all,
**and** I don't even need to manually bind the mnemonics for the menubar :)  

There is also a minor nuisance with command as a param vs. binding,
where one can use the lambda and the other not.  

One more thing.  
Why is everything Python having such a hard time with documentation?  
It is nigh unusable because of that.  

## ~~Anguish~~ Acceptance

~~AAAAAAAAAAAAAAAAARRRRRRRRRRRRRRRRRRGHGHHGGGGGHHHHHH~~
I have come to accept that nothing is perfect,
and everybody makes mistakes. Toolkits are no exception to that.
~~I HATE-~~ I have forgiven GTK3, and welcome it back into my arms.  
What about tcl/tk? Well it had no easy way of... putting pixels on a canvas.
There where hacks and togl, but none of them worked, or made me happy.  
Togl may also not work on Mac, which I can't test. So that is worrying.
~~I HAVE SPENT DaYS! I am no longer MADE FROM PISS! **I AM NUCLEAR FUSION POWERED DEATH!**~~
Also, apparently no one over in tcl-land ever considers the end of X11.
Once distros don't ship anything for XWayland, say bye bye to your tk apps.
Xorg is also not getting any less bugs with time.

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
