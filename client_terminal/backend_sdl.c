/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifdef SDL_BACKEND

#include "backend_sdl.h"

#include "generic.h"

void
draw(SDL_Renderer *r)
{
	SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(r);

	SDL_SetRenderDrawColor(r, 255, 0, 255, SDL_ALPHA_OPAQUE);
	SDL_FRect test = {10, 10, 200, 200};
	SDL_RenderRect(r, &test);
	SDL_RenderPresent(r);

	// insert fine dick joke
}

void
handle_button_down(SDL_MouseButtonEvent     e,
                   const float              delta,
                   bool                    *drag,
                   int                     *drag_start_x,
                   int                     *drag_start_y,
                   struct ToolOptions      *tool_opts,
                   struct World            *world,
                   struct Rect             *world_draw)
{
	int x = e.x;
	int y = e.y;

	switch (e.button) {
	case SDL_BUTTON_LEFT:
		tool_opts->x = x + world_draw->x;
		tool_opts->y = y + world_draw->y;
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
	}
}

#else

static int _dummy;

#endif /* SDL_BACKEND */
