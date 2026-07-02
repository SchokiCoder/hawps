/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <errno.h>
#include <hawps_core.h>
#include <hawps_extra.h>
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

#define CELSIUS_TO_KELVIN 273.15

#define FIRST_REAL_MAT MAT_SAND

#define SIG_INT  '\003'
#define SIG_TSTP '\032'

enum InputMode {
	IM_NORMAL,
	IM_COMMAND,
};

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
"Keybinds:\n"
"\n"
"    Currently, this program is configured at %s.\n"
"    It is static.\n"
"    Therefore you can't just change binds.\n"
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
     const char           *cmdline,
     const size_t          cmdline_len,
     const int             cursor_x,
     const int             cursor_y,
     char                 *display,
     const size_t          display_size,
     const char           *feedback,
     const enum InputMode  input_mode,
     const char           *ip_address,
     const bool            paused,
     const enum Tool       sel_tool,
     const int             sim_subsample,
     const enum Mat        spawner_mat,
     const int             tickrate,
     const bool            th_vision,
     const int             tool_x1,
     const int             tool_y1,
     const int             tool_x2,
     const int             tool_y2,
     const int             win_w,
     const int             win_h,
     const struct World    world,
     const char           *world_name);

struct Rgba
get_normal_dot_color(const struct World world,
                     const int          x,
                     const int          y);

struct Rgba
get_thermal_dot_color(const struct World world,
                      const int          x,
                      const int          y);

bool
handle_args(int     argc,
            char  **argv,
            float  *temperature,
            int    *tickrate);

void
handle_command(const char    *cmdline,
               bool          *active,
               int           *brush_radius,
               int           *eraser_radius,
               char         **feedback,
               clock_t       *feedback_expiration,
               clock_t        now,
               bool          *paused,
               enum Tool     *sel_tool,
               int           *sim_subsample,
               float         *temperature,
               bool          *th_vision,
               float         *thermo_delta,
               int           *thermo_radius,
               int           *tickrate,
               struct World  *world);

void
handle_command_input(const char      *in,
                     bool            *active,
                     int             *brush_radius,
                     char            *cmdline,
                     size_t          *cmdline_len,
                     int             *eraser_radius,
                     char           **feedback,
                     clock_t         *feedback_expiration,
                     enum InputMode  *input_mode,
                     clock_t          now,
                     bool            *paused,
                     enum Tool       *sel_tool,
                     int             *sim_subsample,
                     float           *temperature,
                     bool            *th_vision,
                     float           *thermo_delta,
                     int             *thermo_radius,
                     int             *tickrate,
                     struct World    *world);

void
handle_input(bool            *active,
             char            *cmdline,
             size_t          *cmdline_len,
             enum Mat        *brush_mat,
             int             *brush_radius,
             int             *cursor_x,
             int             *cursor_y,
             int             *eraser_radius,
             char           **feedback,
             clock_t         *feedback_expiration,
             enum InputMode  *input_mode,
             bool            *mouse_pressed,
             clock_t          now,
             bool            *paused,
             enum Tool       *sel_tool,
             int             *sim_subsample,
             enum Mat        *spawner_mat,
             float           *temperature,
             int             *tickrate,
             bool            *th_vision,
             float           *thermo_delta,
             int             *thermo_radius,
             struct World    *world);

void
handle_normal_csi_input(const char     *in,
                        const enum Mat  brush_mat,
                        int            *brush_radius,
                        int            *cursor_x,
                        int            *cursor_y,
                        int            *eraser_radius,
                        bool           *mouse_pressed,
                        const enum Tool sel_tool,
                        const enum Mat  spawner_mat,
                        const float     temperature,
                        const float     thermo_delta,
                        int            *thermo_radius,
                        struct World   *world);

void
handle_normal_input(const char     *in,
                    bool           *active,
                    enum Mat       *brush_mat,
                    int            *brush_radius,
                    int            *cursor_x,
                    int            *cursor_y,
                    int            *eraser_radius,
                    enum InputMode *input_mode,
                    bool           *mouse_pressed,
                    bool           *paused,
                    enum Tool      *sel_tool,
                    int            *sim_subsample,
                    enum Mat       *spawner_mat,
                    const float     temperature,
                    const int       tickrate,
                    bool           *th_vision,
                    const float    *thermo_delta,
                    int            *thermo_radius,
                    struct World   *world);

bool
int_flag_parse(int    argc,
               char **argv,
               int   *idx,
               long  *out);

void
tool_radius_add(const int        radius_change,
                int             *brush_radius,
                int             *eraser_radius,
                const enum Tool  sel_tool,
                int             *thermo_radius);

size_t
render_world(char               *out,
             const size_t        out_size,
             const bool          th_vision,
             const int           tool_x1,
             const int           tool_y1,
             const int           tool_x2,
             const int           tool_y2,
             const struct World  world,
             const int           world_draw_w,
             const int           world_draw_h);

void
use_tool(const enum Mat   brush_mat,
         const int        brush_radius,
         const int        cursor_x,
         const int        cursor_y,
         const int        eraser_radius,
         const enum Tool  sel_tool,
         const enum Mat   spawner_mat,
         const float      temperature,
         const float      thermo_delta,
         const int        thermo_radius,
         struct World    *world);

void
draw(const enum Mat        brush_mat,
     const char           *cmdline,
     const size_t          cmdline_len,
     const int             cursor_x,
     const int             cursor_y,
     char                 *display,
     const size_t          display_size,
     const char           *feedback,
     const enum InputMode  input_mode,
     const char           *ip_address,
     const bool            paused,
     const enum Tool       sel_tool,
     const int             sim_subsample,
     const enum Mat        spawner_mat,
     const int             tickrate,
     const bool            th_vision,
     const int             tool_x1,
     const int             tool_y1,
     const int             tool_x2,
     const int             tool_y2,
     const int             win_w,
     const int             win_h,
     const struct World    world,
     const char           *world_name)
{
	char   buf[BUF_SIZE];
	size_t buf_len = 0;
	size_t display_len = 0;
	float  sim_speed = 0.0;
	size_t space_len = 0;
	size_t st_bar_len = 0;
	char  *vision = NULL;
	int    world_draw_w = 0;
	int    world_draw_h = 0;

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

	display_len += render_world(&display[display_len],
	                            display_size - display_len,
	                            th_vision,
	                            tool_x1,
	                            tool_y1,
	                            tool_x2,
	                            tool_y2,
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

	if (paused) {
		sim_speed = 0.0;
	} else {
		sim_speed = (float) tickrate / (float) sim_subsample;
	}

	st_bar_len = display_len;
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
	                          " | Speed:");
	snprintf(buf, BUF_SIZE, "%.1f", sim_speed);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          buf);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          "/s | ");
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          ip_address);
	st_bar_len = display_len - st_bar_len;

	space_len = win_w - st_bar_len;
	memset(&display[display_len], ' ', space_len);
	display_len += space_len;

	switch (input_mode) {
	case IM_NORMAL:
		buf[0] = '\0';
		buf_len = 0;

		if (feedback != NULL) {
			display_len += string_cat(display,
			                          display_size,
			                          display_len,
			                          feedback);
			space_len = win_w - strlen(feedback);
			break;
		}

		buf_len += string_cat(buf, BUF_SIZE, buf_len, TOOL_NAME[sel_tool]);

		if (sel_tool == TOOL_BRUSH) {
			buf_len += string_cat(buf, BUF_SIZE, buf_len, " ");
			buf_len += string_cat(buf,
			                      BUF_SIZE,
			                      buf_len,
			                      MAT_NAME[brush_mat]);
		} else if (sel_tool == TOOL_SPAWNER) {
			buf_len += string_cat(buf, BUF_SIZE, buf_len, " ");
			buf_len += string_cat(buf,
			                      BUF_SIZE,
			                      buf_len,
			                      MAT_NAME[spawner_mat]);
		}

		display_len += string_cat(display, display_size, display_len, buf);
		space_len = win_w - buf_len;
		break;

	case IM_COMMAND:
		display[display_len] = ':';
		display_len += 1;

		display_len += string_cat(display,
		                          display_size,
		                          display_len,
		                          cmdline);

		display[display_len] = '<';
		display_len += 1;

		space_len = win_w - 1 - cmdline_len - 1;
		break;
	}
	memset(&display[display_len], ' ', space_len);
	display_len += space_len;

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

struct Rgba
get_normal_dot_color(const struct World world,
                     const int          x,
                     const int          y)
{
	struct Rgba a, b, ret;

	a = thermo_to_color(world.thermo[x][y]);

	b.r = MAT_R[world.dot[x][y]];
	b.g = MAT_G[world.dot[x][y]];
	b.b = MAT_B[world.dot[x][y]];
	b.a = 255;

	// TODO make blending a lib_extra function (blend the entire color struct)
	ret.r = ((a.r * a.a) + b.r * (255 - a.a)) >> 8;
	ret.g = ((a.g * a.a) + b.g * (255 - a.a)) >> 8;
	ret.b = ((a.b * a.a) + b.b * (255 - a.a)) >> 8;
	ret.a = 255;

	return ret;
}

struct Rgba
get_thermal_dot_color(const struct World world,
                      const int          x,
                      const int          y)
{
	struct Rgba ret;
	unsigned char vis_t;

	if (world.thermo[x][y] > (THERMAL_VISION_MIN_T + 255)) {
		vis_t = 255;
	} else if (world.thermo[x][y] < THERMAL_VISION_MIN_T) {
		vis_t = 0;
	} else {
		vis_t = world.thermo[x][y] - THERMAL_VISION_MIN_T;
	}

	ret.r = vis_t;
	ret.g = vis_t;
	ret.b = vis_t;
	ret.a = 255;

	return ret;
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
			       CONFIGURED_AT,
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
handle_command(const char    *cmdline,
               bool          *active,
               int           *brush_radius,
               int           *eraser_radius,
               char         **feedback,
               clock_t       *feedback_expiration,
               clock_t        now,
               bool          *paused,
               enum Tool     *sel_tool,
               int           *sim_subsample,
               float         *temperature,
               bool          *th_vision,
               float         *thermo_delta,
               int           *thermo_radius,
               int           *tickrate,
               struct World  *world)
{
	int x, y;

	*feedback = NULL;

	if (strcmp(cmdline, CMD_BRUSH) == 0 ||
	    strcmp(cmdline, CMD_BRUSH_SHORT) == 0) {
		*sel_tool = TOOL_BRUSH;
	} else if (strcmp(cmdline, CMD_CLEAR) == 0 ||
	           strcmp(cmdline, CMD_CLEAR_SHORT) == 0) {
		for (x = 0; x < world->w; x++) {
			for (y = 0; y < world->h; y++) {
				world_clear_dot(world, x, y);
			}
		}
	} else if (strcmp(cmdline, CMD_COOLER) == 0 ||
	           strcmp(cmdline, CMD_COOLER_SHORT) == 0) {
		*sel_tool = TOOL_COOLER;
	} else if (strcmp(cmdline, CMD_DEFAULTS) == 0 ||
	           strcmp(cmdline, CMD_DEFAULTS_SHORT) == 0) {
		*brush_radius = STD_BRUSH_RADIUS;
		*eraser_radius = STD_ERASER_RADIUS;
		*sel_tool = STD_SELECTED_TOOL;
		*sim_subsample = STD_SIM_SUBSAMPLE;
		*temperature = STD_TEMPERATURE;
		*thermo_delta = STD_THERMO_DELTA;
		*thermo_radius = STD_THERMO_RADIUS;
		*tickrate = STD_TICKRATE;
	} else if (strcmp(cmdline, CMD_ERASER) == 0 ||
	           strcmp(cmdline, CMD_ERASER_SHORT) == 0) {
		*sel_tool = TOOL_ERASER;
	} else if (strcmp(cmdline, CMD_HEATER) == 0 ||
	           strcmp(cmdline, CMD_HEATER_SHORT) == 0) {
		*sel_tool = TOOL_HEATER;
	} else if (strcmp(cmdline, CMD_NORMALVISION) == 0 ||
	           strcmp(cmdline, CMD_NORMALVISION_SHORT) == 0) {
		*th_vision = false;
	} else if (strcmp(cmdline, CMD_PAUSE) == 0 ||
	           strcmp(cmdline, CMD_PAUSE_SHORT) == 0) {
		if (*paused) {
			*paused = false;
		} else {
			*paused = true;
		}
	} else if (strcmp(cmdline, CMD_QUIT) == 0 ||
	           strcmp(cmdline, CMD_QUIT_SHORT) == 0 ||
	           strcmp(cmdline, "exit") == 0) {
		*active = false;
	} else if (strcmp(cmdline, CMD_SPAWNER) == 0 ||
	           strcmp(cmdline, CMD_SPAWNER_SHORT) == 0) {
		*sel_tool = TOOL_SPAWNER;
	} else if (strcmp(cmdline, CMD_THERMOVISION) == 0 ||
	           strcmp(cmdline, CMD_THERMOVISION_SHORT) == 0) {
		*th_vision = true;
	} else {
		*feedback = "Command not recognized.";
		*feedback_expiration = now + (CLOCKS_PER_SEC * FEEDBACK_LIFETIME);
	}
}

void
handle_command_input(const char      *in,
                     bool            *active,
                     int             *brush_radius,
                     char            *cmdline,
                     size_t          *cmdline_len,
                     int             *eraser_radius,
                     char           **feedback,
                     clock_t         *feedback_expiration,
                     enum InputMode  *input_mode,
                     clock_t          now,
                     bool            *paused,
                     enum Tool       *sel_tool,
                     int             *sim_subsample,
                     float           *temperature,
                     bool            *th_vision,
                     float           *thermo_delta,
                     int             *thermo_radius,
                     int             *tickrate,
                     struct World    *world)
{
	switch (in[0]) {
	case CHAR_BACKSPACE:
		if (*cmdline_len > 0) {
			cmdline[*cmdline_len - 1] = '\0';
			*cmdline_len -= 1;
		}
		break;

	case '\n':
		handle_command(cmdline,
		               active,
		               brush_radius,
		               eraser_radius,
		               feedback,
		               feedback_expiration,
		               now,
		               paused,
		               sel_tool,
		               sim_subsample,
		               temperature,
		               th_vision,
		               thermo_delta,
		               thermo_radius,
		               tickrate,
		               world);
		/* fallthrough */
	case SIG_INT:
	case SIG_TSTP:
		cmdline[0] = '\0';
		*cmdline_len = 0;
		*input_mode = IM_NORMAL;
		break;

	default:
		if (*cmdline_len < CMDLINE_SIZE - 1) {
			cmdline[*cmdline_len] = in[0];
			cmdline[*cmdline_len + 1] = '\0';
			*cmdline_len += 1;
		}
	}
}

void
handle_input(bool            *active,
             char            *cmdline,
             size_t          *cmdline_len,
             enum Mat        *brush_mat,
             int             *brush_radius,
             int             *cursor_x,
             int             *cursor_y,
             int             *eraser_radius,
             char           **feedback,
             clock_t         *feedback_expiration,
             enum InputMode  *input_mode,
             bool            *mouse_pressed,
             clock_t          now,
             bool            *paused,
             enum Tool       *sel_tool,
             int             *sim_subsample,
             enum Mat        *spawner_mat,
             float           *temperature,
             int             *tickrate,
             bool            *th_vision,
             float           *thermo_delta,
             int             *thermo_radius,
             struct World    *world)
{
	ssize_t input_len = 0;
	char    input[INPUT_SIZE];

	input_len = read(STDIN_FILENO, &input, INPUT_SIZE);

	switch (*input_mode) {
	case IM_NORMAL:
		if (input_len > 0 &&
		    input_len < INPUT_SIZE) {
			input[input_len] = '\0';
			handle_normal_input(input,
			                    active,
			                    brush_mat,
			                    brush_radius,
			                    cursor_x,
			                    cursor_y,
			                    eraser_radius,
			                    input_mode,
			                    mouse_pressed,
			                    paused,
			                    sel_tool,
			                    sim_subsample,
			                    spawner_mat,
			                    *temperature,
			                    *tickrate,
			                    th_vision,
			                    thermo_delta,
			                    thermo_radius,
			                    world);
		}
		break;

	case IM_COMMAND:
		if (input_len > 0 &&
		    input_len < 2) {
			handle_command_input(input,
			                     active,
			                     brush_radius,
			                     cmdline,
			                     cmdline_len,
			                     eraser_radius,
			                     feedback,
			                     feedback_expiration,
			                     input_mode,
			                     now,
			                     paused,
			                     sel_tool,
			                     sim_subsample,
			                     temperature,
			                     th_vision,
			                     thermo_delta,
			                     thermo_radius,
			                     tickrate,
			                     world);
		}
		break;
	}
}

void
handle_normal_csi_input(const char     *in,
                        const enum Mat  brush_mat,
                        int            *brush_radius,
                        int            *cursor_x,
                        int            *cursor_y,
                        int            *eraser_radius,
                        bool           *mouse_pressed,
                        const enum Tool sel_tool,
                        const enum Mat  spawner_mat,
                        const float     temperature,
                        const float     thermo_delta,
                        int            *thermo_radius,
                        struct World   *world)
{
	int   b;
	char  pressed;
	int   x;
	int   y;

	if (strcmp(in, CSI_KEY_LEFT) == 0) {
		if (*cursor_x > 0) {
			*cursor_x -= 1;
		}
	} else if (strcmp(in, CSI_KEY_DOWN) == 0) {
		if (*cursor_y < world->h - 1) {
			*cursor_y += 1;
		}
	} else if (strcmp(in, CSI_KEY_UP) == 0) {
		if (*cursor_y > 0) {
			*cursor_y -= 1;
		}
	} else if (strcmp(in, CSI_KEY_RIGHT) == 0) {
		if (*cursor_x < world->w - 1) {
			*cursor_x += 1;
		}
	} else if (strcmp(in, CSI_KEY_HOME) == 0) {
		*cursor_x = 0;
	} else if (strcmp(in, CSI_KEY_END) == 0) {
		*cursor_x = world->w - 1;
	} else if (strcmp(in, CSI_KEY_PGUP) == 0) {
		*cursor_y = 0;
	} else if (strcmp(in, CSI_KEY_PGDOWN) == 0) {
		*cursor_y = world->h - 1;
	} else if (strcmp(in, CSI_KEY_CTRLHOME) == 0) {
		*cursor_x = 0;
		*cursor_y = 0;
	} else if (strcmp(in, CSI_KEY_CTRLEND) == 0) {
		*cursor_x = world->w - 1;
		*cursor_y = world->h - 1;
	} else if (in[1] == '[' &&
	           in[2] == '<') {
		/* mouse device reporting */
		sscanf(in, CSI_ESCAPE "[<%i;%i;%i%c", &b, &x, &y, &pressed);
		x -= 1;
		y -= 1;

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
				 sel_tool,
				 spawner_mat,
				 temperature,
				 thermo_delta,
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
			tool_radius_add(1,
				        brush_radius,
				        eraser_radius,
				        sel_tool,
				        thermo_radius);
			break;

		case CSI_MB_WHEELDOWN:
			tool_radius_add(-1,
				        brush_radius,
				        eraser_radius,
				        sel_tool,
				        thermo_radius);
			break;
		}
	}
}

void
handle_normal_input(const char     *in,
                    bool           *active,
                    enum Mat       *brush_mat,
                    int            *brush_radius,
                    int            *cursor_x,
                    int            *cursor_y,
                    int            *eraser_radius,
                    enum InputMode *input_mode,
                    bool           *mouse_pressed,
                    bool           *paused,
                    enum Tool      *sel_tool,
                    int            *sim_subsample,
                    enum Mat       *spawner_mat,
                    const float     temperature,
                    const int       tickrate,
                    bool           *th_vision,
                    const float    *thermo_delta,
                    int            *thermo_radius,
                    struct World   *world)
{
	switch (in[0]) {
	case KEY_QUIT:
		*active = false;
		break;

	case KEY_USE:
		use_tool(*brush_mat,
		         *brush_radius,
		         *cursor_x,
		         *cursor_y,
		         *eraser_radius,
		         *sel_tool,
		         *spawner_mat,
		         temperature,
		         *thermo_delta,
		         *thermo_radius,
		         world);
		break;

	case KEY_SWITCH_VISION:
		if (*th_vision)
			*th_vision = false;
		else
			*th_vision = true;
		break;

	case KEY_PREVIOUS_MAT:
		switch (*sel_tool) {
		case TOOL_BRUSH:
			if (*brush_mat > FIRST_REAL_MAT) {
				*brush_mat -= 1;
			}
			break;

		case TOOL_SPAWNER:
			if (*spawner_mat > MAT_NONE) {
				*spawner_mat -= 1;
			}
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_NEXT_MAT:
		switch (*sel_tool) {
		case TOOL_BRUSH:
			if (*brush_mat < MAT_COUNT - 1) {
				*brush_mat += 1;
			}
			break;

		case TOOL_SPAWNER:
			if (*spawner_mat < MAT_COUNT - 1) {
				*spawner_mat += 1;
			}
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
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

	case KEY_SUPERLEFT:
		*cursor_x = 0;
		break;

	case KEY_DOWN:
		if (*cursor_y < world->h - 1)
			*cursor_y += 1;
		break;

	case KEY_SUPERDOWN:
		*cursor_y = world->h - 1;
		break;

	case KEY_UP:
		if (*cursor_y > 0)
			*cursor_y -= 1;
		break;

	case KEY_SUPERUP:
		*cursor_y = 0;
		break;

	case KEY_RIGHT:
		if (*cursor_x < world->w - 1)
			*cursor_x += 1;
		break;

	case KEY_SUPERRIGHT:
		*cursor_x = world->w - 1;
		break;

	case KEY_RADIUS_DOWN:
		tool_radius_add(-1,
		                brush_radius,
		                eraser_radius,
		                *sel_tool,
		                thermo_radius);
		break;

	case KEY_RADIUS_UP:
		tool_radius_add(1,
		                brush_radius,
		                eraser_radius,
		                *sel_tool,
		                thermo_radius);
		break;

	case '-':
	case KEY_SIMSPEED_DOWN:
		if (*sim_subsample < tickrate) {
			*sim_subsample *= 2;
		}
		break;

	case '+':
	case KEY_SIMSPEED_UP:
		if (*sim_subsample > 1) {
			*sim_subsample /= 2;
		}
		break;

	case KEY_CMD:
		*input_mode = IM_COMMAND;
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

	case CHAR_ESCAPE:
		handle_normal_csi_input(in,
		                        *brush_mat,
		                        brush_radius,
		                        cursor_x,
		                        cursor_y,
		                        eraser_radius,
		                        mouse_pressed,
		                        *sel_tool,
		                        *spawner_mat,
		                        temperature,
		                        *thermo_delta,
		                        thermo_radius,
		                        world);
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

void
tool_radius_add(const int        radius_change,
                int             *brush_radius,
                int             *eraser_radius,
                const enum Tool  sel_tool,
                int             *thermo_radius)
{
	int  *target = NULL;

	switch (sel_tool) {
	case TOOL_BRUSH:
		target = brush_radius;
		break;

	case TOOL_SPAWNER:
		return;
		break;

	case TOOL_ERASER:
		target = eraser_radius;
		break;

	case TOOL_HEATER:
	case TOOL_COOLER:
		target = thermo_radius;
		break;

	case TOOL_COUNT:
		break;
	}

	*target += radius_change;
	if (*target < 0) {
		*target = 0;
	} else if (*target > MAX_RADIUS) {
		*target = MAX_RADIUS;
	}
}

size_t
render_world(char               *out,
             const size_t        out_size,
             const bool          th_vision,
             const int           tool_x1,
             const int           tool_y1,
             const int           tool_x2,
             const int           tool_y2,
             const struct World  world,
             const int           world_draw_w,
             const int           world_draw_h)
{
	struct Rgba dot_color;
	struct Rgba (*get_dot_color)(const struct World,
	                             const int    x,
	                             const int    y);
	size_t out_len = 0;
	int    x, y;

	if (th_vision) {
		get_dot_color = get_thermal_dot_color;
	} else {
		get_dot_color = get_normal_dot_color;
	}

	for (y = 0; y < world_draw_h; y++) {
		for (x = 0; x < world_draw_w; x++) {
			if (world.spawner[x][y] == true) {
				out_len += CSI_color_to_string(SPAWNER_R,
				                               SPAWNER_G,
				                               SPAWNER_B,
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

			dot_color = get_dot_color(world, x, y);
			out_len += CSI_color_to_string(dot_color.r,
			                               dot_color.g,
			                               dot_color.b,
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

	for (x = tool_x1; x < tool_x2; x++) {
		for (y = tool_y1; y < tool_y2; y++) {
			out[((y * world.w) + x + 1) *
			    (CSI_COLORSTRING_LEN + 1) -
			    1] = '^';
		}
	}
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
         const float      thermo_delta,
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
		                 thermo_delta,
		                 cursor_x,
		                 cursor_y,
		                 thermo_radius);
		break;

	case TOOL_COOLER:
		world_use_cooler(world,
		                 thermo_delta,
		                 cursor_x,
		                 cursor_y,
		                 thermo_radius);
		break;

	case TOOL_COUNT:
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
	char           cmdline[CMDLINE_SIZE];
	size_t         cmdline_len = 0;
	int            cursor_x = 0;
	int            cursor_y = 0;
	char          *display = NULL;
	size_t         display_size = 0;
	int            eraser_radius = STD_ERASER_RADIUS;
	char          *feedback = NULL;
	clock_t        feedback_expiration = 0;
	enum InputMode input_mode = IM_NORMAL;
	bool           paused = false;
	clock_t        last_tick = 0;
	bool           mouse_pressed = false;
	clock_t        now;
	enum Tool      sel_tool = STD_SELECTED_TOOL;
	int            sim_subsample = STD_SIM_SUBSAMPLE;
	enum Mat       spawner_mat = FIRST_REAL_MAT;
	float          temperature = STD_TEMPERATURE;
	struct winsize tempws;
	bool           th_vision = false;
	float          thermo_delta = STD_THERMO_DELTA;
	int            thermo_radius = STD_THERMO_RADIUS;
	int            tickrate = STD_TICKRATE;
	int            ts_since_sim = 9001; /* ticks since last simulation */
	int            tool_radius = STD_BRUSH_RADIUS;
	int            tool_x1 = 0;
	int            tool_y1 = 0;
	int            tool_x2 = 0;
	int            tool_y2 = 0;
	int            win_w;
	int            win_h;
	struct World   world;

	if (!handle_args(argc, argv, &temperature, &tickrate)) {
		return 0;
	}

	hawps_core_init();
	hawps_extra_init();

	CSI_set_raw();
	fputs(CSI_CLEAR, stdout);

	tempws = CSI_get_size();
	win_w = tempws.ws_col;
	win_h = tempws.ws_row;

	cmdline[0] = '\0';

	display_size = win_w * win_h * (CSI_COLORSTRING_LEN + 1);
	display = malloc(display_size);

	world = world_new(win_w, win_h - 2, temperature);

	while (active) {
		handle_input(&active,
		             cmdline,
		             &cmdline_len,
		             &brush_mat,
		             &brush_radius,
		             &cursor_x,
		             &cursor_y,
		             &eraser_radius,
		             &feedback,
		             &feedback_expiration,
		             &input_mode,
		             &mouse_pressed,
		             now,
		             &paused,
		             &sel_tool,
		             &sim_subsample,
		             &spawner_mat,
		             &temperature,
		             &tickrate,
		             &th_vision,
		             &thermo_delta,
		             &thermo_radius,
		             &world);

		switch (sel_tool) {
		case TOOL_BRUSH:
			tool_radius = brush_radius;
			break;

		case TOOL_SPAWNER:
			tool_radius = 0;
			break;

		case TOOL_ERASER:
			tool_radius = eraser_radius;
			break;

		case TOOL_HEATER:
		case TOOL_COOLER:
			tool_radius = thermo_radius;
			break;

		case TOOL_COUNT:
			break;
		}

		tool_x1 = cursor_x - tool_radius;
		if (tool_x1 < 0)
			tool_x1 = 0;

		tool_y1 = cursor_y - tool_radius;
		if (tool_y1 < 0)
			tool_y1 = 0;

		tool_x2 = cursor_x + tool_radius + 1;
		if (tool_x2 >= world.w)
			tool_x2 = world.w;

		tool_y2 = cursor_y + tool_radius + 1;
		if (tool_y2 >= world.h)
			tool_y2 = world.h;

		now = clock();
		if (now - last_tick >= (long) (CLOCKS_PER_SEC / tickrate)) {
			last_tick = now;

			if (now > feedback_expiration) {
				feedback = NULL;
			}

			if (mouse_pressed) {
				use_tool(brush_mat,
					 brush_radius,
					 cursor_x,
					 cursor_y,
					 eraser_radius,
					 sel_tool,
					 spawner_mat,
					 temperature,
					 thermo_delta,
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
			     cmdline,
			     cmdline_len,
			     cursor_x,
			     cursor_y,
			     display,
			     display_size,
			     feedback,
			     input_mode,
			     "localhost", // TODO implement actual backend
			     paused,
			     sel_tool,
			     sim_subsample,
			     spawner_mat,
			     tickrate,
			     th_vision,
			     tool_x1,
			     tool_y1,
			     tool_x2,
			     tool_y2,
			     win_w,
			     win_h,
			     world,
			     "worldname"); // TODO implement actual backend
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
