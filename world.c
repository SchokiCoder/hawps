// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

#include "world.h"

#include <stdlib.h>

void
apply_gravity(
	struct World *wld);

int
can_liquid_displace(
	enum Mat src, enum Mat dest);

int
can_grain_displace(
	enum Mat src, enum Mat dest);

void
drop_grain(
	struct World *wld,
	int x,
	int y);

void
drop_liquid(
	struct World *wld,
	int x,
	int y);

void
apply_gravity(
	struct World *wld)
{
	int x, y;

	for (x = 0; x < wld->w; x++) {
		for (y = wld->h - 2; y >= 0; y--) {
			switch (MAT_STATE[wld->dots[x][y]]) {
			case MS_none:
			case MS_static:
				break;

			case MS_grain:
				drop_grain(wld, x, y);
				break;

			case MS_liquid:
				drop_liquid(wld, x, y);
				break;
			}
		}
	}
}

int
can_liquid_displace(
	enum Mat src, enum Mat dest)
{
	switch (MAT_STATE[dest]) {
	case MS_none:
		return 1;

	default:
		if (MAT_WEIGHT[dest] < MAT_WEIGHT[src]) {
			return 1;
		}
		break;
	}

	return 0;
}

int
can_grain_displace(
	enum Mat src, enum Mat dest)
{
	switch (MAT_STATE[dest]) {
	case MS_none:
		return 1;

	case MS_liquid:
		if (MAT_WEIGHT[dest] < MAT_WEIGHT[src]) {
			return 1;
		}
		break;

	default:
		break;
	}

	return 0;
}

void
drop_grain(
	struct World *wld,
	int x,
	int y)
{
	enum Mat *below;
	enum Mat *cur;
	enum Mat  tmp;

	cur = &wld->dots[x][y];
	below = &wld->dots[x][y + 1];

	if (can_grain_displace(*cur, *below)) {
		tmp = *below;
		*below = *cur;
		*cur = tmp;
		return;
	}

	if (x - 1 >= 0) {
		below = &wld->dots[x - 1][y + 1];

		if (can_grain_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}
	}
	if (x + 1 < wld->w) {
		below = &wld->dots[x + 1][y + 1];

		if (can_grain_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}
	}
}

void
drop_liquid(
	struct World *wld,
	int x,
	int y)
{
	int bx;
	enum Mat *below;
	enum Mat *cur;
	enum Mat  tmp;

	cur = &wld->dots[x][y];
	below = &wld->dots[x][y + 1];

	if (can_liquid_displace(*cur, *below)) {
		tmp = *below;
		*below = *cur;
		*cur = tmp;
		return;
	}

	for (bx = x - 1; bx >= 0; bx--) {
		below = &wld->dots[bx][y + 1];
		if (can_liquid_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}

		if (MAT_STATE[*below] == MS_static) {
			break;
		}
	}
	for (bx = x + 1; bx < wld->w; bx++) {
		below = &wld->dots[bx][y + 1];
		if (can_liquid_displace(*cur, *below)) {
			tmp = *below;
			*below = *cur;
			*cur = tmp;
			return;
		}

		if (MAT_STATE[*below] == MS_static) {
			break;
		}
	}
}

int
World_new(
	struct World *wld,
	const int     w,
	const int     h)
{
	int x, y;

	wld->w = w;
	wld->h = h;

	wld->_dots_data = malloc(sizeof(enum Mat*) * wld->w * wld->h);
	if (NULL == wld->_dots_data) {
		return 1;
	}

	wld->dots = malloc(sizeof(enum Mat*) * wld->w);

	for (x = 0; x < wld->w; x++) {
		wld->dots[x] = &wld->_dots_data[wld->h * x];

		for (y = 0; y < wld->h; y++) {
			wld->dots[x][y] = M_none;
		}
	}

	return 0;
}

void
World_tick(
	struct World *wld)
{
	apply_gravity(wld);
}

void
World_free(
	struct World *wld)
{
	if (NULL != wld->dots) {
		free(wld->dots);
	}

	if (NULL != wld->_dots_data) {
		free(wld->_dots_data);
	}
}
