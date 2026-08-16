/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _BACKEND_SDL_H
#define _BACKEND_SDL_H

#include <hawps_core.h>
#include <SDL3/SDL.h>

#include "types.h"

void
draw(const bool                no_glowcolor,
     const bool                th_vision,
     const struct ToolOptions  tool_opts,
     SDL_Renderer             *r,
     const struct World        world,
     const SDL_FRect           world_draw,
     SDL_Texture              *world_tx);

void
handle_button_down(SDL_MouseButtonEvent     e,
                   const float              delta,
                   bool                    *drag,
                   int                     *drag_start_x,
                   int                     *drag_start_y,
                   struct ToolOptions      *tool_opts,
                   struct World            *world,
                   SDL_FRect               *world_draw);

#endif /* _BACKEND_SDL_H */
