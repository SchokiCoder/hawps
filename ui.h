// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

#ifndef _UI_H
#define _UI_H

#include <SDL.h>

/* 0 ... 255 */
#define UI_TOOLBOX_R 50
#define UI_TOOLBOX_G 50
#define UI_TOOLBOX_B 50
#define UI_TOOLBOX_A 255
#define UI_MATBOX_R  UI_TOOLBOX_R
#define UI_MATBOX_G  (UI_TOOLBOX_G + 100) // TODO: remove the + 100
#define UI_MATBOX_B  UI_TOOLBOX_B
#define UI_MATBOX_A  UI_TOOLBOX_A

/* percentages of window size from 0.0 to 1.0 */
#define UI_WIDE_TOOL_X  (0.0)
#define UI_WIDE_TOOL_Y  (0.0)
#define UI_WIDE_TOOL_W  (0.15)
#define UI_WIDE_TOOL_H  (0.05)
#define UI_WIDE_MAT_X   (UI_WIDE_TOOL_X)
#define UI_WIDE_MAT_Y   (UI_WIDE_TOOL_Y + UI_WIDE_TOOL_H)
#define UI_WIDE_MAT_W   (UI_WIDE_TOOL_W)
#define UI_WIDE_MAT_H   (1.0 - UI_WIDE_TOOL_H)
#define UI_WIDE_WORLD_X (UI_WIDE_TOOL_X + UI_WIDE_TOOL_W)
#define UI_WIDE_WORLD_Y (0.0)
#define UI_WIDE_WORLD_W (1.0 - UI_WIDE_TOOL_W)
#define UI_WIDE_WORLD_H (1.0)

struct UIBox;

struct UI {
	struct UIBox *tools;
	struct UIBox *mats;
	SDL_Rect     world;
};

struct UI*
UI_new(
	void);

int
UI_wide_layout_init(
	struct UI *ui,
	const int  win_w,
	const int  win_h);

void
UI_draw(
	const struct UI  ui,
	SDL_Surface     *frame);

void
UI_free(
	struct UI *ui);

#endif /* _UI_H */
