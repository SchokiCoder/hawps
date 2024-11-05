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

#define STD_TICKRATE     24.0
#define STD_DOTSCALE     10
#define STD_WORLD_WIDTH  80
#define STD_WORLD_HEIGHT 60

#define _DUMB_MAGIC(arg) #arg
#define DEF_TO_STRING(name) _DUMB_MAGIC(name)

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
"    --dotscale\n"
"        sets the graphical scale of dots and thus the world too\n"
"        default: " DEF_TO_STRING(STD_DOTSCALE) "\n"
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

enum Mat {
	M_none,
	M_sand,
	M_water
};

struct World {
	int        w;
	int        h;
	enum Mat **dots;
};

enum MatState {
	MS_static,
	MS_grain,
	MS_liquid,
};

const char *MAT_NAME[] =          {"None",    "Sand",   "Water"};
const int MAT_WEIGHT[] =          {0,         2,        1};
const enum MatState MAT_STATE[] = {MS_static, MS_grain, MS_liquid};
const Uint8 MAT_R[] =             {0,         238,      100};
const Uint8 MAT_G[] =             {0,         217,      100};
const Uint8 MAT_B[] =             {0,         86,       255};

void
apply_gravity(
	struct World *wld);

int
can_liquid_displace(
	enum Mat src, enum Mat dest);

int
can_grain_displace(
	enum Mat src, enum Mat dest);

void
drop_grain(
	struct World *wld,
	int x,
	int y);

void
drop_liquid(
	struct World *wld,
	int x,
	int y);

int
draw_world(
	const struct World  wld,
	SDL_Surface        *frame,
	SDL_Window         *win);

int
handle_args(
	int    argc,
	char  *argv[],
	int   *dotscale,
	float *tickrate,
	int   *world_w,
	int   *world_h);

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
	struct World *wld)
{
	int x, y;

	for (x = 0; x < wld->w; x++) {
		for (y = wld->h - 2; y >= 0; y--) {
			switch (MAT_STATE[wld->dots[x][y]]) {
			case MS_static:
				break;

			case MS_grain:
				drop_grain(wld, x, y);
				break;

			case MS_liquid:
				drop_liquid(wld, x, y);
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
	struct World *wld,
	int x,
	int y)
{
	enum Mat *below;
	enum Mat *cur;
	enum Mat  tmp;

	cur = &wld->dots[x][y];
	below = &wld->dots[x][y + 1];

	if (can_grain_displace(*cur, *below)) {
		tmp = *below;
		*below = *cur;
		*cur = tmp;
		return;
	}

	if (x - 1 >= 0) {
		below = &wld->dots[x - 1][y + 1];

		if (can_grain_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}
	}
	if (x + 1 < wld->w) {
		below = &wld->dots[x + 1][y + 1];

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
	struct World *wld,
	int x,
	int y)
{
	int bx;
	enum Mat *below;
	enum Mat *cur;
	enum Mat  tmp;

	cur = &wld->dots[x][y];
	below = &wld->dots[x][y + 1];

	if (can_liquid_displace(*cur, *below)) {
		tmp = *below;
		*below = *cur;
		*cur = tmp;
		return;
	}

	for (bx = x - 1; bx >= 0; bx--) {
		below = &wld->dots[bx][y + 1];
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
	for (bx = x + 1; bx < wld->w; bx++) {
		below = &wld->dots[bx][y + 1];
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
	int   *dotscale,
	float *tickrate,
	int   *world_w,
	int   *world_h)
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
		} else if (strcmp(argv[i], "-h") == 0 ||
		           strcmp(argv[i], "--help") == 0) {
			printf("%s", APP_HELP);
			return 0;
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
		} else if (strcmp(argv[i], "-v") == 0 ||
		           strcmp(argv[i], "--version") == 0) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return 0;
		} else if (strcmp(argv[i], "--world_width") == 0) {
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
			*world_w = vi;
		} else if (strcmp(argv[i], "--world_height") == 0) {
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
			*world_h = vi;
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
	int           dotscale = STD_DOTSCALE;
	SDL_Surface  *frame = NULL;
	clock_t       t1, t2;
	float         tickrate = STD_TICKRATE;
	float         pause_mod = 1.0;
	SDL_Window   *win = NULL;
	struct World  wld;
	int           x, y;

	wld.w = STD_WORLD_WIDTH;
	wld.h = STD_WORLD_HEIGHT;

	if (handle_args(argc,
	                argv,
			&dotscale,
			&tickrate,
			&wld.w,
			&wld.h) == 0) {
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL Init failed\n");
		return 0;
	}

	win = SDL_CreateWindow(APP_NAME_FORMAL,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       wld.w * dotscale,
	                       wld.h * dotscale,
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

	wld.dots = malloc(sizeof(enum Mat*) * wld.w);
	if (NULL == wld.dots) {
		fprintf(stderr, "Couldn't create world\n");
		goto cleanup;
	}

	for (x = 0; x < wld.w; x++) {
		wld.dots[x] = malloc(sizeof(enum Mat) * wld.h);
		if (NULL == wld.dots[x]) {
			fprintf(stderr, "Couldn't create world col\n");
			goto cleanup;
		}

		for (y = 0; y < wld.h; y++) {
			wld.dots[x][y] = M_none;
		}
	}

	// TODO: remove manual tomfoolery
	const int spawn1X = wld.w / 3;
	const int spawn1Y = wld.h / 3 * 2;
	const int spawn1W = 10;
	const int spawn1H = 10;
	const int spawn2X = 0;
	const int spawn2Y = 0;
	const int spawn2W = wld.w / 2;
	const int spawn2H = wld.h;

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

	t1 = 0.0;
	t2 = 0.0;

	while (active) {
		handle_events(&active, &pause_mod);

		t1 = clock();
		delta = 1.0 * (t1 - t2) / CLOCKS_PER_SEC;
		if (delta * pause_mod >= (1.0 / tickrate)) {
			if (draw_world(wld, frame, win) != 0) {
				active = 0;
				break;
			}
			apply_gravity(&wld);

			t2 = t1;
		}
	}

cleanup:
	for (x = 0; x < wld.w; x++) {
		if (NULL != wld.dots[x]) {
			free(wld.dots[x]);
		}
	}
	free(wld.dots);

	SDL_FreeSurface(frame);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
