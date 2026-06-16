/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "core/core.h"
#include "csi/csi.h"
#include "config.h"

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

#define THERMAL_VISION_MIN_T (-75.0 + CELSIUS_TO_KELVIN)

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
     const int             win_w,
     const int             win_h,
     const struct World    world,
     const char           *world_name);

bool
handle_args(int     argc,
            char  **argv,
            float  *temperature,
            int    *tickrate);

void
handle_input(bool           *active,
             const enum Mat  brush_mat,
             const int       brush_radius,
             int            *cursor_x,
             int            *cursor_y,
             const int       eraser_radius,
             bool           *paused,
             enum Tool      *sel_tool,
             const enum Mat  spawner_mat,
             const float     temperature,
             const int       thermo_radius,
             struct World   *world);

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
     const int             win_w,
     const int             win_h,
     const struct World    world,
     const char           *world_name)
{
	char left_st_bar[LEFT_ST_BAR_SIZE];
	char right_st_bar[RIGHT_ST_BAR_SIZE];
	char st_bar[ST_BAR_SIZE];
	char vision[VISION_SIZE];
	int  world_draw_w;
	int  world_draw_h;
	int  x, y;

	if (world.w > win_w) {
		world_draw_w = win_w;
	} else {
		world_draw_w = world.w;
	}
	if (world.h > win_h - 2) {
		world_draw_h = win_h - 2;
	} else {
		world_draw_h = world.h;
	}

	CSI_set_cursorpos(0, 0);
	for (y = 0; y < world_draw_h; y++) {
		for (x = 0; x < world_draw_w; x++) {
			if (world.dot[x][y] == MAT_NONE) {
				fputs(" ", stdout);
				continue;
			}

			switch (world.state[x][y]) {
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

	CSI_set_cursorpos(cursor_x, cursor_y);
	fputs("^", stdout);
}

bool
handle_args(int     argc,
            char  **argv,
            float  *temperature,
            int    *tickrate)
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

void
handle_input(bool           *active,
             const enum Mat  brush_mat,
             const int       brush_radius,
             int            *cursor_x,
             int            *cursor_y,
             const int       eraser_radius,
             bool           *paused,
             enum Tool      *sel_tool,
             const enum Mat  spawner_mat,
             const float     temperature,
             const int       thermo_radius,
             struct World   *world)
{
	char in;

	if (read(STDIN_FILENO, &in, sizeof(in)) <= 0) {
		return;
	}

	switch (in) {
	case KEY_QUIT:
		*active = 0;
		break;

	case KEY_USE:
		switch (*sel_tool) {
		case TOOL_BRUSH:
			world_use_brush(world,
			                brush_mat,
			                temperature,
			                *cursor_x,
			                *cursor_y,
			                brush_radius);
			break;

		case TOOL_SPAWNER:
			world->spawner[*cursor_x][*cursor_y] = true;
			world->spawner_mat[*cursor_x][*cursor_y] = spawner_mat;
			break;

		case TOOL_ERASER:
			world_use_eraser(world,
			                 *cursor_x,
			                 *cursor_y,
			                 eraser_radius);
			break;

		case TOOL_HEATER:
			world_use_heater(world,
			                 STD_THERMO_DELTA,
			                 *cursor_x,
			                 *cursor_y,
			                 thermo_radius);
			break;

		case TOOL_COOLER:
			world_use_cooler(world,
			                 STD_THERMO_DELTA,
			                 *cursor_x,
			                 *cursor_y,
			                 thermo_radius);
			break;
		}
		break;

	case KEY_BRUSH:
		*sel_tool = TOOL_BRUSH;
		break;

	case KEY_SPAWNER:
		*sel_tool = TOOL_SPAWNER;
		break;

	case KEY_ERASER:
		*sel_tool = TOOL_ERASER;
		break;

	case KEY_HEATER:
		*sel_tool = TOOL_HEATER;
		break;

	case KEY_COOLER:
		*sel_tool = TOOL_COOLER;
		break;

	case KEY_LEFT:
		if (*cursor_x > 0)
			*cursor_x -= 1;
		break;

	case KEY_DOWN:
		if (*cursor_y < world->h - 1)
			*cursor_y += 1;
		break;

	case KEY_UP:
		if (*cursor_y > 0)
			*cursor_y -= 1;
		break;

	case KEY_RIGHT:
		if (*cursor_x < world->w - 1)
			*cursor_x += 1;
		break;

	case KEY_CMD:
		// TODO *imode = IM_CMD;
		break;

	case KEY_PAUSE:
		if (*paused)
			*paused = false;
		else
			*paused = true;
		break;

	case SIGINT:
	case SIGTSTP:
		*active = 0;
		break;
	}
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
	int            sim_subsample = STD_SIM_SUBSAMPLE;
	enum Mat       spawner_mat = FIRST_REAL_MAT;
	float          temperature = STD_TEMPERATURE;
	struct winsize tempws;
	int            thermo_radius = STD_THERMO_RADIUS;
	bool           th_vision = false;
	clock_t        last_tick = 0;
	int            tickrate = STD_TICKRATE;
	int            ts_since_sim = 9001; // ticks since last simulation
	int            win_w;
	int            win_h;
	struct World   world;

	if (!handle_args(argc, argv, &temperature, &tickrate)) {
		return 0;
	}

	core_init();

	CSI_set_raw();

	tempws = CSI_get_size();
	win_w = tempws.ws_col;
	win_h = tempws.ws_row;

	world = world_new(win_w, win_h - 2, temperature);

	while (active) {
		now = clock();
		if (now - last_tick >= (long) (CLOCKS_PER_SEC / tickrate)) {
			last_tick = now;

			handle_input(&active,
			             brush_mat,
			             brush_radius,
			             &cursor_x,
			             &cursor_y,
			             eraser_radius,
			             &paused,
			             &sel_tool,
			             spawner_mat,
			             temperature,
			             thermo_radius,
			             &world);

			world_update(&world, temperature);
			if (!paused) {
				if (ts_since_sim >= sim_subsample) {
					world_sim(&world);
					ts_since_sim = 0;
				} else {
					ts_since_sim += 1;
				}
			}

			tempws = CSI_get_size();
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
			     "worldname");
		}
	}

	CSI_set_normal();
	world_free(&world);

	return 0;
}
