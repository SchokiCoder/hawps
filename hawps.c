// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

#include <errno.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "world.h"

#define _DUMB_MAGIC(arg) #arg
#define DEF_TO_STRING(name) _DUMB_MAGIC(name)

#define STD_TICKRATE     24.0
#define STD_WORLD_SCALE  10
#define STD_WORLD_WIDTH  40
#define STD_WORLD_HEIGHT 40

const char *APP_HELP =  "Usage: " APP_NAME " [OPTIONS]\n"
"\n"
"Silly program to simulate physics in *very* convincing ways.\n"
"It'll be great. Trust me.\n"
"\n"
"Options:\n"
"\n"
"    -a --about\n"
"        prints program name, version, license and repository information then exits\n"
"\n"
"    -h --help\n"
"        prints this message then exits\n"
"\n"
"    --tickrate\n"
"        sets the tickrate (ticks per second), which effects visible speed\n"
"        default: " DEF_TO_STRING(STD_TICKRATE) "\n"
"\n"
"    -v --version\n"
"        prints version information then exits\n"
"\n"
"    --world_scale\n"
"        sets the graphical scale of the physical world\n"
"        default: " DEF_TO_STRING(STD_WORLD_SCALE) "\n"
"\n"
"    --world_width\n"
"        sets the width of the world\n"
"        default: " DEF_TO_STRING(STD_WORLD_WIDTH) "\n"
"\n"
"    --world_height\n"
"        sets the height of the world\n"
"        default: " DEF_TO_STRING(STD_WORLD_HEIGHT) "\n"
"\n"
"Default keybinds:\n"
"\n"
"    ESC\n"
"        quit the program\n"
"\n"
"    Space\n"
"        pause world\n"
"\n";

int
draw_world(
	const struct World  wld,
	SDL_Surface        *frame,
	SDL_Window         *win);

int
handle_args(
	int    argc,
	char  *argv[],
	int   *wld_scale,
	int   *wld_w,
	int   *wld_h);

void
handle_events(
	int *active,
	float *pause_mod);

void
handle_key(
	SDL_Keycode key,
	int *active,
	float *pause_mod);

int
draw_world(
	const struct World  wld,
	SDL_Surface        *frame,
	SDL_Window         *win)
{
	Uint32 pixel;
	SDL_Rect rect;
	SDL_Surface *tmp;
	int x, y;

	for (x = 0; x < wld.w; x++) {
		for (y = 0; y < wld.h; y++) {
			rect.x = x;
			rect.y = y;
			rect.w = 1;
			rect.h = 1;

			pixel = SDL_MapRGB(frame->format,
			                   MAT_R[wld.dots[x][y]],
			                   MAT_G[wld.dots[x][y]],
			                   MAT_B[wld.dots[x][y]]);
			SDL_FillRect(frame, &rect, pixel);
		}
	}

	tmp = SDL_GetWindowSurface(win);
	if (NULL == tmp) {
		fprintf(stderr, "Window has no surface :O\n");
		return 1;
	}

	SDL_BlitScaled(frame, &frame->clip_rect, tmp, &tmp->clip_rect);
	SDL_UpdateWindowSurface(win);

	return 0;
}

int
handle_args(
	int    argc,
	char  *argv[],
	int   *wld_scale,
	int   *wld_w,
	int   *wld_h)
{
	const char *ERR_ARG_CONV =
		"\"%s\" could not be converted to a %s\n";
	const char *ERR_NO_ARG_VALUE =
		"The argument \"%s\" needs a value from the next argument\n";
	int i;
	int vi;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-a") == 0 ||
		    strcmp(argv[i], "--about") == 0) {
			printf("The source code of \"%s\" aka %s %s is available, "
			       "licensed under the %s at:\n"
			       "%s\n\n"
			       "If you did not receive a copy of the license, "
			       "see below:\n"
			       "%s\n",
			       APP_NAME_FORMAL, APP_NAME, APP_VERSION,
			       APP_LICENSE,
			       APP_REPOSITORY,
			       APP_LICENSE_URL);
			return 0;
		} else if (strcmp(argv[i], "-h") == 0 ||
		           strcmp(argv[i], "--help") == 0) {
			printf("%s", APP_HELP);
			return 0;
		} else if (strcmp(argv[i], "-v") == 0 ||
		           strcmp(argv[i], "--version") == 0) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return 0;
		} else if (strcmp(argv[i], "--world_scale") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 0;
			}
			i++;

			errno = 0;
			vi = strtol(argv[i], NULL, 10);
			if (errno != 0 || 0 == vi) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "int");
				return 0;
			}
			*wld_scale = vi;
		} else if (strcmp(argv[i], "--world_width") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 1;
			}
			i++;

			errno = 0;
			vi = strtol(argv[i], NULL, 10);
			if (errno != 0 || 0 == vi) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "int");
				return 1;
			}
			*wld_w = vi;
		} else if (strcmp(argv[i], "--world_height") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 1;
			}
			i++;

			errno = 0;
			vi = strtol(argv[i], NULL, 10);
			if (errno != 0 || 0 == vi) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "int");
				return 1;
			}
			*wld_h = vi;
		} else {
			fprintf(stderr,
			        "Argument \"%s\" is not recognized.\n",
			        argv[i]);
			return 0;
		}
	}

	return 1;
}

void
handle_events(
	int *active,
	float *pause_mod)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYUP:
			handle_key(event.key.keysym.sym, active, pause_mod);
			break;

		case SDL_QUIT:
			*active = 0;
			break;

		default:
			break;
		}
	}
}

void
handle_key(
	SDL_Keycode key,
	int *active,
	float *pause_mod)
{
	switch (key) {
	case SDLK_ESCAPE:
		*active = 0;
		break;

	case SDLK_SPACE:
		if (*pause_mod != 0.0) {
			*pause_mod = 0.0;
		} else {
			*pause_mod = 1.0;
		}
		break;

	default:
		break;
	}
}

int
main(
	int argc,
	char *argv[])
{
	int           active = 1;
	float         delta;
	SDL_Surface  *frame = NULL;
	clock_t       t1 = 0.0;
	clock_t       t2 = 0.0;
	float         tickrate = STD_TICKRATE;
	float         pause_mod = 1.0;
	SDL_Window   *win = NULL;
	int           wld_scale = STD_WORLD_SCALE;
	struct World  wld = {.w = STD_WORLD_WIDTH, .h = STD_WORLD_HEIGHT};
	int           x, y;

	if (handle_args(argc, argv, &wld_scale, &wld.w, &wld.h) == 0) {
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL Init failed\n");
		return 0;
	}

	win = SDL_CreateWindow(APP_NAME_FORMAL,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       wld.w * wld_scale,
	                       wld.h * wld_scale,
	                       SDL_WINDOW_SHOWN);
	if (NULL == win) {
		fprintf(stderr, "Couldn't open window\n");
		goto cleanup;
	}

	frame = SDL_CreateRGBSurface(0, wld.w, wld.h, 32,
	                             0, 0, 0, 0);
	if (NULL == frame) {
		fprintf(stderr, "Couldn't create surface\n");
		goto cleanup;
	}

	if (World_new(&wld, wld.w, wld.h) != 0) {
		fprintf(stderr, "Couldn't initialize world\n");
		goto cleanup;
	}

	// TODO: remove manual tomfoolery
	const int spawn1X = wld.w / 3;
	const int spawn1Y = wld.h / 3 * 2;
	const int spawn1W = 10;
	const int spawn1H = 10;
	const int spawn2X = wld.w / 3;
	const int spawn2Y = 0;
	const int spawn2W = wld.w / 4;
	const int spawn2H = wld.h / 3;
	const int spawn3X = 0;
	const int spawn3Y = wld.h - 5;
	const int spawn3W = wld.w - 1;
	const int spawn3H = 1;
	for (x = spawn2X; x < spawn2X + spawn2W; x++) {
		for (y = spawn2Y; y < spawn2Y + spawn2H; y++) {
			wld.dots[x][y] = M_water;
		}
	}
	for (x = spawn1X; x < spawn1X + spawn1W; x++) {
		for (y = spawn1Y; y < spawn1Y + spawn1H; y++) {
			wld.dots[x][y] = M_sand;
		}
	}
	for (x = spawn3X; x < spawn3X + spawn3W; x++) {
		for (y = spawn3Y; y < spawn3Y + spawn3H; y++) {
			wld.dots[x][y] = M_iron;
		}
	}

	while (active) {
		handle_events(&active, &pause_mod);

		t1 = clock();
		delta = 1.0 * (t1 - t2) / CLOCKS_PER_SEC;
		if (delta * pause_mod >= (1.0 / tickrate)) {
			if (draw_world(wld, frame, win) != 0) {
				active = 0;
				break;
			}

			World_tick(&wld);

			t2 = t1;
		}
	}

cleanup:
	World_free(&wld);
	SDL_FreeSurface(frame);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
