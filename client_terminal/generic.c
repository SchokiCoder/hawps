/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include "generic.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "int_to_string.h"
#include "str.h"

void
command_temperature(const float   new_temperature,
                    struct World *world)
{
	int x, y;

	for (x = 0; x < world->w; x++) {
		for (y = 0; y < world->h; y++) {
			world->thermo[x][y] = new_temperature;

			if (world->thermo[x][y] >= MAT_BOIL_P[world->dot[x][y]]) {
				if (MAT_MELT_DECOMP[world->dot[x][y]]) {
					world->dot[x][y] = mat_melt_prdct(world->dot[x][y]);
				}
			}
		}
	}
}

size_t
write_statusbar_elem(char                        *out,
                     const size_t                 out_size,
                     const char                  *ip_address,
                     const bool                   paused,
                     const enum StatusbarElement  sbe,
                     const bool                   th_vision,
                     const float                  tickrate,
                     struct ToolOptions           tool_opts,
                     const char                  *world_name)
{
	char  *vision = NULL;
	size_t written = 0;

	switch (sbe) {
	case SBE_WORLD_NAME:
		written += string_cat(out,
		                      out_size,
		                      written,
		                      world_name);
		break;

	case SBE_COORDS:
		written += string_cat(out,
		                      out_size,
		                      written,
		                      NUMBERSTRING[tool_opts.x]);
		out[written] = ',';
		written += 1;
		written += string_cat(out,
		                      out_size,
		                      written,
		                      NUMBERSTRING[tool_opts.y]);
		break;

	case SBE_VIEW:
		if (th_vision) {
			vision = "Thermal";
		} else {
			vision = "Normal";
		}

		written += string_cat(out,
		                      out_size,
		                      written,
		                      "View:");
		written += string_cat(out,
		                      out_size,
		                      written,
		                      vision);
		break;

	case SBE_SPEED:
		written += string_cat(out,
		                      out_size,
		                      written,
		                      "Speed:");
		if (paused) {
			written += string_cat(out,
			                      out_size,
			                      written,
			                      "None");
		} else {
			written += string_cat(out,
			                      out_size,
			                      written,
			                      NUMBERSTRING[(int) tickrate]);
			written += string_cat(out,
			                      out_size,
			                      written,
			                      "/s");
		}
		break;

	case SBE_IP_ADDRESS:
		written += string_cat(out,
		                      out_size,
		                      written,
		                      ip_address);
		break;

	case SBE_COUNT:
		break;
	}

	return written;
}

size_t
write_tool_hint(char                     *out,
                const size_t              out_size,
                const struct ToolOptions  tool_opts)
{
	size_t written = 0;

	written += string_cat(out,
	                      out_size,
	                      written,
	                      TOOL_NAME[tool_opts.sel_tool]);

	if (tool_opts.sel_tool == TOOL_BRUSH) {
		written += string_cat(out, out_size, written, " ");
		written += string_cat(out,
		                      out_size,
		                      written,
		                      MAT_NAME[tool_opts.brush_mat]);
	} else if (tool_opts.sel_tool == TOOL_SPAWNER) {
		written += string_cat(out, out_size, written, " ");
		written += string_cat(out,
		                      out_size,
		                      written,
		                      MAT_NAME[tool_opts.spawner_mat]);
	}

	return written;
}

struct Rgba
get_normal_dot_color(const struct World world,
                     const int          x,
                     const int          y)
{
	struct Rgba a, b;

	a = thermo_to_color(world.thermo[x][y]);

	b.r = MAT_R[world.dot[x][y]];
	b.g = MAT_G[world.dot[x][y]];
	b.b = MAT_B[world.dot[x][y]];
	b.a = 255;

	return rgba_blend(a, b);
}

struct Rgba
get_normal_dot_color_simple(const struct World world,
                            const int          x,
                            const int          y)
{
	struct Rgba ret;

	ret.r = MAT_R[world.dot[x][y]];
	ret.g = MAT_G[world.dot[x][y]];
	ret.b = MAT_B[world.dot[x][y]];
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

void
handle_advanced_command(const char          *cmd,
                        const char          *arg,
                        char               **feedback,
                        clock_t             *feedback_expiration,
                        float               *framerate,
                        const clock_t        now,
                        float               *tickrate,
                        struct ToolOptions  *tool_opts,
                        struct World        *world)
{
	float f = 0.0;
	long l;
	int x, y;

	if (strcmp(cmd, CMD_BRUSHMAT) == 0 ||
	    strcmp(cmd, CMD_BRUSHMAT_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_BRUSH;
		if (mat_from_string(arg, &tool_opts->brush_mat)) {
			return;
		}

		set_feedback(feedback, feedback_expiration, now,
		             "Material not recognized.");
	} else if (strcmp(cmd, CMD_BRUSHRADIUS) == 0 ||
	           strcmp(cmd, CMD_BRUSHRADIUS_SHORT) == 0) {
		errno = 0;
		l = strtol(arg, NULL, 10);

		if (errno != 0 ||
		    l < 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->brush_radius = l;
		}
	} else if (strcmp(cmd, CMD_ERASERRADIUS) == 0 ||
	           strcmp(cmd, CMD_ERASERRADIUS_SHORT) == 0) {
		errno = 0;
		l = strtol(arg, NULL, 10);

		if (errno != 0 ||
		    l < 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->eraser_radius = l;
		}
	} else if (strcmp(cmd, CMD_FRAMERATE) == 0 ||
	           strcmp(cmd, CMD_FRAMERATE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
			return;
		}

		if (f <= 0.0) {
			set_feedback(feedback, feedback_expiration, now,
			             "No.");
			return;
		}

		*framerate = f;
	} else if (strcmp(cmd, CMD_MAT) == 0 ||
	           strcmp(cmd, CMD_MAT_SHORT) == 0) {
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			if (mat_from_string(arg, &tool_opts->brush_mat)) {
				return;
			}

			set_feedback(feedback, feedback_expiration, now,
			             "Material not recognized.");
			break;

		case TOOL_SPAWNER:
			if (mat_from_string(arg, &tool_opts->spawner_mat)) {
				return;
			}

			set_feedback(feedback, feedback_expiration, now,
			             "Material not recognized.");
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			set_feedback(feedback, feedback_expiration, now,
			             "Unsupported tool selected.");
			break;
		}
	} else if (strcmp(cmd, CMD_SPAWNERMAT) == 0 ||
	           strcmp(cmd, CMD_SPAWNERMAT_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_SPAWNER;
		if (mat_from_string(arg, &tool_opts->spawner_mat)) {
			return;
		}

		set_feedback(feedback, feedback_expiration, now,
		             "Material not recognized.");
	} else if (strcmp(cmd, CMD_SPAWNTEMPERATURE) == 0 ||
	           strcmp(cmd, CMD_SPAWNTEMPERATURE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->spawn_temperature = f + CELSIUS_TO_KELVIN;
		}
	} else if (strcmp(cmd, CMD_SPAWNTEMPERATUREK) == 0 ||
	           strcmp(cmd, CMD_SPAWNTEMPERATUREK_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->spawn_temperature = f;
		}
	} else if (strcmp(cmd, CMD_TEMPERATURE) == 0 ||
	           strcmp(cmd, CMD_TEMPERATURE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			command_temperature(f + CELSIUS_TO_KELVIN, world);
		}
	} else if (strcmp(cmd, CMD_TEMPERATUREK) == 0 ||
	           strcmp(cmd, CMD_TEMPERATUREK_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			command_temperature(f, world);
		}
	} else if (strcmp(cmd, CMD_THERMORADIUS) == 0 ||
	           strcmp(cmd, CMD_THERMORADIUS_SHORT) == 0) {
		errno = 0;
		l = strtol(arg, NULL, 10);

		if (errno != 0 ||
		    l < 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->thermo_radius = l;
		}
	} else if (strcmp(cmd, CMD_THERMORATE) == 0 ||
	           strcmp(cmd, CMD_THERMORATE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
			return;
		}

		if (f == 0.0) {
			set_feedback(feedback, feedback_expiration, now,
			             "That's dumb, but okay.");
		}

		tool_opts->thermo_rate = f;
	} else if (strcmp(cmd, CMD_TICKRATE) == 0 ||
	           strcmp(cmd, CMD_TICKRATE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
			return;
		}

		if (f <= 0.0) {
			set_feedback(feedback, feedback_expiration, now,
			             "No.");
			return;
		}

		*tickrate = f;
	} else {
		set_feedback(feedback, feedback_expiration, now,
		             "Command not recognized.");
	}
}

void
handle_cmdline_shift(const size_t          cmdline_len,
                     size_t               *cmdline_shift,
                     const int             win_w)
{
	if (1 + cmdline_len + 1 > (size_t) win_w) {
		*cmdline_shift = 1 + cmdline_len + 1 - win_w;
	} else {
		*cmdline_shift = 0;
	}
}

void
handle_command(char                *cmdline,
               const size_t         cmdline_len,
               bool                *active,
               char               **feedback,
               clock_t             *feedback_expiration,
               float               *framerate,
               bool                *no_glowcolor,
               const clock_t        now,
               bool                *paused,
               bool                *th_vision,
               float               *tickrate,
               struct ToolOptions  *tool_opts,
               struct World        *world)
{
	char buf1[BUF_SIZE];
	char buf2[BUF_SIZE];
	size_t i;

	buf1[0] = '\0';
	buf2[0] = '\0';

	for (i = 0; i < cmdline_len; i++) {
		switch (cmdline[i]) {
		case ' ':
			cmdline[i] = '\0';
			string_cat(buf1, BUF_SIZE, 0, cmdline);
			cmdline[i] = ' ';
			string_cat(buf2, BUF_SIZE, 0, &cmdline[i + 1]);

			handle_advanced_command(buf1, buf2,
			                        feedback,
			                        feedback_expiration,
			                        framerate,
			                        now,
			                        tickrate,
			                        tool_opts,
			                        world);
			return;
			break;

		case '\n':
		case '\r':
		case '\0':
			i = cmdline_len;
			break;
		}
	}

	handle_simple_command(cmdline,
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
}

void
handle_simple_command(const char          *cmdline,
                      bool                *active,
                      char               **feedback,
                      clock_t             *feedback_expiration,
                      float               *framerate,
                      bool                *no_glowcolor,
                      clock_t              now,
                      bool                *paused,
                      bool                *th_vision,
                      float               *tickrate,
                      struct ToolOptions  *tool_opts,
                      struct World        *world)
{
	int x, y;

	*feedback = NULL;

	if (strcmp(cmdline, CMD_BRUSH) == 0 ||
	    strcmp(cmdline, CMD_BRUSH_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_BRUSH;
	} else if (strcmp(cmdline, CMD_CLEAR) == 0 ||
	           strcmp(cmdline, CMD_CLEAR_SHORT) == 0) {
		for (x = 0; x < world->w; x++) {
			for (y = 0; y < world->h; y++) {
				world_clear_dot(world, x, y);
			}
		}
	} else if (strcmp(cmdline, CMD_CLEARALL) == 0 ||
	           strcmp(cmdline, CMD_CLEARALL_SHORT) == 0) {
		for (x = 0; x < world->w; x++) {
			for (y = 0; y < world->h; y++) {
				world_clear_dot(world, x, y);
				world->spawner[x][y] = false;
			}
		}
	} else if (strcmp(cmdline, CMD_COOLER) == 0 ||
	           strcmp(cmdline, CMD_COOLER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_COOLER;
	} else if (strcmp(cmdline, CMD_DEFAULTS) == 0 ||
	           strcmp(cmdline, CMD_DEFAULTS_SHORT) == 0) {
		*framerate = STD_FRAMERATE;
		*tickrate = STD_TICKRATE;
		tool_opts->brush_radius = STD_BRUSH_RADIUS;
		tool_opts->eraser_radius = STD_ERASER_RADIUS;
		tool_opts->sel_tool = STD_SELECTED_TOOL;
		tool_opts->thermo_radius = STD_THERMO_RADIUS;
		tool_opts->thermo_rate = STD_THERMO_RATE;
		tool_opts->spawn_temperature = STD_SPAWN_TEMPERATURE;
	} else if (strcmp(cmdline, CMD_ERASER) == 0 ||
	           strcmp(cmdline, CMD_ERASER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_ERASER;
	} else if (strcmp(cmdline, CMD_GLOWCOLOR) == 0 ||
	           strcmp(cmdline, CMD_GLOWCOLOR_SHORT) == 0) {
		*no_glowcolor = false;
	} else if (strcmp(cmdline, CMD_HEATER) == 0 ||
	           strcmp(cmdline, CMD_HEATER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_HEATER;
	} else if (strcmp(cmdline, CMD_NOGLOWCOLOR) == 0 ||
	           strcmp(cmdline, CMD_NOGLOWCOLOR_SHORT) == 0) {
		*no_glowcolor = true;
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
		tool_opts->sel_tool = TOOL_SPAWNER;
	} else if (strcmp(cmdline, CMD_THERMOVISION) == 0 ||
	           strcmp(cmdline, CMD_THERMOVISION_SHORT) == 0) {
		*th_vision = true;
	} else {
		set_feedback(feedback, feedback_expiration, now,
		             "Command not recognized.");
	}
}

void
set_feedback(char          **feedback,
             clock_t        *feedback_expiration,
             const clock_t   now,
             char           *str)
{
	*feedback = str;
	*feedback_expiration = now + (CLOCKS_PER_SEC * FEEDBACK_LIFETIME);
}

void
tool_radius_add(const int           radius_change,
                struct ToolOptions *tool_opts)
{
	int  *target = NULL;

	switch (tool_opts->sel_tool) {
	case TOOL_BRUSH:
		target = &tool_opts->brush_radius;
		break;

	case TOOL_SPAWNER:
		return;
		break;

	case TOOL_ERASER:
		target = &tool_opts->eraser_radius;
		break;

	case TOOL_HEATER:
	case TOOL_COOLER:
		target = &tool_opts->thermo_radius;
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

void
use_tool(const float         delta,
         struct ToolOptions  tool_opts,
         struct World       *world)
{
	switch (tool_opts.sel_tool) {
	case TOOL_BRUSH:
		world_use_brush(world,
		                tool_opts.brush_mat,
		                tool_opts.spawn_temperature,
		                tool_opts.x,
		                tool_opts.y,
		                tool_opts.brush_radius);
		break;

	case TOOL_SPAWNER:
		world->spawner[tool_opts.x][tool_opts.y] = true;
		world->spawner_mat[tool_opts.x][tool_opts.y] = tool_opts.spawner_mat;
		break;

	case TOOL_ERASER:
		world_use_eraser(world,
		                 tool_opts.x,
		                 tool_opts.y,
		                 tool_opts.eraser_radius);
		break;

	case TOOL_HEATER:
		world_use_heater(world,
		                 tool_opts.thermo_rate * delta,
		                 tool_opts.x,
		                 tool_opts.y,
		                 tool_opts.thermo_radius);
		break;

	case TOOL_COOLER:
		world_use_cooler(world,
		                 tool_opts.thermo_rate * delta,
		                 tool_opts.x,
		                 tool_opts.y,
		                 tool_opts.thermo_radius);
		break;

	case TOOL_COUNT:
		break;
	}
}
