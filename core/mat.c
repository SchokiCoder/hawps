/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#include <stdlib.h>

#include "mat.h"

#define WEIGHT_FACTOR_LIQUID 0.95
#define WEIGHT_FACTOR_GAS    0.90
#define WEIGHTLOSS_LIMIT_GAS 5000.0

static void (*body_sim) (struct World*, int*, const int);

static void
world_clear_dot(struct World *w,
                const int x,
                const int y);

static void
world_update_dot_from_thermo(struct World *w, 
                             const int x,
                             const int y);

static void
world_sim_to_right(struct World *w,
                   int *x,
                   const int y);

static void
world_sim_to_left(struct World *w,
                  int *x,
                  const int y);

static void
world_sim_chemical_reaction(struct World *w,
                            const int x,
                            const int y,
                            const int dx,
                            const int dy);

static void
world_sim_gravity(struct World *w,
                  const int x,
                  const int y);

static void
world_sim_th_conduction(struct World *w,
                        const int x,
                        const int y,
                        const int x2,
                        const int y2);

static void
world_drop_gas(struct World *w,
               const int x,
               const int y);

static int
world_collapse_gas_stack(struct World *w,
                         const int x,
                         const int y,
                         const int dx,
                         const int dy);

static void
world_drop_grain(struct World *w,
                 const int x,
                 const int y);

static void
world_drop_liquid(struct World *w,
                  const int x,
                  const int y);

static int
world_collapse_liquid_stack(struct World *w,
                            const int x,
                            const int y,
                            const int dx,
                            const int dy);

/* Swaps all properties of two coordinates.
 */
static void
world_swap_dots(struct World *w,
                const int x,
                const int y,
                const int x2,
                const int y2);

struct World
world_new(const int w,
          const int h,
          const float temperature)
{
	int x;
	int y;

	struct World ret = {
		.w =            w,
		.h =            h,
		.dots =         calloc(w, sizeof(enum Mat*)),
		._dots =        calloc(w * h, sizeof(enum Mat)),
		.oxid =         calloc(w, sizeof(float*)),
		._oxid =        calloc(w * h, sizeof(float)),
		.spawner =      calloc(w, sizeof(int*)),
		._spawner =     calloc(w * h, sizeof(int)),
		.spawner_mat =  calloc(w, sizeof(enum Mat*)),
		._spawner_mat = calloc(w * h, sizeof(enum Mat)),
		.states =       calloc(w, sizeof(enum MatState*)),
		._states =      calloc(w * h, sizeof(enum MatState)),
		.thermo =       calloc(w, sizeof(float*)),
		._thermo =      calloc(w * h, sizeof(float)),
		.weights =      calloc(w, sizeof(float*)),
		._weights =     calloc(w * h, sizeof(float))
	};

	for (x = 0; x < w; x++) {
		ret.dots[x] = &ret._dots[x * h];
		ret.oxid[x] = &ret._oxid[x * h];
		ret.spawner[x] = &ret._spawner[x * h];
		ret.spawner_mat[x] = &ret._spawner_mat[x * h];
		ret.states[x] = &ret._states[x * h];
		ret.thermo[x] = &ret._thermo[x * h];
		ret.weights[x] = &ret._weights[x * h];

		for (y = 0; y < h; y++) {
			ret.thermo[x][y] = temperature;
		}
	}

	return ret;
}

int
world_can_displace(struct World *w,
                   const int x,
                   const int y,
                   const int dx,
                   const int dy)
{
	if (MAT_NONE == w->dots[dx][dy]) {
		return 1;
	}

	if (MS_STATIC == w->states[dx][dy]) {
		return 0;
	}

	if (w->weights[dx][dy] < w->weights[x][y]) {
		return 1;
	}

	return 0;
}

static void
world_clear_dot(struct World *w,
                const int x,
                const int y)
{
	w->dots[x][y] = MAT_NONE;
	w->states[x][y] = MS_STATIC;
	w->thermo[x][y] = 0;
}

void
world_update(struct World *w,
             const float spawner_temperature)
{
	int x, y;

	for (x = 0; x < w->w; x++) {
		for (y = 0; y < w->h; y++) {
			if (w->spawner[x][y]) {
				w->dots[x][y] = w->spawner_mat[x][y];
				w->thermo[x][y] = spawner_temperature;
			}

			world_update_dot_from_thermo(w, x, y);
		}
	}
}

static void
world_update_dot_from_thermo(struct World *w, 
                             const int x,
                             const int y)
{
	if (w->thermo[x][y] < MAT_MELT_P[w->dots[x][y]]) {
		w->states[x][y] = MAT_SOLID_S[w->dots[x][y]];
		w->weights[x][y] = MAT_FULL_WEIGHT[w->dots[x][y]];
	} else if (w->thermo[x][y] < MAT_BOIL_P[w->dots[x][y]]) {
		w->states[x][y] = MS_LIQUID;

		if (MAT_SAND == w->dots[x][y]) {
			w->dots[x][y] = MAT_GLASS;
		}

		w->weights[x][y] = MAT_FULL_WEIGHT[w->dots[x][y]] *
		                                   WEIGHT_FACTOR_LIQUID;
	} else {
		w->states[x][y] = MS_GAS;
		w->weights[x][y] = MAT_FULL_WEIGHT[w->dots[x][y]] *
		                                   WEIGHT_FACTOR_GAS;
		w->weights[x][y] -= w->weights[x][y] *
		                    (w->thermo[x][y] -
		                     MAT_BOIL_P[w->dots[x][y]]) /
		                    WEIGHTLOSS_LIMIT_GAS;
	}
}

void
world_use_brush(struct World *w,
                const enum Mat m,
                const float t,
                const int x_c,
                const int y_c,
                const int radius)
{
	int x, y;
	int x1 = x_c - radius;
	int x2 = x_c + radius;
	int y1 = y_c - radius;
	int y2 = y_c + radius;

	if (x1 < 0) {
		x1 = 0;
	}
	if (x2 >= w->w) {
		x2 = w->w - 1;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (y2 >= w->h) {
		y2 = w->h - 1;
	}

	for (x = x1; x <= x2; x++) {
		for (y = y1; y <= y2; y++) {
			w->dots[x][y] = m;
			w->thermo[x][y] = t;
		}
	}
}

void
world_use_eraser(struct World *w,
                 const int x_c,
                 const int y_c,
                 const int radius)
{
	int x, y;
	int x1 = x_c - radius;
	int x2 = x_c + radius;
	int y1 = y_c - radius;
	int y2 = y_c + radius;

	if (x1 < 0) {
		x1 = 0;
	}
	if (x2 >= w->w) {
		x2 = w->w - 1;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (y2 >= w->h) {
		y2 = w->h - 1;
	}

	for (x = x1; x <= x2; x++) {
		for (y = y1; y <= y2; y++) {
			world_clear_dot(w, x, y);
			w->spawner[x][y] = 0;
		}
	}
}

void
world_use_cooler(struct World *w,
                 const float delta,
                 const int x_c,
                 const int y_c,
                 const int radius)
{
	int x, y;
	int x1 = x_c - radius;
	int x2 = x_c + radius;
	int y1 = y_c - radius;
	int y2 = y_c + radius;

	if (x1 < 0) {
		x1 = 0;
	}
	if (x2 >= w->w) {
		x2 = w->w - 1;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (y2 >= w->h) {
		y2 = w->h - 1;
	}

	for (x = x1; x <= x2; x++) {
		for (y = y1; y <= y2; y++) {
			w->thermo[x][y] -= delta;

			if (w->thermo[x][y] < 0.0) {
				w->thermo[x][y] = 0.0;
			}
		}
	}
}

void
world_use_heater(struct World *w,
                 const float delta,
                 const int x_c,
                 const int y_c,
                 const int radius)
{
	int x, y;
	int x1 = x_c - radius;
	int x2 = x_c + radius;
	int y1 = y_c - radius;
	int y2 = y_c + radius;

	if (x1 < 0) {
		x1 = 0;
	}
	if (x2 >= w->w) {
		x2 = w->w - 1;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (y2 >= w->h) {
		y2 = w->h - 1;
	}

	for (x = x1; x <= x2; x++) {
		for (y = y1; y <= y2; y++) {
			w->thermo[x][y] += delta;
		}
	}
}

void
world_sim(struct World *w)
{
	int x, y;

	y = w->h - 1;
	for (x = 1; x <= w->w - 2; x++) {
		if (MAT_NONE == w->dots[x][y]) {
			continue;
		}

		world_sim_th_conduction(w, x, y, x - 1, y);
		world_sim_th_conduction(w, x, y, x + 1, y);
		world_sim_chemical_reaction(w, x, y, x - 1, y);
		world_sim_chemical_reaction(w, x, y, x + 1, y);
		world_sim_chemical_reaction(w, x, y, x, y - 1);
	}

	body_sim = world_sim_to_right;
	for (y = w->h - 2; y > 0; y--) {
		body_sim(w, &x, y);
	}

	y = 0;
	for (x = 1; x <= w->w - 2; x++) {
		if (MAT_NONE == w->dots[x][y]) {
			continue;
		}

		world_sim_th_conduction(w, x, y, x, y + 1);
		world_sim_th_conduction(w, x, y, x - 1, y);
		world_sim_th_conduction(w, x, y, x + 1, y);
		world_sim_chemical_reaction(w, x, y, x, y + 1);
		world_sim_chemical_reaction(w, x, y, x - 1, y);
		world_sim_chemical_reaction(w, x, y, x + 1, y);
		world_sim_gravity(w, x, y);
	}

	x = 0;
	for (y = w->h - 2; y >= 0; y--) {
		if (MAT_NONE == w->dots[x][y]) {
			continue;
		}

		world_sim_th_conduction(w, x, y, x, y + 1);
		world_sim_chemical_reaction(w, x, y, x, y + 1);
		world_sim_chemical_reaction(w, x, y, x + 1, y);
		world_sim_gravity(w, x, y);
	}

	x = w->w - 1;
	for (y = w->h - 2; y >= 0; y--) {
		if (MAT_NONE == w->dots[x][y]) {
			continue;
		}

		world_sim_th_conduction(w, x, y, x, y + 1);
		world_sim_chemical_reaction(w, x, y, x, y + 1);
		world_sim_chemical_reaction(w, x, y, x - 1, y);
		world_sim_gravity(w, x, y);
	}
}

static void
world_sim_to_right(struct World *w,
                   int *x,
                   const int y)
{
	for (*x = 1; *x <= w->w - 2; *x += 1) {
		if (MAT_NONE == w->dots[*x][y]) {
			continue;
		}

		world_sim_th_conduction(w, *x, y, *x, y + 1);
		world_sim_th_conduction(w, *x, y, *x - 1, y);
		world_sim_th_conduction(w, *x, y, *x + 1, y);
		world_sim_chemical_reaction(w, *x, y, *x, y + 1);
		world_sim_chemical_reaction(w, *x, y, *x, y - 1);
		world_sim_chemical_reaction(w, *x, y, *x - 1, y);
		world_sim_chemical_reaction(w, *x, y, *x + 1, y);
		world_sim_gravity(w, *x, y);
	}
	body_sim = world_sim_to_left;
}

static void
world_sim_to_left(struct World *w,
                  int *x,
                  const int y)
{
	for (*x = w->w - 2; *x >= 1; *x -= 1) {
		if (MAT_NONE == w->dots[*x][y]) {
			continue;
		}

		world_sim_th_conduction(w, *x, y, *x, y + 1);
		world_sim_th_conduction(w, *x, y, *x - 1, y);
		world_sim_th_conduction(w, *x, y, *x + 1, y);
		world_sim_chemical_reaction(w, *x, y, *x, y + 1);
		world_sim_chemical_reaction(w, *x, y, *x, y - 1);
		world_sim_chemical_reaction(w, *x, y, *x - 1, y);
		world_sim_chemical_reaction(w, *x, y, *x + 1, y);
		world_sim_gravity(w, *x, y);
	}
	body_sim = world_sim_to_right;
}

static void
world_sim_chemical_reaction(struct World *w,
                            const int x,
                            const int y,
                            const int dx,
                            const int dy)
{
	enum Mat op1, op2;
	float th;

	if (MAT_OXID_TH[w->dots[x][y]] > 0.0) {
		if (MAT_OXYGEN == w->dots[dx][dy]) {
			if (w->thermo[x][y] > MAT_IGN_P[w->dots[x][y]]) {
				th = MAT_OXID_TH[w->dots[x][y]] *
				     MAT_OXID_SPD[w->dots[x][y]] /
				     2.0;

				w->oxid[x][y] += MAT_OXID_SPD[w->dots[x][y]];
				w->thermo[x][y] += th;
				w->thermo[dx][dy] += th;

				if (w->oxid[x][y] >= 1.0) {
					op1 = MAT_OXID_PRDCT1[w->dots[x][y]];
					op2 = MAT_OXID_PRDCT2[w->dots[x][y]];

					w->dots[x][y] = op1;
					w->dots[dx][dy] = op2;
					w->oxid[x][y] = 0.0;
				}
			}
		}
	}
}

static void
world_sim_gravity(struct World *w,
                  const int x,
                  const int y)
{
	switch (w->states[x][y]) {
	case MS_GAS:
		world_drop_gas(w, x, y);
		break;

	case MS_GRAIN:
		world_drop_grain(w, x, y);
		break;

	case MS_LIQUID:
		world_drop_liquid(w, x, y);
		break;

	default:
		break;
	}
}

static void
world_sim_th_conduction(struct World *w,
                        const int x,
                        const int y,
                        const int x2,
                        const int y2)
{
	float c1, c2, combCond;

	if (MAT_NONE == w->dots[x2][y2]) {
		return;
	}

	combCond = (MAT_TH_COND[w->dots[x][y]] + MAT_TH_COND[w->dots[x2][y2]]) / 2;
	c1 = (w->thermo[x2][y2] - w->thermo[x][y]) * combCond;
	c2 = (w->thermo[x][y] - w->thermo[x2][y2]) * combCond;

	w->thermo[x][y] += c1;
	w->thermo[x2][y2] += c2;
}

static void
world_drop_gas(struct World *w,
               const int x,
               const int y)
{
	int dx, dy;

	dx = x;
	dy = y + 1;
	if (world_can_displace(w, x, y, dx, dy)) {
		world_swap_dots(w, x, y, dx, dy);
		return;
	}

	dy = y + 1;
	for (dx = x - 1; dx >= 0; dx--) {
		if (world_collapse_gas_stack(w, x, y, dx, dy)) {
			break;
		}
	}
	for (dx = x + 1; dx < w->w; dx++) {
		if (world_collapse_gas_stack(w, x, y, dx, dy)) {
			break;
		}
	}
}

static int
world_collapse_gas_stack(struct World *w,
                         const int x,
                         const int y,
                         const int dx,
                         const int dy)
{
	if (world_can_displace(w, x, y, dx, dy)) {
		world_swap_dots(w, x, y, dx, dy);
		return 0;
	}

	if (MS_STATIC == w->states[dx][dy] ||
	    MS_GRAIN == w->states[dx][dy]) {
		return 1;
	}

	return 0;
}

static void
world_drop_grain(struct World *w,
                 const int x,
                 const int y)
{
	int dx, dy;

	dx = x;
	dy = y + 1;
	if (world_can_displace(w, x, y, dx, dy)) {
		world_swap_dots(w, x, y, dx, dy);
		return;
	}

	if (x - 1 >= 0) {
		dx = x - 1;
		dy = y + 1;

		if (world_can_displace(w, x, y, dx, dy)) {
			world_swap_dots(w, x, y, dx, dy);
			return;
		}
	}
	if (x + 1 < w->w) {
		dx = x + 1;
		dy = y + 1;

		if (world_can_displace(w, x, y, dx, dy)) {
			world_swap_dots(w, x, y, dx, dy);
			return;
		}
	}
}

static void
world_drop_liquid(struct World *w,
                  const int x,
                  const int y)
{
	int dx, dy;

	dx = x;
	dy = y + 1;
	if (world_can_displace(w, x, y, dx, dy)) {
		world_swap_dots(w, x, y, dx, dy);
		return;
	}

	dy = y + 1;
	for (dx = x - 1; dx >= 0; dx--) {
		if (world_collapse_liquid_stack(w, x, y, dx, dy)) {
			break;
		}
	}
	for (dx = x + 1; dx < w->w; dx++) {
		if (world_collapse_liquid_stack(w, x, y, dx, dy)) {
			break;
		}
	}
}

static int
world_collapse_liquid_stack(struct World *w,
                            const int x,
                            const int y,
                            const int dx,
                            const int dy)
{
	if (world_can_displace(w, x, y, dx, dy)) {
		world_swap_dots(w, x, y, dx, dy);
		return 0;
	}

	if (MS_STATIC == w->states[dx][dy] ||
	    MS_GRAIN == w->states[dx][dy]) {
		return 1;
	}

	return 0;
}

static void
world_swap_dots(struct World *w,
                const int x,
                const int y,
                const int x2,
                const int y2)
{
	enum Mat      tmpM = w->dots[x][y];
	float         tmpO = w->oxid[x][y];
	enum MatState tmpS = w->states[x][y];
	float         tmpT = w->thermo[x][y];

	w->dots[x][y] = w->dots[x2][y2];
	w->oxid[x][y] = w->oxid[x2][y2];
	w->states[x][y] = w->states[x2][y2];
	w->thermo[x][y] = w->thermo[x2][y2];

	w->dots[x2][y2] = tmpM;
	w->oxid[x2][y2] = tmpO;
	w->states[x2][y2] = tmpS;
	w->thermo[x2][y2] = tmpT;
}
