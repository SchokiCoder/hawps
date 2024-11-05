// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

#ifndef _WORLD_H
#define _WORLD_H

#include <stdint.h>

enum Mat {
	M_none,
	M_sand,
	M_water
};

struct World {
	int        w;
	int        h;
	enum Mat **dots;
};

enum MatState {
	MS_none,
	MS_static,
	MS_grain,
	MS_liquid,
};

const char *MAT_NAME[] =          {"None",  "Sand",   "Water"};
const int MAT_WEIGHT[] =          {0,       2,        1};
const enum MatState MAT_STATE[] = {MS_none, MS_grain, MS_liquid};
const uint8_t MAT_R[] =           {0,       238,      100};
const uint8_t MAT_G[] =           {0,       217,      100};
const uint8_t MAT_B[] =           {0,       86,       255};

/* Returns 0 on success
 */
int
World_new(
	struct World *wld,
	const int     w,
	const int     h);

void
World_tick(
	struct World *wld);

void
World_free(
	struct World *wld);

#endif /* _WORLD_H */
