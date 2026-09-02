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

#include "config.h"
#include "generic.h"
#include "int_to_string.h"
#include "str.h"
#include "types.h"

#ifdef SDL_BACKEND
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "backend_sdl.h"
#else
#include "csi.h"
#include "backend_terminal.h"
#endif

/* Macros
 */

/* Constant defines
 */

#define FIRST_REAL_MAT MAT_SAND

#define FLAG_ABOUT                  "-about"
#define FLAG_ABOUT_SHORT            "-a"
#define FLAG_BRUSHRADIUS            "-brushradius"
#define FLAG_BRUSHRADIUS_SHORT      "-br"
#define FLAG_ERASERRADIUS           "-eraserradius"
#define FLAG_ERASERRADIUS_SHORT     "-er"
#define FLAG_FRAMERATE              "-framerate"
#define FLAG_FRAMERATE_SHORT        "-fr"
#define FLAG_HELP                   "-help"
#define FLAG_HELP_SHORT             "-h"
#define FLAG_SPAWNTEMPERATURE       "-spawntemperature"
#define FLAG_SPAWNTEMPERATURE_SHORT "-st"
#define FLAG_THERMORADIUS           "-thermoradius"
#define FLAG_THERMORADIUS_SHORT     "-thrd"
#define FLAG_THERMORATE             "-thermorate"
#define FLAG_THERMORATE_SHORT       "-thrt"
#define FLAG_TICKRATE               "-tickrate"
#define FLAG_TICKRATE_SHORT         "-tr"
#define FLAG_VERSION                "-version"
#define FLAG_VERSION_SHORT          "-v"

#ifndef SDL_BACKEND
#define FLAG_NOCOLOR                "-nocolor"
#define FLAG_NOCOLOR_SHORT          "-noc"
#define FLAG_NOGLOWCOLOR            "-noglowcolor"
#define FLAG_NOGLOWCOLOR_SHORT      "-nogc"
#endif

/* Constants
 */

static const char APP_ABOUT[] = "The source code of \"" APP_NAME_FORMAL "\" "
"aka " APP_NAME " " APP_VERSION " is available,\n"
"licensed under the " APP_LICENSE " at:\n"
APP_REPOSITORY "\n"
"\n"
"If you did not receive a copy of the license, see below:\n"
APP_LICENSE_URL "\n";

static const char APP_HELP[] = "Usage: " APP_NAME " [OPTIONS]\n"
"\n"
"Silly program to simulate physics in *very* convincing ways.\n"
"It'll be great. Trust me.\n"
"\n";

static const char APP_HELP_COMMANDS[] = "Commands:\n"
"\n"
"    You can enter these commands into the internal command line.\n"
"    There is a short and a long variant for most commands.\n"
"    Some accept arguments.\n"
"\n"
"    " CMD_BRUSH_SHORT " " CMD_BRUSH "\n"
"        selects the brush as active tool\n"
"\n"
"    " CMD_BRUSHMAT_SHORT " " CMD_BRUSHMAT " TEXT\n"
"        selects the given material for the brush\n"
"\n"
"    " CMD_BRUSHRADIUS_SHORT " " CMD_BRUSHRADIUS " NUMBER\n"
"        sets the brush radius to the given number\n"
"\n"
"    " CMD_CLEAR_SHORT " " CMD_CLEAR "\n"
"        clears the world\n"
"\n"
"    " CMD_CLEARALL_SHORT " " CMD_CLEARALL "\n"
"        clears the world, including spawners\n"
"\n"
"    " CMD_COOLER_SHORT " " CMD_COOLER "\n"
"        selects the cooler as active tool\n"
"\n"
"    " CMD_DEFAULTS_SHORT " " CMD_DEFAULTS "\n"
"        sets everything back to default settings\n"
"\n"
"    " CMD_ERASER_SHORT " " CMD_ERASER "\n"
"        selects the eraser as active tool\n"
"\n"
"    " CMD_ERASERRADIUS_SHORT " " CMD_ERASERRADIUS " NUMBER\n"
"        sets the eraser radius to the given number\n"
"\n"
"    " CMD_FRAMERATE_SHORT " " CMD_FRAMERATE " DECIMAL\n"
"        sets the rate-limit for frames per second\n"
"\n"
"    " CMD_GLOWCOLOR_SHORT " " CMD_GLOWCOLOR "\n"
"        enables dot glow coloring\n"
"\n"
"    " CMD_HEATER_SHORT " " CMD_HEATER "\n"
"        selects the heater as active tool\n"
"\n"
"    " CMD_MAT_SHORT " " CMD_MAT " TEXT\n"
"        sets the material of the currently active tool\n"
"\n"
"    " CMD_NOGLOWCOLOR_SHORT " " CMD_NOGLOWCOLOR "\n"
"        disables dot glow coloring\n"
"\n"
"    " CMD_NORMALVISION_SHORT " " CMD_NORMALVISION "\n"
"        disables thermal vision\n"
"\n"
"    " CMD_PAUSE_SHORT " " CMD_PAUSE "\n"
"        pauses the simulation\n"
"\n"
"    " CMD_QUIT_SHORT " " CMD_QUIT "\n"
"        quits and closes the application\n"
"\n"
"    " CMD_SPAWNER_SHORT " " CMD_SPAWNER "\n"
"        selects the spawner as active tool\n"
"\n"
"    " CMD_SPAWNERMAT_SHORT " " CMD_SPAWNERMAT "\n"
"        selects the given material for the spawner\n"
"\n"
"    " CMD_SPAWNTEMPERATURE_SHORT " " CMD_SPAWNTEMPERATURE " DECIMAL\n"
"        sets the temperature of newly spawned dots\n"
"\n"
"    " CMD_SPAWNTEMPERATUREK_SHORT " " CMD_SPAWNTEMPERATUREK " DECIMAL\n"
"        sets the temperature of newly spawned dots in degrees Kelvin\n"
"\n"
"    " CMD_TEMPERATURE_SHORT " " CMD_TEMPERATURE " DECIMAL\n"
"        sets the temperature of existing dots\n"
"\n"
"    " CMD_TEMPERATUREK_SHORT " " CMD_TEMPERATUREK " DECIMAL\n"
"        sets the temperature of existing dots in degrees Kelvin\n"
"\n"
"    " CMD_THERMORADIUS_SHORT " " CMD_THERMORADIUS " NUMBER\n"
"        sets the radius of thermo tools to the given number\n"
"\n"
"    " CMD_THERMORATE_SHORT " " CMD_THERMORATE " DECIMAL\n"
"        sets the rate of thermo tools, with which heating/cooling occurs\n"
"\n"
"    " CMD_THERMOVISION_SHORT " " CMD_THERMOVISION "\n"
"        enables thermal vision\n"
"\n"
"    " CMD_TICKRATE_SHORT " " CMD_TICKRATE " DECIMAL\n"
"        sets the rate for ticks (simulations) per second\n"
"\n";

static const char APP_HELP_FLAGS[] = "Options:\n"
"\n"
"    " FLAG_ABOUT_SHORT " " FLAG_ABOUT "\n"
"        prints program name, version, license and repository information then exits\n"
"\n"
"    " FLAG_BRUSHRADIUS_SHORT " " FLAG_BRUSHRADIUS " NUMBER\n"
"        sets the radius of the brush\n"
"        default: %i\n"
"\n"
"    " FLAG_ERASERRADIUS_SHORT " " FLAG_ERASERRADIUS " NUMBER\n"
"        sets the radius of the eraser\n"
"        default: %i\n"
"\n"
"    " FLAG_FRAMERATE_SHORT " " FLAG_FRAMERATE " DECIMAL\n"
"        sets the rate-limit for frames per second\n"
"        default: %.2f\n"
"\n"
"    " FLAG_HELP_SHORT " " FLAG_HELP "\n"
"        prints this message then exits\n"
"\n"
#ifndef SDL_BACKEND
"    " FLAG_NOCOLOR_SHORT " " FLAG_NOCOLOR "\n"
"        disables all world dot coloring\n"
"\n"
"    " FLAG_NOGLOWCOLOR_SHORT " " FLAG_NOGLOWCOLOR "\n"
"        disables dot glow coloring\n"
"\n"
#endif
"    " FLAG_SPAWNTEMPERATURE_SHORT " " FLAG_SPAWNTEMPERATURE " DECIMAL\n"
"        sets the temperature of every new dot in Kelvin\n"
"        0 °C == %.2f K\n"
"        default: %.2f\n"
"\n"
"    " FLAG_THERMORADIUS_SHORT " " FLAG_THERMORADIUS " NUMBER\n"
"        sets the radius of thermo tools\n"
"        default: %i\n"
"\n"
"    " FLAG_THERMORATE_SHORT " " FLAG_THERMORATE " DECIMAL\n"
"        sets the rate of thermo tools, with which heating/cooling occurs\n"
"        default: %.2f\n"
"\n"
"    " FLAG_TICKRATE_SHORT " " FLAG_TICKRATE " DECIMAL\n"
"        sets the rate for ticks (simulations) per second\n"
"        default: %.2f\n"
"\n"
"    " FLAG_VERSION_SHORT " " FLAG_VERSION "\n"
"        prints version information then exits\n"
"\n";

static const char APP_HELP_MATERIALS[] = "Material list:\n"
"\n";

static const char APP_HELP_KEYBINDS[] = "Keybinds:\n"
"\n"
"    Currently, this program is configured at " CONFIGURED_AT ".\n"
"    It is static.\n"
"    Therefore you can't just change binds.\n"
"\n"
"    %c Escape\n"
"        quit the program\n"
"\n"
"    %c Left-Mouse\n"
"        use currently active tool\n"
"\n"
"    Middle-Mouse\n"
"        pick material for currently active tool\n"
"\n"
"    Right-Mouse\n"
"        drag world view\n"
"\n"
"    %c\n"
"        toggle thermal vision\n"
"        the grayscale displays from %.0f to %.0f degrees Celsius\n"
"\n"
"    %c\n"
"        select an upper material from the material list, for current tool\n"
"\n"
"    %c\n"
"        select uppermost material from the material list, for current tool\n"
"\n"
"    %c\n"
"        select a lower material from the material list, for current tool\n"
"\n"
"    %c\n"
"        select a lowest material from the material list, for current tool\n"
"\n"
"    %c\n"
"        set brush as current tool\n"
"\n"
"    %c\n"
"        set spawner as current tool\n"
"\n"
"    %c\n"
"        set eraser as current tool\n"
"\n"
"    %c\n"
"        set heater as current tool\n"
"\n"
"    %c\n"
"        set cooler as current tool\n"
"\n"
"    %c Left\n"
"        move tool cursor left\n"
"\n"
"    %c Home\n"
"        move tool cursor leftmost\n"
"\n"
"    %c Down\n"
"        move tool cursor down\n"
"\n"
"    %c PgDn\n"
"        move tool cursor to bottom\n"
"\n"
"    %c Up\n"
"        move tool cursor up\n"
"\n"
"    %c PgUp\n"
"        move tool cursor to top\n"
"\n"
"    %c Right\n"
"        move tool cursor right\n"
"\n"
"    %c End\n"
"        move tool cursor rightmost\n"
"\n"
"    Ctrl+Home\n"
"        move tool cursor leftmost and top\n"
"\n"
"    Ctrl+End\n"
"        move tool cursor rightmost and bottom\n"
"\n"
"    %c\n"
"        decrease tool radius\n"
"\n"
"    %c\n"
"        set smallest tool radius\n"
"\n"
"    %c\n"
"        increase tool radius\n"
"\n"
"    %c\n"
"        set biggest tool radius\n"
"\n"
"    %c\n"
"        decrease the simulation speed\n"
"\n"
"    %c\n"
"        set to slowest simulation speed\n"
"\n"
"    %c\n"
"        increase the simulation speed\n"
"\n"
"    %c\n"
"        set to fastest simulation speed\n"
"\n"
"    %c\n"
"        enter the command line\n"
"\n"
"    %s\n"
"        pause world\n"
"\n";

/* Function declarations
 */

bool
handle_args(int                  argc,
            char               **argv,
            float               *framerate,
#ifndef SDL_BACKEND
            bool                *no_color,
            bool                *no_glowcolor,
#endif
            float               *tickrate,
            struct ToolOptions  *tool_opts);

bool
handle_flag_float_arg(int    argc,
                      char **argv,
                      int   *idx,
                      float *out);

bool
handle_flag_int_arg(int    argc,
                    char **argv,
                    int   *idx,
                    int   *out);

void
handle_input(
#ifdef SDL_BACKEND
             SDL_Window          *win,
             SDL_FRect           *world_draw,
#else
             size_t              *cmdline_shift,
             bool                *lmb_pressed,
             const int            win_w,
             struct Rect         *world_draw,
#endif /* SDL_BACKEND */
             bool                *active,
             char                *cmdline,
             size_t              *cmdline_len,
             const float          delta,
             int                 *drag_start_x,
             int                 *drag_start_y,
             char               **feedback,
             clock_t             *feedback_expiration,
             float               *framerate,
             enum InputMode      *input_mode,
             bool                *no_glowcolor,
             clock_t              now,
             bool                *paused,
             float               *tickrate,
             bool                *th_vision,
             struct ToolOptions  *tool_opts,
             struct World        *world);

/* @in: Input.
 * @active: Runtime data.
 * @delta: Runtime data.
 * @input_mode: Runtime data.
 * @paused: Runtime data.
 * @tickrate: Runtime data.
 * @th_vision: Runtime data.
 * @tool_opts: Runtime data.
 * @world: Runtime data.
 * @world_draw: Runtime data.
 *
 * Returns true if the input had been fully handled.
 */
bool
handle_normal_input(const char         *in,
                    bool               *active,
                    const float         delta,
                    enum InputMode     *input_mode,
                    bool               *paused,
                    float              *tickrate,
                    bool               *th_vision,
                    struct ToolOptions *tool_opts,
                    struct World       *world,
#ifdef SDL_BACKEND
                    SDL_FRect          *world_draw);
#else
                    struct Rect        *world_draw);
#endif

struct ToolOptions
new_tool_options(void);

/* Function definitions
 */

bool
handle_args(int                  argc,
            char               **argv,
            float               *framerate,
#ifndef SDL_BACKEND
            bool                *no_color,
            bool                *no_glowcolor,
#endif
            float               *tickrate,
            struct ToolOptions  *tool_opts)
{
	float flagargf;
	int   flagargi;
	int   i;
	char  key_pause[8] = "Space";

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], FLAG_ABOUT_SHORT) == 0 ||
		    strcmp(argv[i], FLAG_ABOUT) == 0) {
			printf(APP_ABOUT);
			return false;
		} else if (strcmp(argv[i], FLAG_BRUSHRADIUS) == 0 ||
		           strcmp(argv[i], FLAG_BRUSHRADIUS_SHORT) == 0) {
			if (!handle_flag_int_arg(argc, argv, &i, &flagargi)) {
				return false;
			}
			tool_opts->brush_radius = flagargi;
			if (tool_opts->brush_radius < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_ERASERRADIUS) == 0 ||
		           strcmp(argv[i], FLAG_ERASERRADIUS_SHORT) == 0) {
			if (!handle_flag_int_arg(argc, argv, &i, &flagargi)) {
				return false;
			}
			tool_opts->eraser_radius = flagargi;
			if (tool_opts->eraser_radius < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_FRAMERATE) == 0 ||
		           strcmp(argv[i], FLAG_FRAMERATE_SHORT) == 0) {
			if (!handle_flag_float_arg(argc, argv, &i, &flagargf)) {
				return false;
			}
			*framerate = flagargf;
			if (*framerate <= 0.0) {
				fprintf(stderr,
				        "The value for \"%s\" must be positive\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_HELP) == 0 ||
		           strcmp(argv[i], FLAG_HELP_SHORT) == 0) {
			printf(APP_HELP);

			printf(APP_HELP_FLAGS,
			       STD_BRUSH_RADIUS,
			       STD_ERASER_RADIUS,
			       STD_FRAMERATE,
			       CELSIUS_TO_KELVIN,
			       STD_SPAWN_TEMPERATURE,
			       STD_THERMO_RADIUS,
			       STD_THERMO_RATE,
			       STD_TICKRATE);

			if (KEY_PAUSE != ' ') {
				key_pause[0] = KEY_PAUSE;
				key_pause[1] = '\0';
			}
			printf(APP_HELP_KEYBINDS,
			       KEY_QUIT,
			       KEY_USE,
			       KEY_SWITCH_VISION,
			       THERMAL_VISION_MIN_T - CELSIUS_TO_KELVIN,
			       THERMAL_VISION_MIN_T - CELSIUS_TO_KELVIN + 255,
			       KEY_PREVIOUS_MAT,
			       KEY_FIRST_MAT,
			       KEY_NEXT_MAT,
			       KEY_LAST_MAT,
			       KEY_BRUSH,
			       KEY_SPAWNER,
			       KEY_ERASER,
			       KEY_HEATER,
			       KEY_COOLER,
			       KEY_LEFT,
			       KEY_LEFT_MAX,
			       KEY_DOWN,
			       KEY_DOWN_MAX,
			       KEY_UP,
			       KEY_UP_MAX,
			       KEY_RIGHT,
			       KEY_RIGHT_MAX,
			       KEY_RADIUS_DOWN,
			       KEY_RADIUS_MIN,
			       KEY_RADIUS_UP,
			       KEY_RADIUS_MAX,
			       KEY_SIMSPEED_DOWN,
			       KEY_SIMSPEED_MIN,
			       KEY_SIMSPEED_UP,
			       KEY_SIMSPEED_MAX,
			       KEY_CMD,
			       key_pause);

			printf(APP_HELP_COMMANDS);

			printf(APP_HELP_MATERIALS);
			for (i = 0; i < MAT_COUNT; i++) {
				printf("    %s\n", MAT_NAME[i]);
			}
			printf("\n");

			return false;
#ifndef SDL_BACKEND
		} else if (strcmp(argv[i], FLAG_NOCOLOR) == 0 ||
		           strcmp(argv[i], FLAG_NOCOLOR_SHORT) == 0) {
			*no_color = true;
		} else if (strcmp(argv[i], FLAG_NOGLOWCOLOR) == 0 ||
		           strcmp(argv[i], FLAG_NOGLOWCOLOR_SHORT) == 0) {
			*no_glowcolor = true;
#endif
		} else if (strcmp(argv[i], FLAG_SPAWNTEMPERATURE) == 0 ||
		           strcmp(argv[i], FLAG_SPAWNTEMPERATURE_SHORT) == 0) {
			if (!handle_flag_float_arg(argc, argv, &i, &flagargf)) {
				return false;
			}
			tool_opts->spawn_temperature = flagargf;
			if (tool_opts->spawn_temperature < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_THERMORADIUS) == 0 ||
		           strcmp(argv[i], FLAG_THERMORADIUS_SHORT) == 0) {
			if (!handle_flag_int_arg(argc, argv, &i, &flagargi)) {
				return false;
			}
			tool_opts->thermo_radius = flagargi;
			if (tool_opts->thermo_radius < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_THERMORATE) == 0 ||
		           strcmp(argv[i], FLAG_THERMORATE_SHORT) == 0) {
			if (!handle_flag_float_arg(argc, argv, &i, &flagargf)) {
				return false;
			}
			tool_opts->thermo_rate = flagargf;
			if (tool_opts->thermo_rate < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_TICKRATE) == 0 ||
		           strcmp(argv[i], FLAG_TICKRATE_SHORT) == 0) {
			if (!handle_flag_float_arg(argc, argv, &i, &flagargf)) {
				return false;
			}
			*tickrate = flagargf;
			if (*tickrate <= 0.0) {
				fprintf(stderr,
				        "The value for \"%s\" must be positive\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_VERSION_SHORT) == 0 ||
		           strcmp(argv[i], FLAG_VERSION) == 0) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return false;
		} else {
			fprintf(stderr,
			        "Argument \"%s\" is not recognized\n",
			        argv[i]);
			return false;
		}
	}

	return true;
}

bool
handle_flag_float_arg(int    argc,
                      char **argv,
                      int   *idx,
                      float *out)
{
	if (argc <= *idx + 1) {
		fprintf(stderr,
		        "The argument \"%s\" needs to be followed by a value\n",
		        argv[*idx]);
		return false;
	}
	*idx += 1;

	errno = 0;
	*out = strtof(argv[*idx], NULL);
	if (errno != 0) {
		fprintf(stderr,
		        "\"%s\" could not be converted to a float\n",
		        argv[*idx - 1]);
		return false;
	}

	return true;
}

bool
handle_flag_int_arg(int    argc,
                    char **argv,
                    int   *idx,
                    int   *out)
{
	if (argc <= *idx + 1) {
		fprintf(stderr,
		        "The argument \"%s\" needs to be followed by a value\n",
		        argv[*idx]);
		return false;
	}
	*idx += 1;

	errno = 0;
	*out = strtol(argv[*idx], NULL, 10);
	if (errno != 0) {
		fprintf(stderr,
		        "\"%s\" could not be converted to an int\n",
		        argv[*idx - 1]);
		return false;
	}

	return true;
}

void
handle_input(
#ifdef SDL_BACKEND
             SDL_Window          *win,
             SDL_FRect           *world_draw,
#else
             size_t              *cmdline_shift,
             bool                *lmb_pressed,
             const int            win_w,
             struct Rect         *world_draw,
#endif /* SDL_BACKEND */
             bool                *active,
             char                *cmdline,
             size_t              *cmdline_len,
             const float          delta,
             int                 *drag_start_x,
             int                 *drag_start_y,
             char               **feedback,
             clock_t             *feedback_expiration,
             float               *framerate,
             enum InputMode      *input_mode,
             bool                *no_glowcolor,
             clock_t              now,
             bool                *paused,
             float               *tickrate,
             bool                *th_vision,
             struct ToolOptions  *tool_opts,
             struct World        *world)
{
#ifdef SDL_BACKEND
	SDL_Event e;
	int mx, my;
	int win_w, win_h;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (SDL_BUTTON_RIGHT == e.button.button) {
				*drag_start_x = e.button.x + world_draw->x;
				*drag_start_y = e.button.y + world_draw->y;
			}
			break;

		case SDL_EVENT_MOUSE_MOTION:
			mx = e.motion.x;
			my = e.motion.y;
			SDL_GetWindowSize(win, &win_w, &win_h);

			if (mx < 0.0) {
				mx  = 0.0;
			}
			else if (mx > win_w) {
				mx  = win_w;
			}
			if (my < 0.0) {
				my  = 0.0;
			}
			else if (my > win_h) {
				my  = win_h;
			}

			tool_opts->x = (mx - world_draw->x) / SDL_WORLD_SCALE;
			tool_opts->y = (my - world_draw->y) / SDL_WORLD_SCALE;
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			tool_radius_add(e.wheel.y, tool_opts);
			break;

		case SDL_EVENT_KEY_DOWN:
			switch (e.key.key) {
			case SDLK_BACKSPACE:
				if (*cmdline_len > 0) {
					cmdline[*cmdline_len - 1] = '\0';
					*cmdline_len -= 1;
				}
				break;

			case SDLK_RETURN:
				handle_command(cmdline,
				               *cmdline_len,
				               active,
				               feedback,
				               feedback_expiration,
				               framerate,
				               no_glowcolor,
				               now,
				               paused,
				               th_vision,
				               tickrate,
				               tool_opts,
				               world);
				cmdline[0] = '\0';
				*cmdline_len = 0;
				*input_mode = IM_NORMAL;
				break;
			}
			break;

		case SDL_EVENT_TEXT_INPUT:
			switch (*input_mode) {
			case IM_COMMAND:
				if (*cmdline_len < CMDLINE_SIZE - 1) {
					cmdline[*cmdline_len] = e.text.text[0];
					cmdline[*cmdline_len + 1] = '\0';
					*cmdline_len += 1;
				}
				break;

			case IM_NORMAL:
				handle_normal_input(e.text.text,
				                    active,
				                    delta,
				                    input_mode,
				                    paused,
				                    tickrate,
				                    th_vision,
				                    tool_opts,
				                    world,
				                    world_draw);
				break;
			}
			break;

		case SDL_EVENT_QUIT:
			*active = false;
			break;
		}
	}

	handle_mouse_state(delta,
	                   drag_start_x,
	                   drag_start_y,
	                   tool_opts,
	                   win,
	                   world,
	                   world_draw);

#else /* SDL_BACKEND */

	ssize_t input_len = 0;
	char    input[INPUT_SIZE];

	input_len = read(STDIN_FILENO, &input, INPUT_SIZE);

	switch (*input_mode) {
	case IM_NORMAL:
		if (input_len > 0 &&
		    input_len < INPUT_SIZE) {
			input[input_len] = '\0';
			if (!handle_normal_input(input,
			                         active,
			                         delta,
			                         input_mode,
			                         paused,
			                         tickrate,
			                         th_vision,
			                         tool_opts,
			                         world,
			                         world_draw)) {
				handle_normal_csi_input(input,
				                        delta,
				                        drag_start_x,
				                        drag_start_y,
				                        lmb_pressed,
				                        tool_opts,
				                        world,
				                        world_draw);
			}
		}
		break;

	case IM_COMMAND:
		if (input_len == 1) {
			handle_command_input(input,
			                     active,
			                     cmdline,
			                     cmdline_len,
			                     cmdline_shift,
			                     feedback,
			                     feedback_expiration,
			                     framerate,
			                     input_mode,
			                     no_glowcolor,
			                     now,
			                     paused,
			                     th_vision,
			                     tickrate,
			                     tool_opts,
			                     win_w,
			                     world);
		}
		break;
	}
#endif /* SDL_BACKEND */
}

bool
handle_normal_input(const char         *in,
                    bool               *active,
                    const float         delta,
                    enum InputMode     *input_mode,
                    bool               *paused,
                    float              *tickrate,
                    bool               *th_vision,
                    struct ToolOptions *tool_opts,
                    struct World       *world,
#ifdef SDL_BACKEND
                    SDL_FRect          *world_draw)
#else
                    struct Rect        *world_draw)
#endif
{
	switch (in[0]) {
	case KEY_QUIT:
		*active = false;
		break;

	case KEY_USE:
		use_tool(delta, *tool_opts, world);
		break;

	case KEY_SWITCH_VISION:
		if (*th_vision)
			*th_vision = false;
		else
			*th_vision = true;
		break;

	case KEY_PREVIOUS_MAT:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			if (tool_opts->brush_mat > FIRST_REAL_MAT) {
				tool_opts->brush_mat -= 1;
			}
			break;

		case TOOL_SPAWNER:
			if (tool_opts->spawner_mat > MAT_NONE) {
				tool_opts->spawner_mat -= 1;
			}
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_FIRST_MAT:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_mat = FIRST_REAL_MAT;
			break;

		case TOOL_SPAWNER:
			tool_opts->spawner_mat = MAT_NONE;
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_NEXT_MAT:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			if (tool_opts->brush_mat < MAT_COUNT - 1) {
				tool_opts->brush_mat += 1;
			}
			break;

		case TOOL_SPAWNER:
			if (tool_opts->spawner_mat < MAT_COUNT - 1) {
				tool_opts->spawner_mat += 1;
			}
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_LAST_MAT:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_mat = MAT_COUNT - 1;
			break;

		case TOOL_SPAWNER:
			tool_opts->spawner_mat = MAT_COUNT - 1;
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_BRUSH:
		tool_opts->sel_tool = TOOL_BRUSH;
		break;

	case KEY_SPAWNER:
		tool_opts->sel_tool = TOOL_SPAWNER;
		break;

	case KEY_ERASER:
		tool_opts->sel_tool = TOOL_ERASER;
		break;

	case KEY_HEATER:
		tool_opts->sel_tool = TOOL_HEATER;
		break;

	case KEY_COOLER:
		tool_opts->sel_tool = TOOL_COOLER;
		break;

	case KEY_LEFT:
		if (tool_opts->x > 0) {
			tool_opts->x -= 1;
			if (tool_opts->x < world_draw->x) {
				world_draw->x -= 1;
			}
		}
		break;

	case KEY_LEFT_MAX:
		tool_opts->x = 0;
		world_draw->x = 0;
		break;

	case KEY_DOWN:
		if (tool_opts->y < world->h - 1) {
			tool_opts->y += 1;
			if (tool_opts->y >= world_draw->y + world_draw->h) {
				world_draw->y += 1;
			}
		}
		break;

	case KEY_DOWN_MAX:
		tool_opts->y = world->h - 1;
		world_draw->y = world->h - world_draw->h;
		break;

	case KEY_UP:
		if (tool_opts->y > 0) {
			tool_opts->y -= 1;
			if (tool_opts->y < world_draw->y) {
				world_draw->y -= 1;
			}
		}
		break;

	case KEY_UP_MAX:
		tool_opts->y = 0;
		world_draw->y = 0;
		break;

	case KEY_RIGHT:
		if (tool_opts->x < world->w - 1) {
			tool_opts->x += 1;
			if (tool_opts->x >= world_draw->x + world_draw->w) {
				world_draw->x += 1;
			}
		}
		break;

	case KEY_RIGHT_MAX:
		tool_opts->x = world->w - 1;
		world_draw->x = world->w - world_draw->w;
		break;

	case KEY_RADIUS_DOWN:
		tool_radius_add(-1, tool_opts);
		break;

	case KEY_RADIUS_MIN:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_radius = 0;
			break;

		case TOOL_SPAWNER:
			break;

		case TOOL_ERASER:
			tool_opts->eraser_radius = 0;
			break;

		case TOOL_HEATER:
		case TOOL_COOLER:
			tool_opts->thermo_radius = 0;
			break;

		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_RADIUS_UP:
		tool_radius_add(1, tool_opts);
		break;

	case KEY_RADIUS_MAX:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_radius = MAX_RADIUS;
			break;

		case TOOL_SPAWNER:
			break;

		case TOOL_ERASER:
			tool_opts->eraser_radius = MAX_RADIUS;
			break;

		case TOOL_HEATER:
		case TOOL_COOLER:
			tool_opts->thermo_radius = MAX_RADIUS;
			break;

		case TOOL_COUNT:
			break;
		}
		break;

	case '-':
	case KEY_SIMSPEED_DOWN:
		if (*tickrate > MIN_TICKRATE) {
			*tickrate /= 2;
		}
		break;

	case KEY_SIMSPEED_MIN:
		*tickrate = MIN_TICKRATE;
		break;

	case '+':
	case KEY_SIMSPEED_UP:
		if (*tickrate < MAX_TICKRATE) {
			*tickrate *= 2;
		}
		break;

	case KEY_SIMSPEED_MAX:
		*tickrate = MAX_TICKRATE;
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

	default:
		return false;
		break;
	}

	return true;
}

struct ToolOptions
new_tool_options(void)
{
	struct ToolOptions ret = {
		.brush_mat = FIRST_REAL_MAT,
		.brush_radius = STD_BRUSH_RADIUS,
		.eraser_radius = STD_ERASER_RADIUS,
		.sel_tool = STD_SELECTED_TOOL,
		.spawn_temperature = STD_SPAWN_TEMPERATURE,
		.spawner_mat = FIRST_REAL_MAT,
		.thermo_radius = STD_THERMO_RADIUS,
		.thermo_rate = STD_THERMO_RATE,
		.x = 0,
		.y = 0,
	};
	return ret;
}

int
main(int    argc,
     char **argv)
{
	bool                   active = true;
	char                   cmdline[CMDLINE_SIZE];
	size_t                 cmdline_len = 0;
	float                  delta = 0.0;
	int                    drag_start_x = 0;
	int                    drag_start_y = 0;
	char                  *feedback = NULL;
	clock_t                feedback_expiration = 0;
	float                  framerate = STD_FRAMERATE;
	enum InputMode         input_mode = IM_NORMAL;
	char                  *ip_address = "localhost";
	bool                   paused = false;
	clock_t                last_input = 0;
	clock_t                last_frame = 0;
	clock_t                last_tick = 0;
	bool                   no_glowcolor = false;
	clock_t                now = 0;
	bool                   th_vision = false;
	float                  tickrate = STD_TICKRATE;
	struct ToolOptions     tool_opts;
	struct World           world;
	char                  *world_name = "worldname";

#ifdef SDL_BACKEND
	SDL_Renderer *renderer = NULL;
	TTF_Font     *font = NULL;
	size_t        i;
	SDL_Window   *win = NULL;
	SDL_FRect     world_draw = {
		.x = 0,
		.y = 0,
		.w = 0,
		.h = 0,
	};
	SDL_Texture  *world_tx = NULL;
#else
	size_t                 cmdline_shift = 0;
	char                  *display = NULL;
	size_t                 display_size = 0;
	size_t                 dot_depth = 0;
	bool                   lmb_pressed = false;
	bool                   no_color = false;
	size_t                 statusbar_elems = 0;
	enum StatusbarElement  statusbar_elem[ARRSIZE(STATUSBAR_DISPLAY_PRIORITY)];
	int                    win_w = 0;
	int                    win_h = 0;
	struct winsize         ws;
	struct Rect            world_draw = {
		.x = 0,
		.y = 0,
		.w = 0,
		.h = 0,
	};
	int                    world_draw_space_w = 0;
	int                    world_draw_space_h = 0;
#endif

	if (!handle_args(argc, argv,
	                 &framerate,
#ifndef SDL_BACKEND
	                 &no_color,
	                 &no_glowcolor,
#endif
	                 &tickrate,
	                 &tool_opts)) {
		return 0;
	}

	hawps_core_init();
	hawps_extra_init();

	cmdline[0] = '\0';
	tool_opts = new_tool_options();

#ifdef SDL_BACKEND
	// TODO add proper identifier
	SDL_SetAppMetadata(APP_NAME, APP_VERSION, "lol.69." APP_NAME);

	if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
		fprintf(stderr, "%s\n", SDL_GetError());
		goto cleanup;
	}

	if (!TTF_Init()) {
		fprintf(stderr, "%s\n", SDL_GetError());
		goto cleanup;
	}

	for (i = 0; i < ARRLEN(FONTPATH); i++) {
		font = TTF_OpenFont(FONTPATH[i], SDL_FONT_SIZE);
		if (NULL != font) {
			break;
		}
	}
	if (NULL == font) {
		fprintf(stderr, "%s\n", SDL_GetError());
		goto cleanup;
	}
	TTF_SetFontDirection(font, TTF_DIRECTION_LTR);
	TTF_SetFontLanguage(font, "en");

	if (!SDL_CreateWindowAndRenderer(APP_NAME_FORMAL,
	                                 SDL_WIN_WIDTH, SDL_WIN_HEIGHT,
	                                 SDL_WINDOW_RESIZABLE,
	                                 &win, &renderer)) {
		fprintf(stderr, "%s\n", SDL_GetError());
		goto cleanup;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_StartTextInput(win);

	SDL_GetWindowSize(win, &world.w, &world.h);
	world.h -= SDL_FONT_SIZE * 2;

	world_draw.x = 0;
	world_draw.y = 0;
	world_draw.w = world.w;
	world_draw.h = world.h;

	world.w /= SDL_WORLD_SCALE;
	world.h /= SDL_WORLD_SCALE;
	world_tx = SDL_CreateTexture(renderer,
	                             SDL_PIXELFORMAT_RGBA8888,
	                             SDL_TEXTUREACCESS_TARGET,
	                             world.w, world.h);
	SDL_SetTextureScaleMode(world_tx, SDL_SCALEMODE_PIXELART);
#else
	CSI_set_raw();
	fputs(CSI_CLEAR, stdout);

	if (no_color) {
		dot_depth = 1;
	} else {
		dot_depth = CSI_COLORSTRING_LEN + 1;
	}

	ws = CSI_get_size();
	world.w = ws.ws_col;
	world.h = ws.ws_row - 2;

	/* we love hacks
	 * handle_resize ONLY reallocs for performance */
	display = malloc(1);
	if (NULL == display) {
		fprintf(stderr, "Could not allocate memory\n");
		goto cleanup;
	}

	handle_resize(cmdline_len,
		      &cmdline_shift,
		      &display,
		      &display_size,
		      dot_depth,
		      input_mode,
		      ip_address,
		      &statusbar_elems,
		      statusbar_elem,
		      &win_w,
		      &win_h,
		      world,
		      &world_draw,
		      &world_draw_space_w,
		      &world_draw_space_h,
		      world_name);
#endif /* SDL_BACKEND */

	world = world_new(world.w, world.h, tool_opts.spawn_temperature);

	while (active) {
		now = clock();
		delta = (float) (now - last_input) / (float) CLOCKS_PER_SEC;

		last_input = now;

		handle_input(
#ifdef SDL_BACKEND
		             win,
		             &world_draw,
#else
		             &cmdline_shift,
		             &lmb_pressed,
		             win_w,
		             &world_draw,
#endif
		             &active,
		             cmdline,
		             &cmdline_len,
		             delta,
		             &drag_start_x,
		             &drag_start_y,
		             &feedback,
		             &feedback_expiration,
		             &framerate,
		             &input_mode,
		             &no_glowcolor,
		             now,
		             &paused,
		             &tickrate,
		             &th_vision,
		             &tool_opts,
		             &world);

#ifdef SDL_BACKEND
#else
		if (lmb_pressed) {
			use_tool(delta, tool_opts, &world);
		}
#endif
		if (now - last_tick >= (long) (CLOCKS_PER_SEC / tickrate)) {
			last_tick = now;

			world_update(&world, tool_opts.spawn_temperature);

			if (!paused) {
				world_sim(&world);
			}
		}

		if (now - last_frame >= (long) (CLOCKS_PER_SEC / framerate)) {
			last_frame = now;

			if (now > feedback_expiration) {
				feedback = NULL;
			}

#ifdef SDL_BACKEND
			draw(cmdline,
			     feedback,
			     font,
			     input_mode,
			     ip_address,
			     no_glowcolor,
			     paused,
			     th_vision,
			     tickrate,
			     tool_opts,
			     renderer,
			     world,
			     world_draw,
			     world_name,
			     world_tx);
#else
			handle_resize(cmdline_len,
				      &cmdline_shift,
				      &display,
				      &display_size,
				      dot_depth,
				      input_mode,
				      ip_address,
				      &statusbar_elems,
				      statusbar_elem,
				      &win_w,
				      &win_h,
				      world,
				      &world_draw,
				      &world_draw_space_w,
				      &world_draw_space_h,
				      world_name);

			draw(cmdline,
			     cmdline_len,
			     cmdline_shift,
			     display,
			     display_size,
			     dot_depth,
			     feedback,
			     input_mode,
			     ip_address,
			     no_color,
			     no_glowcolor,
			     paused,
			     statusbar_elems,
			     statusbar_elem,
			     tickrate,
			     th_vision,
			     tool_opts,
			     win_w,
			     world,
			     world_draw,
			     world_draw_space_w,
			     world_draw_space_h,
			     world_name);
#endif /* SDL_BACKEND */
		}
	}

cleanup:
#ifdef SDL_BACKEND
	SDL_DestroyTexture(world_tx);
	SDL_StopTextInput(win);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
#else
	CSI_set_normal();
	fputs(CSI_CLEAR, stdout);
	fputs(CSI_FG_DEFAULT, stdout);
	fputs(CSI_BG_DEFAULT, stdout);

	if (display != NULL) {
		free(display);
	}
#endif

	world_free(&world);

	return 0;
}
