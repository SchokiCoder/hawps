// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

#include "ui.h"

struct UI
UI_new(void)
{
	struct UI ret = {
		.toolbox_col = {
			.r = UI_TOOLBOX_R,
			.g = UI_TOOLBOX_G,
			.b = UI_TOOLBOX_B,
			.a = UI_TOOLBOX_A
		},
		.matbox_col = {
			.r = UI_MATBOX_R,
			.g = UI_MATBOX_G,
			.b = UI_MATBOX_B,
			.a = UI_MATBOX_A
		},
	};

	return ret;
}

void
UI_set_wide_layout(
	struct UI *ui,
	const int  win_w,
	const int  win_h)
{
	ui->toolbox.x = win_w / 1.0 * UI_WIDE_TOOL_X;
	ui->toolbox.y = win_h / 1.0 * UI_WIDE_TOOL_Y;
	ui->toolbox.w = win_w / 1.0 * UI_WIDE_TOOL_W;
	ui->toolbox.h = win_h / 1.0 * UI_WIDE_TOOL_H;

	ui->matbox.x = win_w / 1.0 * UI_WIDE_MAT_X;
	ui->matbox.y = win_h / 1.0 * UI_WIDE_MAT_Y;
	ui->matbox.w = win_w / 1.0 * UI_WIDE_MAT_W;
	ui->matbox.h = win_h / 1.0 * UI_WIDE_MAT_H;

	ui->world.x = win_w / 1.0 * UI_WIDE_WORLD_X;
	ui->world.y = win_h / 1.0 * UI_WIDE_WORLD_Y;
	ui->world.w = win_w / 1.0 * UI_WIDE_WORLD_W;
	ui->world.h = win_h / 1.0 * UI_WIDE_WORLD_H;
}

void
UI_draw(
	const struct UI  ui,
	SDL_Surface     *frame)
{
	Uint32 col;

	col = SDL_MapRGBA(frame->format,
	            ui.toolbox_col.r,
		    ui.toolbox_col.g,
		    ui.toolbox_col.b,
		    ui.toolbox_col.a);
	SDL_FillRect(frame, &ui.toolbox, col);

	col = SDL_MapRGBA(frame->format,
	            ui.matbox_col.r,
		    ui.matbox_col.g,
		    ui.matbox_col.b,
		    ui.matbox_col.a);
	SDL_FillRect(frame, &ui.matbox, col);
}
