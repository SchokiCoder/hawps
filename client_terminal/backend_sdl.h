/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _BACKEND_SDL_H
#define _BACKEND_SDL_H

#include <hawps_core.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "types.h"

#if defined(__linux__)
static const char *FONTPATH[] = {
	"/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
	"/usr/share/fonts/open-sans/OpenSans-Regular.ttf",
	"/usr/share/fonts/adwaita-sans-fonts/AdwaitaSans-Regular.ttf",
	"/usr/share/fonts/google-noto-sans-cjk-vf-fonts/NotoSansCJK-VF.ttc",
	"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
	"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
	"/usr/share/fonts/truetype/ubuntu/Ubuntu-Th.ttf",
	"/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
};
#elif defined(_WIN32)
static const char *FONTPATH[] = {
	"C:\\Windows\\Fonts\\segoeui.ttf",
	"C:\\Windows\\Fonts\\arial.ttf",
	"C:\\Windows\\Fonts\\cour.ttf",
};
#elif defined(__APPLE__)
static const char *FONTPATH[] = {
	"/System/Library/Fonts/SFNS.ttf",
};
#else
static const char *FONTPATH[] = {
	"/usr/share/fonts/truetype/freefont/FreeSans.ttf",
};
#endif

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
     SDL_Texture              *world_tx);

void
handle_mouse_state(const float           delta,
                   bool                 *drag,
                   int                  *drag_start_x,
                   int                  *drag_start_y,
                   struct ToolOptions   *tool_opts,
                   struct World         *world,
                   SDL_FRect            *world_draw);

#endif /* _BACKEND_SDL_H */
