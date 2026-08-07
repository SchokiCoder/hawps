/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _BACKEND_SDL_H
#define _BACKEND_SDL_H

#include <hawps_core.h>
#include <SDL3/SDL.h>

#include "types.h"

void
draw(SDL_Renderer *r);

void
handle_button_down(SDL_MouseButtonEvent     e,
                   const float              delta,
                   bool                    *drag,
                   int                     *drag_start_x,
                   int                     *drag_start_y,
                   struct ToolOptions      *tool_opts,
                   struct World            *world,
                   struct Rect             *world_draw);

#endif /* _BACKEND_SDL_H */
