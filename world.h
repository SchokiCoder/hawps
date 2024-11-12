// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

#ifndef _WORLD_H
#define _WORLD_H

#include <stdint.h>

enum Mat {
	M_none,
	M_sand,
	M_water,
	M_iron
};

struct World {
	int        w;
	int        h;
	enum Mat **dots;       /* convenience pointers to actual data */
	enum Mat  *_dots_data; /* actual data in here */
};

enum MatState {
	MS_none,
	MS_static,
	MS_grain,
	MS_liquid,
};

static const char *MAT_NAME[] =          {"None",  "Sand",   "Water",   "Iron"};
static const float MAT_WEIGHT[] =        {0.0,     1.5,      0.999,     7.874}; /* g/cm³ */
static const enum MatState MAT_STATE[] = {MS_none, MS_grain, MS_liquid, MS_static};
static const uint8_t MAT_R[] =           {0,       238,      100,       200};
static const uint8_t MAT_G[] =           {0,       217,      100,       200};
static const uint8_t MAT_B[] =           {0,       86,       255,       200};

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
