/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core/core.h"
#include "csi/csi.h"

enum Tool {
	TOOL_BRUSH,
	TOOL_SPAWNER,
	TOOL_ERASER,
	TOOL_HEATER,
	TOOL_COOLER,
};

#define LEFT_ST_BAR_SIZE  150
#define RIGHT_ST_BAR_SIZE 100
#define ST_BAR_SIZE       LEFT_ST_BAR_SIZE + RIGHT_ST_BAR_SIZE
#define VISION_SIZE       16

#define CELSIUS_TO_KELVIN 273.15

#define FIRST_REAL_MAT MAT_SAND

#define KEY_BACKSPACE "\x7f"
#define KEY_HOME      "\x1b[H"
#define KEY_INSERT    "\033[2~"
#define KEY_DELETE    "\033[3~"
#define KEY_PGUP      "\033[5~"
#define KEY_PGDOWN    "\033[6~"
#define KEY_END       "\x1b[F"

#define SIG_INT  "\003"
#define SIG_TSTP "\004"

#define THERMAL_VISION_MIN_T (-75.0 + CELSIUS_TO_KELVIN)

#define STD_BRUSH_RADIUS  2
#define STD_ERASER_RADIUS 5
#define STD_SELECTED_TOOL TOOL_BRUSH
#define STD_SIM_SUBSAMPLE 4
#define STD_TEMPERATURE   (20.0 + CELSIUS_TO_KELVIN)
#define STD_THERMO_RADIUS STD_BRUSH_RADIUS
#define STD_TICKRATE      120

static const char APP_ABOUT[] = "The source code of \"%s\" aka %s %s is available,\n"
"licensed under the %s at:\n"
"%s\n"
"\n"
"If you did not receive a copy of the license, see below:\n"
"%s\n";

static const char APP_HELP[] = "Usage: %s [OPTIONS]\n"
"\n"
"Silly program to simulate physics in *very* convincing ways.\n"
"It'll be great. Trust me.\n"
"\n"
"Options:\n"
"\n"
"    -a -about\n"
"        prints program name, version, license and repository information then exits\n"
"\n"
"    -h -help\n"
"        prints this message then exits\n"
"\n"
"    -temperature\n"
"        sets the temperature of every new dot in Kelvin\n"
"        0 °C == %.2f K\n"
"        default: %.2f\n"
"\n"
"    -tickrate NUMBER\n"
"        sets the tickrate (ticks per second),\n"
"        which also effects simulation speed\n"
"        only use when otherwise performance problems occur\n"
"        default: %i\n"
"\n"
"    -v -version\n"
"        prints version information then exits\n"
"\n"
"Default keybinds:\n"
"\n"
"    /\n"
"        enter the command line\n"
"\n"
"    Escape\n"
"        quit the program\n"
"\n"
"    Space\n"
"        pause world\n"
"\n"
"    H J K L\n"
"        move cursor left, down, up, and right, respectively\n"
"\n"
"    Plus and Minus\n"
"        increase and decrease the simulation speed respectively\n"
"        default: %.2f updates per second\n"
"\n"
"    T\n"
"        toggle thermal vision (grayscale displaying %.0f to %.0f degree Celsius)\n"
"\n";

void
draw(const enum Mat        brush_mat,
     const bool            cmdmode,
     const int             cursor_x,
     const int             cursor_y,
     const char           *ip_address,
     const enum Tool       sel_tool,
     const enum Mat        spawner_mat,
     const bool            th_vision,
     const size_t          win_w,
     const size_t          win_h,
     const struct World    world,
     const char           *world_name,
     const size_t          world_w,
     const size_t          world_h);

bool
handle_args(int     argc,
            char  **argv,
            float  *temperature,
            size_t *tickrate);

bool
int_flag_parse(int    argc,
               char **argv,
               int   *idx,
               long  *out);

void
draw(const enum Mat        brush_mat,
     const bool            cmdmode,
     const int             cursor_x,
     const int             cursor_y,
     const char           *ip_address,
     const enum Tool       sel_tool,
     const enum Mat        spawner_mat,
     const bool            th_vision,
     const size_t          win_w,
     const size_t          win_h,
     const struct World    world,
     const char           *world_name,
     const size_t          world_w,
     const size_t          world_h)
{
	char left_st_bar[LEFT_ST_BAR_SIZE];
	char right_st_bar[RIGHT_ST_BAR_SIZE];
	char st_bar[ST_BAR_SIZE];
	char vision[VISION_SIZE];
	size_t world_draw_w;
	size_t world_draw_h;
	size_t x, y;

	fputs(CSI_CLEAR, stdout);

	if (world_w > win_w) {
		world_draw_w = win_w;
	} else {
		world_draw_w = world_w;
	}
	if (world_h > win_h - 2) {
		world_draw_h = win_h - 2;
	} else {
		world_draw_h = world_h;
	}

	for (y = 0; y < world_draw_h; y++) {
		for (x = 0; x < world_draw_w; x++) {
			if (world.dots[x][y] == MAT_NONE) {
				fputs(" ", stdout);
				continue;
			}

			switch (world.states[x][y]) {
			case MS_STATIC:
			case MS_GRAIN:
				fputs("X", stdout);
				break;

			case MS_LIQUID:
				fputs("+", stdout);
				break;

			case MS_GAS:
				fputs("-", stdout);
				break;

			case MS_COUNT:
				break;
			}
		}
	}

	if (th_vision) {
		strncpy(vision, "Thermal", VISION_SIZE);
	} else {
		strncpy(vision, "Normal", VISION_SIZE);
	}

	sprintf(left_st_bar, "%s (%i,%i) | View:%s | %s",
	        world_name, cursor_x, cursor_y, vision, ip_address);
	sprintf(right_st_bar, "%s: bindings, %s: help", "bind1", "bind2");
	sprintf(st_bar, "%%s%%%lus", win_w - strlen(left_st_bar));
	printf(st_bar, left_st_bar, right_st_bar);

	if (cmdmode) {
		fputs(":cmd input currently not implemented", stdout);
	} else {
		switch (sel_tool) {
		case TOOL_BRUSH:
			printf("BRUSH %s", MAT_NAME[brush_mat]);
			break;

		case TOOL_SPAWNER:
			printf("SPAWNER %s", MAT_NAME[spawner_mat]);
			break;

		case TOOL_ERASER:
			fputs("ERASER", stdout);
			break;

		case TOOL_HEATER:
			fputs("HEATER", stdout);
			break;

		case TOOL_COOLER:
			fputs("COOLER", stdout);
			break;
		}
	}
}

bool
handle_args(int     argc,
            char  **argv,
            float  *temperature,
            size_t *tickrate)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-a") ||
		    strcmp(argv[i], "-about")) {
			printf(APP_ABOUT,
			       APP_NAME_FORMAL, APP_NAME, APP_VERSION,
			       APP_LICENSE,
			       APP_REPOSITORY,
			       APP_LICENSE_URL);
			return false;
		} else if (strcmp(argv[i], "-h") ||
		           strcmp(argv[i], "-help")) {
			printf(APP_HELP,
			       APP_NAME,
			       CELSIUS_TO_KELVIN,
			       STD_TEMPERATURE,
			       STD_TICKRATE,
			       (float) STD_TICKRATE / (float) STD_SIM_SUBSAMPLE,
			       THERMAL_VISION_MIN_T - CELSIUS_TO_KELVIN,
			       THERMAL_VISION_MIN_T - CELSIUS_TO_KELVIN + 255);
			return false;
		} else if (strcmp(argv[i], "-temperature")) {
			if (int_flag_parse(argc, argv, &i, (long*) temperature)) {
				return false;
			}
			if (*temperature < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], "-tickrate")) {
			if (int_flag_parse(argc, argv, &i, (long*) tickrate)) {
				return false;
			}
			i++;
		} else if (strcmp(argv[i], "-v") ||
		           strcmp(argv[i],  "-version")) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return false;
		} else {
			fprintf(stderr,
			        "Argument \"%s\" is not recognized",
			        argv[i]);
			return false;
		}
	}

	return true;
}

bool
int_flag_parse(int    argc,
               char **argv,
               int   *idx,
               long  *out)
{
	if (argc <= *idx + 1) {
		fprintf(stderr,
		        "The argument \"%s\" needs to be followed by a value",
		        argv[*idx]);
		return false;
	}
	*idx += 1;

	errno = 0;
	*out = strtol(argv[*idx], NULL, 10);
	if (errno != 0) {
		fprintf(stderr,
		        "\"%s\" could not be converted to an int",
		        argv[*idx - 1]);
		return false;
	}

	return true;
}

int
main(int    argc,
     char **argv)
{
	bool           active = true;
	enum Mat       brush_mat = FIRST_REAL_MAT;
	int            brush_radius = STD_BRUSH_RADIUS;
	bool           cmdmode = false;
	int            cursor_x = 0;
	int            cursor_y = 0;
	int            eraser_radius = STD_ERASER_RADIUS;
	bool           paused = false;
	clock_t        now;
	enum Tool      sel_tool = STD_SELECTED_TOOL;
	size_t         sim_subsample = STD_SIM_SUBSAMPLE;
	enum Mat       spawner_mat = FIRST_REAL_MAT;
	float          temperature = STD_TEMPERATURE;
	struct winsize tempws;
	int            thermo_radius = STD_THERMO_RADIUS;
	bool           th_vision = false;
	clock_t        tick = 0;
	size_t         tickrate = STD_TICKRATE;
	size_t         ts_since_sim = 9001; // ticks since last simulation
	size_t         win_w;
	size_t         win_h;
	struct World   world;
	size_t         world_w;
	size_t         world_h;

	if (!handle_args(argc, argv, &temperature, &tickrate)) {
		return 0;
	}

	tempws = term_get_size();
	win_w = tempws.ws_col;
	win_h = tempws.ws_row;

	world_w = win_w;
	world_h = win_h - 2;
	world = world_new(world_w, world_h, temperature);

	while (active) {
		now = clock();
		if (now - tick >= (long) (CLOCKS_PER_SEC / tickrate)) {
			tick = now;

			if (!paused) {
				if (ts_since_sim >= sim_subsample) {
					world_sim(&world);
					ts_since_sim = 0;
				} else {
					ts_since_sim++;
				}
			}

			tempws = term_get_size();
			win_w = tempws.ws_col;
			win_h = tempws.ws_row;
			draw(brush_mat,
			     cmdmode,
			     cursor_x, cursor_y,
			     "localhost",
			     sel_tool,
			     spawner_mat,
			     th_vision,
			     win_w,
			     win_h,
			     world,
			     "worldname",
			     world_w,
			     world_h);

			// TODO remove this
			active = 0;
		}
	}

	world_free(&world);

	return 0;
}
