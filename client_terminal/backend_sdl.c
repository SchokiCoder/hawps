/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifdef SDL_BACKEND

#include "backend_sdl.h"

#include "config.h"
#include "generic.h"
#include "str.h"

/* Macros
 */

#define DOT_RENDER_LOOP(COLOR_FN_CALL) \
	for (x = 0; x < world.w; x++) { \
		for (y = 0; y < world.h; y++) { \
			dc = COLOR_FN_CALL; \
			SDL_SetRenderDrawColor(r, dc.r, dc.g, dc.b, dc.a); \
			SDL_RenderPoint(r, x, y); \
		} \
	}

/* Function declarations
 */

void
handle_button(const float           delta,
              bool                 *drag,
              int                  *drag_start_x,
              int                  *drag_start_y,
              SDL_MouseButtonFlags  mbf,
              struct ToolOptions   *tool_opts,
              struct World         *world);

void
render_world(const bool          no_glowcolor,
             SDL_Renderer       *r,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world);

/* Function definitions
 */

void
draw(const char               *cmdline,
     const char               *feedback,
     TTF_Font                 *font,
     const enum InputMode      input_mode,
     const char               *ip_address,
     const bool                no_glowcolor,
     const bool                paused,
     const bool                th_vision,
     const float               tickrate,
     const struct ToolOptions  tool_opts,
     SDL_Renderer             *r,
     const struct World        world,
     const SDL_FRect           world_draw,
     const char               *world_name,
     SDL_Texture              *world_tx)
{
	SDL_Color    bg;
	char         cmdl[CMDLINE_SIZE];
	size_t       cmdl_len = 0;
	SDL_FRect    cmdlr;
	SDL_Surface *cmdls;
	SDL_Texture *cmdlt;
	SDL_Color    fg;
	size_t       i;
	char         sb[CMDLINE_SIZE];
	size_t       sb_len = 0;
	SDL_FRect    sbr;
	SDL_Surface *sbs;
	SDL_Texture *sbt;
	SDL_Window  *win;
	int          win_w;
	int          win_h;

	cmdl[0] = '\0';
	sb[0] = '\0';
	win = SDL_GetRenderWindow(r);
	SDL_GetWindowSize(win, &win_w, &win_h);
	sbr = (SDL_FRect) {
		.x = 0,
		.y = win_h - (SDL_FONT_SIZE * 2),
		.w = 0,
		.h = SDL_FONT_SIZE,
	};
	cmdlr = (SDL_FRect) {
		.x = 0,
		.y = sbr.y + sbr.h,
		.w = 0,
		.h = SDL_FONT_SIZE,
	};

	SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(r);

	if (th_vision) {
		bg.r = THERMAL_VISION_R;
		bg.g = THERMAL_VISION_G;
		bg.b = THERMAL_VISION_B;
		bg.a = SDL_ALPHA_OPAQUE;
	} else {
		bg.r = 0;
		bg.g = 0;
		bg.b = 0;
		bg.a = SDL_ALPHA_OPAQUE;
	}
	fg.r = 255 - bg.r;
	fg.g = 255 - bg.g;
	fg.b = 255 - bg.b;
	fg.a = SDL_ALPHA_OPAQUE;

	SDL_SetRenderDrawColor(r, bg.r, bg.g, bg.b, bg.a);
	SDL_RenderClear(r);

	SDL_SetRenderTarget(r, world_tx);
	SDL_RenderClear(r);
	render_world(no_glowcolor, r, th_vision, tool_opts, world);
	SDL_SetRenderTarget(r, NULL);

	SDL_RenderTexture(r, world_tx, NULL, &world_draw);

	// TODO impl sbe priority system
	i = 0;
	while (1) {
		sb_len += write_statusbar_elem(&sb[sb_len],
		                               CMDLINE_SIZE - sb_len,
		                               ip_address,
		                               paused,
		                               STATUSBAR_DISPLAY_ORDER[i],
		                               th_vision,
		                               tickrate,
		                               tool_opts,
		                               world_name);

		i++;
		if (i >= ARRSIZE(STATUSBAR_DISPLAY_ORDER)) {
			break;
		}

		sb_len += string_cat(sb,
		                     CMDLINE_SIZE,
		                     sb_len,
		                     STATUSBAR_SEPARATOR);
	}
	sbs = TTF_RenderText_LCD(font, sb, sb_len, fg, bg);
	sbt = SDL_CreateTextureFromSurface(r, sbs);
	sbr.w = sbt->w;
	SDL_RenderTexture(r, sbt, NULL, &sbr);

	switch (input_mode) {
	case IM_NORMAL:
		if (feedback != NULL) {
			cmdl_len += string_cat(cmdl,
			                       CMDLINE_SIZE,
			                       cmdl_len,
			                       feedback);
			break;
		}

		cmdl_len += write_tool_hint(&cmdl[cmdl_len],
		                            CMDLINE_SIZE - cmdl_len,
		                            tool_opts);
		break;

	case IM_COMMAND:
		cmdl[0] = CMDLINE_INDICATOR;
		cmdl_len += 1;
		cmdl[1] = '\0';

		cmdl_len += string_cat(cmdl,
		                       CMDLINE_SIZE,
		                       cmdl_len,
		                       cmdline);

		cmdl[cmdl_len] = CMDLINE_CURSOR;
		cmdl_len += 1;
		cmdl[cmdl_len] = '\0';
		break;
	}
	cmdls = TTF_RenderText_LCD(font, cmdl, cmdl_len, fg, bg);
	cmdlt = SDL_CreateTextureFromSurface(r, cmdls);
	cmdlr.w = cmdlt->w;
	if (cmdlr.w > win_w) {
		cmdlr.x = win_w - cmdlr.w;
	}
	SDL_RenderTexture(r, cmdlt, NULL, &cmdlr);

	SDL_RenderPresent(r);

	SDL_DestroySurface(cmdls);
	SDL_DestroyTexture(cmdlt);
	SDL_DestroySurface(sbs);
	SDL_DestroyTexture(sbt);
}

void
handle_button(const float           delta,
              bool                 *drag,
              int                  *drag_start_x,
              int                  *drag_start_y,
              SDL_MouseButtonFlags  mbf,
              struct ToolOptions   *tool_opts,
              struct World         *world)
{
	int x = tool_opts->x;
	int y = tool_opts->y;

	switch (mbf) {
	case SDL_BUTTON_LEFT:
		use_tool(delta, *tool_opts, world);
		break;

	case SDL_BUTTON_MIDDLE:
		if (x >= world->w ||
		    y >= world->h) {
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

	case SDL_BUTTON_RIGHT:
		*drag = true;
		*drag_start_x = x;
		*drag_start_y = y;
		break;

	case SDL_BUTTON_X1:
	case SDL_BUTTON_X2:
		break;

	default:
		*drag = false;
		break;
	}
}

void
render_world(const bool          no_glowcolor,
             SDL_Renderer       *r,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world)
{
	int         alpha;
	struct Rgba dc;
	int         tool_radius = 0;
	SDL_FRect   tool;
	int         x, y;

	if (th_vision) {
		for (x = 0; x < world.w; x++) {
			for (y = 0; y < world.h; y++) {
				/* the following hack is sponsored
				 * by optimization (we avoid an if) */
				alpha = 255 * world.dot[x][y];

				dc = get_thermal_dot_color(world, x, y);
				dc.a = alpha;
				SDL_SetRenderDrawColor(r, dc.r, dc.g, dc.b, dc.a);
				SDL_RenderPoint(r, x, y);
			}
		}
	} else if (no_glowcolor) {
		DOT_RENDER_LOOP(get_normal_dot_color_simple(world, x, y))
	} else {
		DOT_RENDER_LOOP(get_normal_dot_color(world, x, y))
	}

	SDL_SetRenderDrawColor(r, SPAWNER_R, SPAWNER_G, SPAWNER_B, SPAWNER_A);
	for (x = 0; x < world.w; x++) {
		for (y = 0; y < world.h; y++) {
			if (world.spawner[x][y]) {
				SDL_RenderPoint(r, x, y);
			}
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

	tool.x = tool_opts.x - tool_radius;
	tool.y = tool_opts.y - tool_radius;
	tool.w = tool_radius * 2 + 1;
	tool.h = tool_radius * 2 + 1;

	SDL_SetRenderDrawColor(r,
	                       TOOL_HOVER_R,
	                       TOOL_HOVER_G,
	                       TOOL_HOVER_B,
	                       TOOL_HOVER_A);
	SDL_RenderFillRect(r, &tool);
}

void
handle_mouse_state(const float           delta,
                   bool                 *drag,
                   int                  *drag_start_x,
                   int                  *drag_start_y,
                   struct ToolOptions   *tool_opts,
                   struct World         *world,
                   SDL_FRect            *world_draw)
{
	SDL_MouseButtonFlags mbf;
	int x, y;

	mbf = SDL_GetMouseState(NULL, NULL);
	x = tool_opts->x;
	y = tool_opts->y;

	if (*drag) {
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
	}

	handle_button(delta,
	              drag,
	              drag_start_x,
	              drag_start_y,
	              mbf,
	              tool_opts,
	              world);
}

#else

static int _dummy;

#endif /* SDL_BACKEND */
