// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

#include "ui.h"

#include "world.h"

struct UIBox {
	SDL_Surface  *surface;
	SDL_Rect      rect;
	SDL_Color     bg;
	int           cols;
	int           rows;
	int           n_tiles;
	SDL_Surface **tiles;
	int           n_visible_tiles;
	SDL_Surface **visible_tiles;
	int           cursor;
};

struct UIBox*
UIBox_new(
	void)
{
	struct UIBox *ret = NULL;

	ret = calloc(1, sizeof(struct UIBox));

	return ret;
}

int
UIBox_init(
	struct UIBox   *b,
	const int       cols,
	const int       rows)
{
	b->surface = SDL_CreateRGBSurface(0, b->rect.w, b->rect.h, 32,
	                                  0, 0, 0, 0);
	if (NULL == b->surface) {
		return 1;
	}

	b->cols = cols;
	b->rows = rows;
	b->n_tiles = cols * rows;
	b->tiles = calloc(b->n_tiles, sizeof(SDL_Surface*));
	if (NULL == b->tiles) {
		return 1;
	}

	return 0;
}

void
UIBox_draw(
	struct UIBox *b)
{
	Uint32 bg;
	int i;
	SDL_Rect dest = {
		.x = 0,
		.y = 0,
		.w = b->rect.w / b->cols,
		.h = b->rect.h / b->rows};

	bg = SDL_MapRGBA(b->surface->format,
	                 b->bg.r, b->bg.g, b->bg.b, b->bg.a);
	SDL_FillRect(b->surface, NULL, bg);

	for (i = 0; i < b->n_visible_tiles; i++) {
		dest.x = dest.w * (i % b->cols);
		dest.y = dest.h * (i / b->cols);

		SDL_BlitSurface(b->visible_tiles[i], NULL, b->surface, &dest);
	}
}

void
UIBox_free(
	struct UIBox *b)
{
	int i;

	if (NULL == b) {
		return;
	}

	SDL_FreeSurface(b->surface);

	for (i = 0; i < b->n_tiles; i++) {
		SDL_FreeSurface(b->tiles[i]);
	}

	if (NULL != b->visible_tiles) {
		free(b->visible_tiles);
	}
	b->visible_tiles = NULL;
	b->n_visible_tiles = 0;

	if (NULL != b->tiles) {
		free(b->tiles);
	}
	b->tiles = NULL;
	b->n_tiles = 0;

	free(b);
}

struct UI*
UI_new(void)
{
	struct UI *ret = calloc(1, sizeof(struct UI));
	if (NULL == ret) {
		return ret;
	}

	ret->tools = UIBox_new();
	ret->mats = UIBox_new();
	if (NULL == ret->tools || NULL == ret->mats) {
		return ret;
	}

	ret->tools->bg.r = UI_TOOLBOX_R;
	ret->tools->bg.g = UI_TOOLBOX_G;
	ret->tools->bg.b = UI_TOOLBOX_B;
	ret->tools->bg.a = UI_TOOLBOX_A;

	ret->mats->bg.r = UI_MATBOX_R;
	ret->mats->bg.g = UI_MATBOX_G;
	ret->mats->bg.b = UI_MATBOX_B;
	ret->mats->bg.a = UI_MATBOX_A;

	return ret;
}

int
UI_wide_layout_init(
	struct UI *ui,
	const int  win_w,
	const int  win_h)
{
	int cols;
	int ret = 0;

	ui->tools->rect.x = win_w / 1.0 * UI_WIDE_TOOL_X;
	ui->tools->rect.y = win_h / 1.0 * UI_WIDE_TOOL_Y;
	ui->tools->rect.w = win_w / 1.0 * UI_WIDE_TOOL_W;
	ui->tools->rect.h = win_h / 1.0 * UI_WIDE_TOOL_H;

	ui->mats->rect.x = win_w / 1.0 * UI_WIDE_MAT_X;
	ui->mats->rect.y = win_h / 1.0 * UI_WIDE_MAT_Y;
	ui->mats->rect.w = win_w / 1.0 * UI_WIDE_MAT_W;
	ui->mats->rect.h = win_h / 1.0 * UI_WIDE_MAT_H;

	ui->world.x = win_w / 1.0 * UI_WIDE_WORLD_X;
	ui->world.y = win_h / 1.0 * UI_WIDE_WORLD_Y;
	ui->world.w = win_w / 1.0 * UI_WIDE_WORLD_W;
	ui->world.h = win_h / 1.0 * UI_WIDE_WORLD_H;

	ret = UIBox_init(ui->tools, 3, 1);
	if (0 != ret) {
		return ret;
	}

	cols = M_COUNT / 2;
	if (M_COUNT % 2 != 0.0) {
		cols++;
	}

	ret = UIBox_init(ui->mats, cols, 2);
	if (0 != ret) {
		return ret;
	}

	return 0;
}

void
UI_draw(
	const struct UI  ui,
	SDL_Surface     *frame)
{
	UIBox_draw(ui.tools);
	SDL_BlitSurface(ui.tools->surface, NULL, frame, &ui.tools->rect);

	UIBox_draw(ui.mats);
	SDL_BlitSurface(ui.mats->surface, NULL, frame, &ui.mats->rect);
}

void
UI_free(
	struct UI *ui)
{
	if (NULL == ui) {
		return;
	}

	UIBox_free(ui->tools);
	ui->tools = NULL;

	UIBox_free(ui->mats);
	ui->tools = NULL;

	free(ui);
}
