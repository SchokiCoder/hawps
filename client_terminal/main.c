/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <errno.h>
#include <hawps_core.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "csi.h"
#include "config.h"
#include "str.h"

enum Tool {
	TOOL_BRUSH,
	TOOL_SPAWNER,
	TOOL_ERASER,
	TOOL_HEATER,
	TOOL_COOLER,
};

#define CELSIUS_TO_KELVIN 273.15

#define FIRST_REAL_MAT MAT_SAND

#define KEY_BACKSPACE '\x7f'
#define KEY_HOME      '\x1b[H'
#define KEY_INSERT    '\033[2~'
#define KEY_DELETE    '\033[3~'
#define KEY_PGUP      '\033[5~'
#define KEY_PGDOWN    '\033[6~'
#define KEY_END       '\x1b[F'

#define SIG_INT  '\003'
#define SIG_TSTP '\032'

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
     char                 *display,
     const size_t          display_size,
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
             int            *brush_radius,
             int            *cursor_x,
             int            *cursor_y,
             int            *eraser_radius,
             bool           *mouse_pressed,
             bool           *paused,
             enum Tool      *sel_tool,
             const enum Mat  spawner_mat,
             const float     temperature,
             bool           *th_vision,
             int            *thermo_radius,
             struct World   *world);

bool
int_flag_parse(int    argc,
               char **argv,
               int   *idx,
               long  *out);

size_t
render_world(const int           cursor_x,
             const int           cursor_y,
             char               *out,
             const size_t        out_size,
             const struct World  world,
             const size_t        world_draw_w,
             const size_t        world_draw_h);

void
use_tool(const enum Mat   brush_mat,
         const int        brush_radius,
         const int        cursor_x,
         const int        cursor_y,
         const int        eraser_radius,
         const enum Tool  sel_tool,
         const enum Mat   spawner_mat,
         const float      temperature,
         const int        thermo_radius,
         struct World    *world);

void
draw(const enum Mat        brush_mat,
     const bool            cmdmode,
     const int             cursor_x,
     const int             cursor_y,
     char                 *display,
     const size_t          display_size,
     const char           *ip_address,
     const enum Tool       sel_tool,
     const enum Mat        spawner_mat,
     const bool            th_vision,
     const int             win_w,
     const int             win_h,
     const struct World    world,
     const char           *world_name)
{
	char   buf[BUF_SIZE];
	size_t buf_len = 0;
	size_t display_len = 0;
	size_t left_st_bar_len = 0;
	size_t right_st_bar_len = 0;
	size_t space_len = 0;
	char  *vision = NULL;
	size_t world_draw_w = 0;
	size_t world_draw_h = 0;

	display[0] = '\0';

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

	if (th_vision) {
		display_len += CSI_color_to_string(THERMAL_VISION_R,
		                                   THERMAL_VISION_G,
		                                   THERMAL_VISION_B,
		                                   false,
		                                   &display[display_len],
		                                   display_size - display_len);
	}

	display_len += render_world(cursor_x,
	                            cursor_y,
	                            &display[display_len],
	                            display_size - display_len,
	                            world,
	                            world_draw_w,
	                            world_draw_h);

	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          CSI_FG_DEFAULT);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          CSI_BG_DEFAULT);

	if (th_vision) {
		vision = "Thermal";
	} else {
		vision = "Normal";
	}

	left_st_bar_len = display_len;
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          world_name);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          " (");
	snprintf(buf, BUF_SIZE, "%i", cursor_x);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          buf);
	display[display_len] = ',';
	display_len += 1;
	snprintf(buf, BUF_SIZE, "%i", cursor_y);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          buf);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          ") | View:");
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          vision);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          " | ");
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          ip_address);
	left_st_bar_len = display_len - left_st_bar_len;

	buf[0] = '\0';
	buf_len = 0;
	buf_len += string_cat(buf,
	                      BUF_SIZE,
	                      buf_len,
	                      "bind1");
	buf_len += string_cat(buf,
	                      BUF_SIZE,
	                      buf_len,
	                      ": bindings, ");
	buf_len += string_cat(buf,
	                      BUF_SIZE,
	                      buf_len,
	                      "bind2");
	buf_len += string_cat(buf,
	                      BUF_SIZE,
	                      buf_len,
	                      ": help");
	right_st_bar_len = buf_len;

	space_len = win_w - (left_st_bar_len + right_st_bar_len);
	memset(&display[display_len], ' ', space_len);
	display_len += space_len;

	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          buf);

	if (cmdmode) {
		// TODO implement
		display_len += string_cat(display,
		                          display_size,
		                          display_len,
		                          ":cmd input currently not implemented");
		// TODO draw ' ' until row end
	} else {
		buf[0] = '\0';
		buf_len = 0;

		switch (sel_tool) {
		case TOOL_BRUSH:
			buf_len += string_cat(buf, BUF_SIZE, buf_len, "BRUSH ");
			buf_len += string_cat(buf,
			                      BUF_SIZE,
			                      buf_len,
			                      MAT_NAME[brush_mat]);
			break;

		case TOOL_SPAWNER:
			buf_len += string_cat(buf, BUF_SIZE, buf_len, "SPAWNER ");
			buf_len += string_cat(buf,
			                      BUF_SIZE,
			                      buf_len,
			                      MAT_NAME[spawner_mat]);
			break;

		case TOOL_ERASER:
			buf_len += string_cat(buf, BUF_SIZE, buf_len, "ERASER");
			break;

		case TOOL_HEATER:
			buf_len += string_cat(buf, BUF_SIZE, buf_len, "HEATER");
			break;

		case TOOL_COOLER:
			buf_len += string_cat(buf, BUF_SIZE, buf_len, "COOLER");
			break;
		}

		display_len += string_cat(display, display_size, display_len, buf);
		space_len = win_w - buf_len;
		memset(&display[display_len], ' ', space_len);
		display_len += space_len;
	}

	display[display_len] = '\0';

	CSI_set_cursorpos(0, 0);
	buf_len = 0;
	while (buf_len < display_len) {
		buf_len += fwrite(&display[buf_len],
		                  1,
		                  display_len - buf_len,
		                  stdout);
	}
}

bool
handle_args(int     argc,
            char  **argv,
            float  *temperature,
            int    *tickrate)
{
	int i;
	long l;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-a") == 0 ||
		    strcmp(argv[i], "-about") == 0) {
			printf(APP_ABOUT,
			       APP_NAME_FORMAL, APP_NAME, APP_VERSION,
			       APP_LICENSE,
			       APP_REPOSITORY,
			       APP_LICENSE_URL);
			return false;
		} else if (strcmp(argv[i], "-h") == 0 ||
		           strcmp(argv[i], "-help") == 0) {
			printf(APP_HELP,
			       APP_NAME,
			       CELSIUS_TO_KELVIN,
			       STD_TEMPERATURE,
			       STD_TICKRATE,
			       (float) STD_TICKRATE / (float) STD_SIM_SUBSAMPLE,
			       THERMAL_VISION_MIN_T - CELSIUS_TO_KELVIN,
			       THERMAL_VISION_MIN_T - CELSIUS_TO_KELVIN + 255);
			return false;
		} else if (strcmp(argv[i], "-temperature") == 0) {
			if (!int_flag_parse(argc, argv, &i, &l)) {
				return false;
			}
			*temperature = l;
			if (*temperature < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], "-tickrate") == 0) {
			if (!int_flag_parse(argc, argv, &i, &l)) {
				return false;
			}
			*tickrate = l;
			i++;
		} else if (strcmp(argv[i], "-v") == 0 ||
		           strcmp(argv[i],  "-version") == 0) {
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
             int            *brush_radius,
             int            *cursor_x,
             int            *cursor_y,
             int            *eraser_radius,
             bool           *mouse_pressed,
             bool           *paused,
             enum Tool      *sel_tool,
             const enum Mat  spawner_mat,
             const float     temperature,
             bool           *th_vision,
             int            *thermo_radius,
             struct World   *world)
{
	int   b;
	char  in[INPUT_SIZE];
	char  pressed;
	int  *target;
	int   x;
	int   y;

	if (read(STDIN_FILENO, &in, INPUT_SIZE) <= 0) {
		return;
	}

	switch (in[0]) {
	case KEY_QUIT:
		*active = false;
		break;

	case KEY_USE:
		use_tool(brush_mat,
		         *brush_radius,
		         *cursor_x,
		         *cursor_y,
		         *eraser_radius,
		         *sel_tool,
		         spawner_mat,
		         temperature,
		         *thermo_radius,
		         world);
		break;

	case KEY_SWITCH_VISION:
		if (*th_vision)
			*th_vision = false;
		else
			*th_vision = true;
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

	case SIG_INT:
	case SIG_TSTP:
		*active = false;
		break;

	default:
		/* mouse device reporting */
		if (in[0] != 27 ||
		    in[1] != 91 ||
		    in[2] != 60) {
			break;
		}

		sscanf(in, "\033[<%i;%i;%i%c", &b, &x, &y, &pressed);

		switch (b) {
		case CSI_MB_LEFT:
		case CSI_MB_LEFT_DRAG:
			*cursor_x = x;
			*cursor_y = y;
			use_tool(brush_mat,
			         *brush_radius,
			         *cursor_x,
			         *cursor_y,
			         *eraser_radius,
			         *sel_tool,
			         spawner_mat,
			         temperature,
			         *thermo_radius,
			         world);

			if ('M' == pressed) {
				*mouse_pressed = true;
			} else {
				*mouse_pressed = false;
			}
			break;

		case CSI_MB_HOVER:
			*cursor_x = x;
			*cursor_y = y;
			*mouse_pressed = false;
			break;

		case CSI_MB_MIDDLE:
		case CSI_MB_MIDDLE_DRAG:
		case CSI_MB_RIGHT:
		case CSI_MB_RIGHT_DRAG:
			break;

		case CSI_MB_WHEELUP:
			switch (*sel_tool) {
			case TOOL_BRUSH:
				target = brush_radius;
				break;

			case TOOL_SPAWNER:
				break;

			case TOOL_ERASER:
				target = eraser_radius;
				break;

			case TOOL_HEATER:
			case TOOL_COOLER:
				target = thermo_radius;
				break;
			}

			*target += 1;
			if (*target > MAX_RADIUS) {
				*target = MAX_RADIUS;
			}
			break;

		case CSI_MB_WHEELDOWN:
			switch (*sel_tool) {
			case TOOL_BRUSH:
				target = brush_radius;
				break;

			case TOOL_SPAWNER:
				break;

			case TOOL_ERASER:
				target = eraser_radius;
				break;

			case TOOL_HEATER:
			case TOOL_COOLER:
				target = thermo_radius;
				break;
			}

			*target -= 1;
			if (*target < 0) {
				*target = 0;
			}
			break;
		}
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

size_t
render_world(const int           cursor_x,
             const int           cursor_y,
             char               *out,
             const size_t        out_size,
             const struct World  world,
             const size_t        world_draw_w,
             const size_t        world_draw_h)
{
	size_t out_len = 0;
	size_t x, y;

	for (y = 0; y < world_draw_h; y++) {
		for (x = 0; x < world_draw_w; x++) {
			if (world.spawner[x][y] == true) {
				out_len += CSI_color_to_string(COLOR_SPAWNER_R,
				                               COLOR_SPAWNER_G,
				                               COLOR_SPAWNER_B,
				                               true,
				                               &out[out_len],
				                               out_size - out_len);
				out[out_len] = 'O';
				out_len += 1;
				continue;
			}

			if (world.dot[x][y] == MAT_NONE) {
				out_len += CSI_color_to_string(255, 255, 255,
				                               true,
				                               &out[out_len],
				                               out_size - out_len);
				out[out_len] = ' ';
				out_len += 1;
				continue;
			}

			out_len += CSI_color_to_string(255, 255, 255,
			                               true,
			                               &out[out_len],
			                               out_size - out_len);
			switch (world.state[x][y]) {
			case MS_STATIC:
			case MS_GRAIN:
				out[out_len] = 'X';
				out_len += 1;
				break;

			case MS_LIQUID:
				out[out_len] = '+';
				out_len += 1;
				break;

			case MS_GAS:
				out[out_len] = '-';
				out_len += 1;
				break;

			default:
				out[out_len] = '?';
				out_len += 1;
				break;
			}
		}
	}
	out[((cursor_y * world.w) + cursor_x + 1) *
	    (CSI_COLORSTRING_LEN + 1) -
	    1] = '^';

	out[out_len] = '\0';

	return out_len;
}

void
use_tool(const enum Mat   brush_mat,
         const int        brush_radius,
         const int        cursor_x,
         const int        cursor_y,
         const int        eraser_radius,
         const enum Tool  sel_tool,
         const enum Mat   spawner_mat,
         const float      temperature,
         const int        thermo_radius,
         struct World    *world)
{
	switch (sel_tool) {
	case TOOL_BRUSH:
		world_use_brush(world,
		                brush_mat,
		                temperature,
		                cursor_x,
		                cursor_y,
		                brush_radius);
		break;

	case TOOL_SPAWNER:
		world->spawner[cursor_x][cursor_y] = true;
		world->spawner_mat[cursor_x][cursor_y] = spawner_mat;
		break;

	case TOOL_ERASER:
		world_use_eraser(world,
		                 cursor_x,
		                 cursor_y,
		                 eraser_radius);
		break;

	case TOOL_HEATER:
		world_use_heater(world,
		                 STD_THERMO_DELTA,
		                 cursor_x,
		                 cursor_y,
		                 thermo_radius);
		break;

	case TOOL_COOLER:
		world_use_cooler(world,
		                 STD_THERMO_DELTA,
		                 cursor_x,
		                 cursor_y,
		                 thermo_radius);
		break;
	}
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
	char          *display = NULL;
	size_t         display_size = 0;
	int            eraser_radius = STD_ERASER_RADIUS;
	bool           paused = false;
	bool           mouse_pressed = false;
	clock_t        now;
	enum Tool      sel_tool = STD_SELECTED_TOOL;
	int            sim_subsample = STD_SIM_SUBSAMPLE;
	enum Mat       spawner_mat = FIRST_REAL_MAT;
	float          temperature = STD_TEMPERATURE;
	struct winsize tempws;
	bool           th_vision = false;
	int            thermo_radius = STD_THERMO_RADIUS;
	clock_t        last_tick = 0;
	int            tickrate = STD_TICKRATE;
	int            ts_since_sim = 9001; /* ticks since last simulation */
	int            win_w;
	int            win_h;
	struct World   world;

	if (!handle_args(argc, argv, &temperature, &tickrate)) {
		return 0;
	}

	hawps_core_init();

	CSI_set_raw();
	fputs(CSI_CLEAR, stdout);

	tempws = CSI_get_size();
	win_w = tempws.ws_col;
	win_h = tempws.ws_row;

	display_size = win_w * win_h * (CSI_COLORSTRING_LEN + 1);
	display = malloc(display_size);

	world = world_new(win_w, win_h - 2, temperature);

	while (active) {
		handle_input(&active,
		             brush_mat,
		             &brush_radius,
		             &cursor_x,
		             &cursor_y,
		             &eraser_radius,
		             &mouse_pressed,
		             &paused,
		             &sel_tool,
		             spawner_mat,
		             temperature,
		             &th_vision,
		             &thermo_radius,
		             &world);

		now = clock();
		if (now - last_tick >= (long) (CLOCKS_PER_SEC / tickrate)) {
			last_tick = now;

			if (mouse_pressed) {
				use_tool(brush_mat,
					 brush_radius,
					 cursor_x,
					 cursor_y,
					 eraser_radius,
					 sel_tool,
					 spawner_mat,
					 temperature,
					 thermo_radius,
					 &world);
			}

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
			if (win_w != tempws.ws_col ||
			    win_h != tempws.ws_row) {
				win_w = tempws.ws_col;
				win_h = tempws.ws_row;

				free(display);
				display_size = win_w *
				               win_h *
				               (CSI_COLORSTRING_LEN + 1);
				display = malloc(display_size);
			}

			draw(brush_mat,
			     cmdmode,
			     cursor_x, cursor_y,
			     display,
			     display_size,
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
	fputs(CSI_CLEAR, stdout);
	fputs(CSI_FG_DEFAULT, stdout);
	fputs(CSI_BG_DEFAULT, stdout);
	world_free(&world);
	if (display != NULL) {
		free(display);
	}

	return 0;
}
