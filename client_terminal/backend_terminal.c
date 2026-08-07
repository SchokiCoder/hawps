/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef SDL_BACKEND

#include "backend_terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic.h"
#include "int_to_string.h"
#include "str.h"

/* Macros
 */

#define DOT_EMPTY_LINES_RENDER_LOOP \
	for (y = 0; y < world_draw_space_h; y++) { \
		memset(&out[written], ' ', world_draw.w + world_draw_space_w); \
		written += world_draw.w + world_draw_space_w;\
	}

#define DOT_RENDER_LOOP(COLOR) \
	for (y = world_draw.y; y < world_draw.h + world_draw.y; y++) { \
		for (x = world_draw.x; x < world_draw.w + world_draw.x; x++) { \
			written += render_dot(&out[written], \
			                      out_size - written, \
			                      COLOR, \
			                      world, \
			                      x, \
			                      y); \
		} \
		memset(&out[written], ' ', world_draw_space_w); \
		written += world_draw_space_w;\
	} \
	DOT_EMPTY_LINES_RENDER_LOOP

#define DOT_RENDER_LOOP_NO_COLOR \
	for (y = world_draw.y; y < world_draw.h + world_draw.y; y++) { \
		for (x = world_draw.x; x < world_draw.w + world_draw.x; x++) { \
			written += render_dot_no_color(&out[written], \
			                               world, \
			                               x, \
			                               y); \
		} \
		memset(&out[written], ' ', world_draw_space_w); \
		written += world_draw_space_w;\
	} \
	DOT_EMPTY_LINES_RENDER_LOOP

/* Function declarations
 */

size_t
render_dot(char               *out,
           const size_t        out_size,
           const struct Rgba   color,
           const struct World  world,
           const int           x,
           const int           y);

size_t
render_dot_no_color(char               *out,
                    const struct World  world,
                    const int           x,
                    const int           y);

size_t
render_statusbar_display(char                        *out,
                         const size_t                 out_size,
                         const char                  *ip_address,
                         const bool                   paused,
                         const enum StatusbarElement  sbe,
                         const float                  tickrate,
                         const bool                   th_vision,
                         struct ToolOptions           tool_opts,
                         const char                  *world_name);

size_t
render_tool_hint(char                     *out,
                 const size_t              out_size,
                 const struct ToolOptions  tool_opts);

size_t
render_world(char               *out,
             const size_t        out_size,
             const size_t        dot_depth,
             const bool          no_color,
             const bool          no_glowcolor,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world,
             const struct Rect   world_draw,
             const int           world_draw_space_w,
             const int           world_draw_space_h);

/* Function definitions
 */

void
draw(const char                  *cmdline,
     const size_t                 cmdline_len,
     const size_t                 cmdline_shift,
     char                        *display,
     const size_t                 display_size,
     const size_t                 dot_depth,
     const char                  *feedback,
     const enum InputMode         input_mode,
     const char                  *ip_address,
     const bool                   no_color,
     const bool                   no_glowcolor,
     const bool                   paused,
     const size_t                 statusbar_elems,
     const enum StatusbarElement *statusbar_elem,
     const float                  tickrate,
     const bool                   th_vision,
     const struct ToolOptions     tool_opts,
     const int                    win_w,
     const struct World           world,
     const struct Rect            world_draw,
     const int                    world_draw_space_w,
     const int                    world_draw_space_h,
     const char                  *world_name)
{
	char   buf[BUF_SIZE];
	size_t buf_len = 0;
	size_t display_len = 0;
	size_t feedback_len;
	size_t i;
	size_t space_len = 0;
	size_t st_bar_len = 0;

	display[0] = '\0';

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
	                            dot_depth,
	                            no_color,
	                            no_glowcolor,
	                            th_vision,
	                            tool_opts,
	                            world,
	                            world_draw,
	                            world_draw_space_w,
	                            world_draw_space_h);

	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          CSI_FG_DEFAULT);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          CSI_BG_DEFAULT);

	st_bar_len = display_len;

	i = 0;
	while (1) {
		display_len += render_statusbar_display(&display[display_len],
		                                        display_size - display_len,
		                                        ip_address,
		                                        paused,
		                                        statusbar_elem[i],
		                                        tickrate,
		                                        th_vision,
		                                        tool_opts,
		                                        world_name);

		i++;
		if (i >= statusbar_elems) {
			break;
		}

		display_len += string_cat(display,
		                          display_size,
		                          display_len,
		                          STATUSBAR_SEPARATOR);
	}

	st_bar_len = display_len - st_bar_len;

	space_len = win_w - st_bar_len;
	memset(&display[display_len], ' ', space_len);
	display_len += space_len;

	switch (input_mode) {
	case IM_NORMAL:
		if (feedback != NULL) {
			feedback_len = strlen(feedback);
			if (feedback_len > (size_t) win_w) {
				feedback_len -= feedback_len - win_w;
			}
			display_len += string_cat(display,
			                          /* hack: */
			                          display_len + feedback_len + 1,
			                          display_len,
			                          feedback);
			space_len = win_w - feedback_len;
			break;
		}

		buf[0] = '\0';
		buf_len = 0;
		buf_len = render_tool_hint(buf, BUF_SIZE, tool_opts);

		if (buf_len > (size_t) win_w) {
			buf_len -= buf_len - win_w;
			buf[buf_len] = '\0';
		}

		display_len += string_cat(display, display_size, display_len, buf);

		space_len = win_w - buf_len;
		break;

	case IM_COMMAND:
		display[display_len] = CMDLINE_INDICATOR;
		display_len += 1;

		display_len += string_cat(display,
		                          display_size,
		                          display_len,
		                          &cmdline[cmdline_shift]);

		display[display_len] = CMDLINE_CURSOR;
		display_len += 1;

		space_len = win_w - 1 - cmdline_len + cmdline_shift - 1;
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

void
handle_mouse_input(const char         *in,
                   const float         delta,
                   int                *drag_start_x,
                   int                *drag_start_y,
                   bool               *lmb_pressed,
                   struct ToolOptions *tool_opts,
                   struct World       *world,
                   struct Rect        *world_draw)
{
	unsigned int  b;
	size_t        i;
	size_t        l_start = 3;
	char          pressed;
	unsigned int  report_vals[3];
	unsigned int  x;
	unsigned int  y;

	for (i = 0; i < 3; i++) {
		l_start += string_to_uint(&in[l_start], &report_vals[i]) + 1;
	}
	b = report_vals[0];
	x = report_vals[1];
	y = report_vals[2];
	x -= 1;
	y -= 1;

	l_start -= 1;
	pressed = in[l_start];

	switch (b) {
	case CSI_MB_LEFT:
	case CSI_MB_LEFT_DRAG:
		tool_opts->x = x + world_draw->x;
		tool_opts->y = y + world_draw->y;
		use_tool(delta, *tool_opts, world);

		if ('M' == pressed) {
			*lmb_pressed = true;
		} else {
			*lmb_pressed = false;
		}
		break;

	case CSI_MB_HOVER:
		tool_opts->x = x + world_draw->x;
		tool_opts->y = y + world_draw->y;
		*lmb_pressed = false;
		break;

	case CSI_MB_MIDDLE:
	case CSI_MB_MIDDLE_DRAG:
		if (x >= (unsigned int) world->w ||
		    y >= (unsigned int) world->h) {
			break;
		}

		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_mat = world->dot[x][y];
			break;

		case TOOL_SPAWNER:
			tool_opts->spawner_mat = world->dot[x][y];
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case CSI_MB_RIGHT:
		*drag_start_x = x + world_draw->x;
		*drag_start_y = y + world_draw->y;
		break;

	case CSI_MB_RIGHT_DRAG:
		world_draw->x = *drag_start_x - x;
		world_draw->y = *drag_start_y - y;

		if (world_draw->x < 0) {
			world_draw->x = 0;
		}
		if (world_draw->y < 0) {
			world_draw->y = 0;
		}
		if (world_draw->x > world->w - world_draw->w) {
			world_draw->x = world->w - world_draw->w;
		}
		if (world_draw->y > world->h - world_draw->h) {
			world_draw->y = world->h - world_draw->h;
		}
		break;

	case CSI_MB_WHEELUP:
		tool_radius_add(1, tool_opts);
		break;

	case CSI_MB_WHEELDOWN:
		tool_radius_add(-1, tool_opts);
		break;
	}
}

void
handle_normal_csi_input(const char         *in,
                        const float         delta,
                        int                *drag_start_x,
                        int                *drag_start_y,
                        bool               *lmb_pressed,
                        struct ToolOptions *tool_opts,
                        struct World       *world,
                        struct Rect        *world_draw)
{
	if (strcmp(in, CSI_KEY_LEFT) == 0) {
		if (tool_opts->x > 0) {
			tool_opts->x -= 1;
			if (tool_opts->x < world_draw->x) {
				world_draw->x -= 1;
			}
		}
	} else if (strcmp(in, CSI_KEY_DOWN) == 0) {
		if (tool_opts->y < world->h - 1) {
			tool_opts->y += 1;
			if (tool_opts->y >= world_draw->y + world_draw->h) {
				world_draw->y += 1;
			}
		}
	} else if (strcmp(in, CSI_KEY_UP) == 0) {
		if (tool_opts->y > 0) {
			tool_opts->y -= 1;
			if (tool_opts->y < world_draw->y) {
				world_draw->y -= 1;
			}
		}
	} else if (strcmp(in, CSI_KEY_RIGHT) == 0) {
		if (tool_opts->x < world->w - 1) {
			tool_opts->x += 1;
			if (tool_opts->x >= world_draw->x + world_draw->w) {
				world_draw->x += 1;
			}
		}
	} else if (strcmp(in, CSI_KEY_HOME) == 0) {
		tool_opts->x = 0;
		world_draw->x = 0;
	} else if (strcmp(in, CSI_KEY_END) == 0) {
		tool_opts->x = world->w - 1;
		world_draw->x = world->w - world_draw->w;
	} else if (strcmp(in, CSI_KEY_PGUP) == 0) {
		tool_opts->y = 0;
		world_draw->y = 0;
	} else if (strcmp(in, CSI_KEY_PGDOWN) == 0) {
		tool_opts->y = world->h - 1;
		world_draw->y = world->h - world_draw->h;
	} else if (strcmp(in, CSI_KEY_CTRLHOME) == 0) {
		tool_opts->x = 0;
		tool_opts->y = 0;
		world_draw->x = 0;
		world_draw->y = 0;
	} else if (strcmp(in, CSI_KEY_CTRLEND) == 0) {
		tool_opts->x = world->w - 1;
		tool_opts->y = world->h - 1;
		world_draw->x = world->w - world_draw->w;
		world_draw->y = world->h - world_draw->h;
	} else if (in[1] == '[' &&
	           in[2] == '<') {
		handle_mouse_input(in,
		                   delta,
		                   drag_start_x,
		                   drag_start_y,
		                   lmb_pressed,
		                   tool_opts,
		                   world,
		                   world_draw);
	}
}

void
handle_resize(const size_t            cmdline_len,
              size_t                 *cmdline_shift,
              char                  **display,
              size_t                 *display_size,
              const size_t            dot_depth,
              const enum InputMode    input_mode,
              const char             *ip_address,
              size_t                 *statusbar_elems,
              enum StatusbarElement  *statusbar_elem,
              int                    *win_w,
              int                    *win_h,
              const struct World      world,
              struct Rect            *world_draw,
              int                    *world_draw_space_w,
              int                    *world_draw_space_h,
              const char             *world_name)
{
	size_t             a, b;
	char               buf[BUF_SIZE];
	size_t             buf_len;
	struct ToolOptions maxcoords_to = {
		.x = 999,
		.y = 999,
	};
	size_t             new_display_size;
	size_t             statusbar_len = 0;
	size_t             statusbar_max_elems = 0;
	struct winsize     ws;

	ws = CSI_get_size();
	if (*win_w != ws.ws_col ||
	    *win_h != ws.ws_row) {
		*win_w = ws.ws_col;
		*win_h = ws.ws_row;

		world_draw->x = 0;
		world_draw->y = 0;

		if (world.w > *win_w) {
			world_draw->w = *win_w;
		} else {
			world_draw->w = world.w;
			*world_draw_space_w = *win_w - world.w;
		}

		if (world.h > *win_h - 2) {
			world_draw->h = *win_h - 2;
		} else {
			world_draw->h = world.h;
			*world_draw_space_h = *win_h - 2 - world.h;
		}

		new_display_size = (size_t) ((float) *win_w *
		                             (float) *win_h *
		                             (float) DISPLAY_SIZE_MODIFIER) *
		                   dot_depth;
		if (new_display_size > *display_size) {
			*display_size = new_display_size;
			*display = realloc(*display, *display_size);
		}

		for (a = 0; a < ARRSIZE(STATUSBAR_DISPLAY_PRIORITY); a++) {
			buf[0] = '\0';
			/* Here it is important to render the biggest possible
			 * thing, unless it's not expected to change.
			 * Only in that case use real data.
			 */
			statusbar_len += render_statusbar_display(buf,
		                                                  BUF_SIZE,
		                                                  ip_address,
		                                                  false,
		                                                  STATUSBAR_DISPLAY_PRIORITY[a],
		                                                  120.0,
		                                                  true,
		                                                  maxcoords_to,
		                                                  world_name);

			if (statusbar_len > (size_t) *win_w) {
				break;
			}

			statusbar_len += strlen(STATUSBAR_SEPARATOR);
		}
		statusbar_max_elems = a;
		*statusbar_elems = 0;

		for (a = 0; a < ARRSIZE(STATUSBAR_DISPLAY_PRIORITY); a++) {
			for (b = 0; b < statusbar_max_elems; b++) {
				if (STATUSBAR_DISPLAY_ORDER[a] == STATUSBAR_DISPLAY_PRIORITY[b]) {
					statusbar_elem[*statusbar_elems] = STATUSBAR_DISPLAY_ORDER[a];
					*statusbar_elems += 1;
				}
			}
		}

		if (input_mode == IM_COMMAND) {
			handle_cmdline_shift(cmdline_len,
			                     cmdline_shift,
			                     *win_w);
		}
	}
}

size_t
render_dot(char               *out,
           const size_t        out_size,
           const struct Rgba   color,
           const struct World  world,
           const int           x,
           const int           y)
{
	size_t written = 0;

	if (world.spawner[x][y] == true) {
		written += CSI_color_to_string(SPAWNER_R,
		                               SPAWNER_G,
		                               SPAWNER_B,
		                               true,
		                               &out[written],
		                               out_size - written);
		out[written] = 'O';
		written += 1;
	} else if (world.dot[x][y] == MAT_NONE) {
		written += CSI_color_to_string(255, 255, 255,
		                               true,
		                               &out[written],
		                               out_size - written);
		out[written] = ' ';
		written += 1;
	} else {
		written += CSI_color_to_string(color.r,
		                               color.g,
		                               color.b,
		                               true,
		                               &out[written],
		                               out_size - written);
		out[written] = DOT_APPEARANCE[world.state[x][y]];
		written += 1;
	}

	out[written] = '\0';
	return written;
}

size_t
render_dot_no_color(char               *out,
                    const struct World  world,
                    const int           x,
                    const int           y)
{
	size_t written = 0;

	if (world.spawner[x][y] == true) {
		out[written] = 'O';
		written += 1;
	} else if (world.dot[x][y] == MAT_NONE) {
		out[written] = ' ';
		written += 1;
	} else {
		out[written] = DOT_APPEARANCE[world.state[x][y]];
		written += 1;
	}

	out[written] = '\0';
	return written;
}

size_t
render_statusbar_display(char                        *out,
                         const size_t                 out_size,
                         const char                  *ip_address,
                         const bool                   paused,
                         const enum StatusbarElement  sbe,
                         const float                  tickrate,
                         const bool                   th_vision,
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
render_tool_hint(char                     *out,
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

size_t
render_world(char               *out,
             const size_t        out_size,
             const size_t        dot_depth,
             const bool          no_color,
             const bool          no_glowcolor,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world,
             const struct Rect   world_draw,
             const int           world_draw_space_w,
             const int           world_draw_space_h)
{
	int    tool_radius = 0;
	int    tool_x1 = 0;
	int    tool_y1 = 0;
	int    tool_x2 = 0;
	int    tool_y2 = 0;
	size_t written = 0;
	int    x, y;

	if (th_vision) {
		if (no_color) {
			DOT_RENDER_LOOP_NO_COLOR
		} else {
			DOT_RENDER_LOOP(get_thermal_dot_color(world, x, y))
		}
	} else if (no_glowcolor) {
		if (no_color) {
			DOT_RENDER_LOOP_NO_COLOR
		} else {
			DOT_RENDER_LOOP(get_normal_dot_color_simple(world, x, y))
		}
	} else {
		if (no_color) {
			DOT_RENDER_LOOP_NO_COLOR
		} else {
			DOT_RENDER_LOOP(get_normal_dot_color(world, x, y))
		}
	}

	switch (tool_opts.sel_tool) {
	case TOOL_BRUSH:
		tool_radius = tool_opts.brush_radius;
		break;
	case TOOL_SPAWNER:
		tool_radius = 0;
		break;
	case TOOL_ERASER:
		tool_radius = tool_opts.eraser_radius;
		break;
	case TOOL_HEATER:
	case TOOL_COOLER:
		tool_radius = tool_opts.thermo_radius;
		break;
	case TOOL_COUNT:
		break;
	}

	tool_x1 = tool_opts.x - tool_radius - world_draw.x;
	if (tool_x1 < 0)
		tool_x1 = 0;

	tool_y1 = tool_opts.y - tool_radius - world_draw.y;
	if (tool_y1 < 0)
		tool_y1 = 0;

	tool_x2 = tool_opts.x + tool_radius + 1 - world_draw.x;
	if (tool_x2 >= world_draw.w)
		tool_x2 = world_draw.w;

	tool_y2 = tool_opts.y + tool_radius + 1 - world_draw.y;
	if (tool_y2 >= world_draw.h)
		tool_y2 = world_draw.h;

	for (x = tool_x1; x < tool_x2; x++) {
		for (y = tool_y1; y < tool_y2; y++) {
			out[((y * world_draw.w) + x + 1) * dot_depth +
			    (y * world_draw_space_w) -
			    1] = '^';
		}
	}

	return written;
}

#else

static int _dummy;

#endif /* SDL_BACKEND */
