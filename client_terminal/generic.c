/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include "generic.h"

#include <stddef.h>

#include "config.h"
#include "int_to_string.h"
#include "str.h"

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
