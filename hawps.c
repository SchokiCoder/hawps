// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

#include <errno.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ui.h"
#include "world.h"

#define _DUMB_MAGIC(arg) #arg
#define DEF_TO_STRING(name) _DUMB_MAGIC(name)

#define STD_TICKRATE     24.0
#define STD_WINDOW_W     640
#define STD_WINDOW_H     480
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
"    -a -about\n"
"        prints program name, version, license and repository information then exits\n"
"\n"
"    -H -height DECIMAL\n"
"        sets the window height\n"
"        default: " DEF_TO_STRING(STD_WINDOW_H) "\n"
"\n"
"    -h -help\n"
"        prints this message then exits\n"
"\n"
"    -noborder\n"
"        removes window decoration from window\n"
"\n"
"    -tickrate FRACTION\n"
"        sets the tickrate (ticks per second), which effects visible speed\n"
"        default: " DEF_TO_STRING(STD_TICKRATE) "\n"
"\n"
"    -v -version\n"
"        prints version information then exits\n"
"\n"
"    -W -width DECIMAL\n"
"        sets the window width\n"
"        default: " DEF_TO_STRING(STD_WINDOW_W) "\n"
"\n"
"    -window -windowed\n"
"        starts the app in windowed mode... not fullscreen\n"
"\n"
"    -world_scale DECIMAL\n"
"        sets the graphical scale of the physical world\n"
"        default: " DEF_TO_STRING(STD_WORLD_SCALE) "\n"
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
draw(
	SDL_Surface        *frame,
	const struct UI     ui,
	SDL_Window         *win,
	const struct World  wld,
	SDL_Surface        *wld_surface);

void
draw_world(
	const struct World  wld,
	const SDL_Rect      wld_rect,
	SDL_Surface        *wld_surface);

int
handle_args(
	int     argc,
	char   *argv[],
	float  *tickrate,
	Uint32 *win_flags,
	int    *win_w,
	int    *win_h,
	int    *wld_scale);

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
draw(
	SDL_Surface        *frame,
	const struct UI     ui,
	SDL_Window         *win,
	const struct World  wld,
	SDL_Surface        *wld_surface)
{
	SDL_Surface *tmp;

	UI_draw(ui, frame);
	draw_world(wld, ui.world, wld_surface);

	SDL_BlitSurface(wld_surface,
	                &wld_surface->clip_rect,
	                frame,
	                (SDL_Rect*) &ui.world);

	tmp = SDL_GetWindowSurface(win);
	if (NULL == tmp) {
		fprintf(stderr, "Window has no surface :O\n");
		return 1;
	}

	SDL_BlitScaled(frame, &frame->clip_rect, tmp, &tmp->clip_rect);
	SDL_UpdateWindowSurface(win);

	return 0;
}

void
draw_world(
	const struct World  wld,
	const SDL_Rect      wld_rect,
	SDL_Surface        *wld_surface)
{
	Uint32 pixel;
	SDL_Rect rect = {.w = 1, .h = 1};
	int x, y;

	for (x = 0; x < wld.w; x++) {
		rect.x = wld_rect.x + x;

		for (y = 0; y < wld.h; y++) {
			rect.y = wld_rect.y + y;

			pixel = SDL_MapRGB(wld_surface->format,
			                   MAT_R[wld.dots[x][y]],
			                   MAT_G[wld.dots[x][y]],
			                   MAT_B[wld.dots[x][y]]);
			SDL_FillRect(wld_surface, &rect, pixel);
		}
	}
}

int
handle_args(
	int     argc,
	char   *argv[],
	float  *tickrate,
	Uint32 *win_flags,
	int    *win_w,
	int    *win_h,
	int    *wld_scale)
{
	const char *ERR_ARG_CONV =
		"\"%s\" could not be converted to a %s\n";
	const char *ERR_NO_ARG_VALUE =
		"The argument \"%s\" needs a value from the next argument\n";
	int i;
	int vi;
	float vf;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-a") == 0 ||
		    strcmp(argv[i], "-about") == 0) {
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
		} else if (strcmp(argv[i], "-H") == 0 ||
		           strcmp(argv[i], "-height") == 0) {
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
			*win_h = vi;
		} else if (strcmp(argv[i], "-h") == 0 ||
		           strcmp(argv[i], "-help") == 0) {
			printf("%s", APP_HELP);
			return 0;
		} else if (strcmp(argv[i], "-noborder") == 0) {
			*win_flags |= SDL_WINDOW_BORDERLESS;
		} else if (strcmp(argv[i], "-tickrate") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 1;
			}
			i++;

			errno = 0;
			vf = strtof(argv[i], NULL);
			if (errno != 0) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "float");
				return 1;
			}
			*tickrate = vf;
		} else if (strcmp(argv[i], "-v") == 0 ||
		           strcmp(argv[i], "-version") == 0) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return 0;
		} else if (strcmp(argv[i], "-W") == 0 ||
		           strcmp(argv[i], "-width") == 0) {
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
			*win_w = vi;
		} else if (strcmp(argv[i], "-window") == 0 ||
		           strcmp(argv[i], "-windowed") == 0) {
			*win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
			*win_flags |= SDL_WINDOW_SHOWN;
		} else if (strcmp(argv[i], "-world_scale") == 0) {
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
	clock_t       t1 = 0;
	clock_t       t2 = -9999999;
	float         tickrate = STD_TICKRATE;
	float         pause_mod = 1.0;
	struct UI     ui = UI_new();
	SDL_Window   *win = NULL;
	Uint32        win_flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	int           win_w = STD_WINDOW_W;
	int           win_h = STD_WINDOW_H;
	int           wld_scale = STD_WORLD_SCALE;
	SDL_Surface  *wld_surface = NULL;
	struct World  wld = {.w = STD_WORLD_WIDTH, .h = STD_WORLD_HEIGHT};

	if (handle_args(argc,
	                argv,
	                &tickrate,
	                &win_flags,
	                &win_w,
	                &win_h,
	                &wld_scale) == 0) {
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL Init failed\n");
		return 0;
	}

	win = SDL_CreateWindow(APP_NAME_FORMAL,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       win_w,
	                       win_h,
	                       win_flags);
	if (NULL == win) {
		fprintf(stderr, "Couldn't open window\n");
		goto cleanup;
	}

	SDL_GetWindowSize(win, &win_w, &win_h);
	UI_set_wide_layout(&ui, win_w, win_h);

	wld_surface = SDL_CreateRGBSurface(0, wld.w, wld.h, 32,
	                             0, 0, 0, 0);
	if (NULL == wld_surface) {
		fprintf(stderr, "Couldn't create surface\n");
		goto cleanup;
	}

	frame = SDL_CreateRGBSurface(0, win_w, win_h, 32,
	                             0, 0, 0, 0);
	if (NULL == frame) {
		fprintf(stderr, "Couldn't create surface\n");
		goto cleanup;
	}

	if (World_new(&wld, wld.w, wld.h) != 0) {
		fprintf(stderr, "Couldn't initialize world\n");
		goto cleanup;
	}

	while (active) {
		handle_events(&active, &pause_mod);

		t1 = clock();
		delta = 1.0 * (t1 - t2) / CLOCKS_PER_SEC;
		if (delta * pause_mod >= (1.0 / tickrate)) {
			if (draw(frame, ui, win, wld, wld_surface) != 0) {
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
	SDL_FreeSurface(wld_surface);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
