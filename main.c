// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

#include <errno.h>
#include <SDL.h>
#include <stdio.h>
#include <time.h>

#define APP_NAME        "hawps"
#define APP_NAME_FORMAL "Half Assed Wannabe Physics Simulator"
#define APP_VERSION     "v0.1"
#define APP_LICENSE     "GPL-2.0-or-later"
#define APP_REPOSITORY  "https://github.com/SchokiCoder/hawps"
#define APP_LICENSE_URL "https://www.gnu.org/licenses/gpl-2.0.html"

#define STD_TICKRATE 24.0
#define STD_DOTSCALE 10
#define WORLD_WIDTH  80
#define WORLD_HEIGHT 60

enum Mat {
	M_none,
	M_sand,
	M_water
};

enum MatState {
	MS_static,
	MS_grain,
	MS_liquid,
};

typedef enum Mat World[WORLD_WIDTH][WORLD_HEIGHT];

const char *MAT_NAME[] =          {"None",    "Sand",   "Water"};
const int MAT_WEIGHT[] =          {0,         2,        1};
const enum MatState MAT_STATE[] = {MS_static, MS_grain, MS_liquid};
const Uint8 MAT_R[] =             {0,         238,      100};
const Uint8 MAT_G[] =             {0,         217,      100};
const Uint8 MAT_B[] =             {0,         86,       255};

void
apply_gravity(
	World dots);

int
can_liquid_displace(
	enum Mat src, enum Mat dest);

int
can_grain_displace(
	enum Mat src, enum Mat dest);

void
drop_grain(
	World dots,
	int x,
	int y);

void
drop_liquid(
	World dots,
	int x,
	int y);

int
draw_world(
	World        dots,
	SDL_Surface *frame,
	SDL_Window  *win);

int
handle_args(
	int    argc,
	char  *argv[],
	int   *dotscale,
	float *tickrate);

void
handle_events(
	int *active,
	float *pause_mod);

void
handle_key(
	SDL_Keycode key,
	int *active,
	float *pause_mod);

void
apply_gravity(
	World dots)
{
	int x, y;

	for (x = 0; x < WORLD_WIDTH; x++) {
		for (y = WORLD_HEIGHT - 2; y >= 0; y--) {
			switch (MAT_STATE[dots[x][y]]) {
			case MS_static:
				break;

			case MS_grain:
				drop_grain(dots, x, y);
				break;

			case MS_liquid:
				drop_liquid(dots, x, y);
				break;
			}
		}
	}
}

int
can_liquid_displace(
	enum Mat src, enum Mat dest)
{
	if (M_none == dest ||
	    MAT_WEIGHT[dest] < MAT_WEIGHT[src]) {
		return 1;
	}

	return 0;
}

int
can_grain_displace(
	enum Mat src, enum Mat dest)
{
	if (M_none == dest ||
	   (MAT_WEIGHT[dest] < MAT_WEIGHT[src] &&
	    MAT_STATE[dest] == MS_liquid)) {
		return 1;
	}

	return 0;
}

void
drop_grain(
	World dots,
	int x,
	int y)
{
	enum Mat *below;
	enum Mat *cur;
	enum Mat  tmp;

	cur = &dots[x][y];
	below = &dots[x][y + 1];

	if (can_grain_displace(*cur, *below)) {
		tmp = *below;
		*below = *cur;
		*cur = tmp;
		return;
	}

	if (x - 1 >= 0) {
		below = &dots[x - 1][y + 1];

		if (can_grain_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}
	}
	if (x + 1 < WORLD_WIDTH) {
		below = &dots[x + 1][y + 1];

		if (can_grain_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}
	}
}

void
drop_liquid(
	World dots,
	int x,
	int y)
{
	int bx;
	enum Mat *below;
	enum Mat *cur;
	enum Mat  tmp;

	cur = &dots[x][y];
	below = &dots[x][y + 1];

	if (can_liquid_displace(*cur, *below)) {
		tmp = *below;
		*below = *cur;
		*cur = tmp;
		return;
	}

	for (bx = x - 1; bx >= 0; bx--) {
		below = &dots[bx][y + 1];
		if (can_liquid_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}

		if (MAT_STATE[*below] == MS_static) {
			break;
		}
	}
	for (bx = x + 1; bx < WORLD_WIDTH; bx++) {
		below = &dots[bx][y + 1];
		if (can_liquid_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}

		if (MAT_STATE[*below] == MS_static) {
			break;
		}
	}
}

int
draw_world(
	World        dots,
	SDL_Surface *frame,
	SDL_Window  *win)
{
	Uint32 pixel;
	SDL_Rect rect;
	SDL_Surface *tmp;
	int x, y;

	for (x = 0; x < WORLD_WIDTH; x++) {
		for (y = 0; y < WORLD_HEIGHT; y++) {
			rect.x = x;
			rect.y = y;
			rect.w = 1;
			rect.h = 1;

			pixel = SDL_MapRGB(frame->format,
			                   MAT_R[dots[x][y]],
			                   MAT_G[dots[x][y]],
			                   MAT_B[dots[x][y]]);
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
	int   *dotscale,
	float *tickrate)
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
		} else if (strcmp(argv[i], "--dotscale") == 0) {
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
			*dotscale = vi;
		} else if (strcmp(argv[i], "--tickrate") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 0;
			}
			i++;

			errno = 0;
			vf = strtof(argv[i], NULL);
			if (errno != 0) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "float");
				return 0;
			}
			*tickrate = vf;
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
	int          active = 1;
	float        delta;
	World        dots;
	int          dotscale = STD_DOTSCALE;
	SDL_Surface *frame = NULL;
	clock_t      t1, t2;
	float        tickrate = STD_TICKRATE;
	float        pause_mod = 1.0;
	SDL_Window  *win = NULL;
	int          x, y;

	if (handle_args(argc, argv, &dotscale, &tickrate) == 0) {
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL Init failed\n");
		return 0;
	}

	win = SDL_CreateWindow(APP_NAME_FORMAL,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       WORLD_WIDTH * dotscale,
	                       WORLD_HEIGHT * dotscale,
	                       SDL_WINDOW_SHOWN);
	if (NULL == win) {
		fprintf(stderr, "Couldn't open window\n");
		goto cleanup;
	}

	frame = SDL_CreateRGBSurface(0, WORLD_WIDTH, WORLD_HEIGHT, 32,
	                             0, 0, 0, 0);
	if (NULL == frame) {
		fprintf(stderr, "Couldn't create surface\n");
		goto cleanup;
	}

	for (x = 0; x < WORLD_WIDTH; x++) {
		for (y = 0; y < WORLD_HEIGHT; y++) {
			dots[x][y] = M_none;
		}
	}

	// TODO: remove manual tomfoolery
	const int spawn1X = WORLD_WIDTH / 3;
	const int spawn1Y = WORLD_HEIGHT / 3 * 2;
	const int spawn1W = 10;
	const int spawn1H = 10;
	const int spawn2X = 0;
	const int spawn2Y = 0;
	const int spawn2W = WORLD_WIDTH / 2;
	const int spawn2H = WORLD_HEIGHT;

	for (x = spawn2X; x < spawn2X + spawn2W; x++) {
		for (y = spawn2Y; y < spawn2Y + spawn2H; y++) {
			dots[x][y] = M_water;
		}
	}
	for (x = spawn1X; x < spawn1X + spawn1W; x++) {
		for (y = spawn1Y; y < spawn1Y + spawn1H; y++) {
			dots[x][y] = M_sand;
		}
	}

	t1 = 0.0;
	t2 = 0.0;

	while (active) {
		handle_events(&active, &pause_mod);

		t1 = clock();
		delta = 1.0 * (t1 - t2) / CLOCKS_PER_SEC;
		if (delta * pause_mod >= (1.0 / tickrate)) {
			if (draw_world(dots, frame, win) != 0) {
				active = 0;
				break;
			}
			apply_gravity(dots);

			t2 = t1;
		}
	}

cleanup:
	SDL_FreeSurface(frame);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
