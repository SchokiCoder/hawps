/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <stdbool.h>
#include <stdlib.h>

#include "hawps_world.h"

#define WEIGHT_FACTOR_LIQUID 0.95
#define WEIGHT_FACTOR_GAS    0.90
#define WEIGHTLOSS_LIMIT_GAS 5000.0

static bool
world_collapse_gas_stack(struct World *w,
                         const int     x,
                         const int     y,
                         const int     dx,
                         const int     dy);

static bool
world_collapse_liquid_stack(struct World *w,
                            const int     x,
                            const int     y,
                            const int     dx,
                            const int     dy);

static void
world_drop_gas(struct World *w,
               const int     x,
               const int     y);

static void
world_drop_grain(struct World *w,
                 const int     x,
                 const int     y);

static void
world_drop_liquid(struct World *w,
                  const int     x,
                  const int     y);

static void
world_sim_to_right(struct World *w,
                   int          *x,
                   const int     y);

static void
world_sim_to_left(struct World *w,
                  int          *x,
                  const int     y);

static void
world_sim_chemical_reaction(struct World *w,
                            const int     x,
                            const int     y,
                            const int     dx,
                            const int     dy);

static void
world_sim_gravity(struct World *w,
                  const int     x,
                  const int     y);

static void
world_sim_th_conduction(struct World *w,
                        const int     x,
                        const int     y,
                        const int     x2,
                        const int     y2);

/* Swaps all properties of two coordinates.
 */
static void
world_swap_dots(struct World *w,
                const int     x,
                const int     y,
                const int     x2,
                const int     y2);

static void
world_update_dot_from_thermo(struct World *w,
                             const int     x,
                             const int     y);

struct World
world_new(const int   w,
          const int   h,
          const float temperature)
{
	int x;
	int y;

	struct World ret = {
		.w =            w,
		.h =            h,
		.dissol =       calloc(w, sizeof(float*)),
		._dissol =      calloc(w * h, sizeof(float)),
		.dot =          calloc(w, sizeof(enum Mat*)),
		._dot =         calloc(w * h, sizeof(enum Mat)),
		.oxid =         calloc(w, sizeof(float*)),
		._oxid =        calloc(w * h, sizeof(float)),
		.spawner =      calloc(w, sizeof(int*)),
		._spawner =     calloc(w * h, sizeof(int)),
		.spawner_mat =  calloc(w, sizeof(enum Mat*)),
		._spawner_mat = calloc(w * h, sizeof(enum Mat)),
		.state =        calloc(w, sizeof(enum MatState*)),
		._state =       calloc(w * h, sizeof(enum MatState)),
		.thermo =       calloc(w, sizeof(float*)),
		._thermo =      calloc(w * h, sizeof(float)),
		.weight =       calloc(w, sizeof(float*)),
		._weight =      calloc(w * h, sizeof(float))
	};

	for (x = 0; x < w; x++) {
		ret.dissol[x] = &ret._dissol[x * h];
		ret.dot[x] = &ret._dot[x * h];
		ret.oxid[x] = &ret._oxid[x * h];
		ret.spawner[x] = &ret._spawner[x * h];
		ret.spawner_mat[x] = &ret._spawner_mat[x * h];
		ret.state[x] = &ret._state[x * h];
		ret.thermo[x] = &ret._thermo[x * h];
		ret.weight[x] = &ret._weight[x * h];

		for (y = 0; y < h; y++) {
			ret.thermo[x][y] = temperature;
		}
	}

	return ret;
}

bool
world_can_displace(struct World *w,
                   const int     x,
                   const int     y,
                   const int     dx,
                   const int     dy)
{
	if (MAT_NONE == w->dot[dx][dy]) {
		return true;
	}

	switch (w->state[dx][dy]) {
	case MS_STATIC:
		return false;
		break;

	case MS_GRAIN:
		if (w->state[x][y] == MS_GRAIN) {
			return false;
		}
		break;

	case MS_LIQUID:
	case MS_GAS:
		if (w->weight[dx][dy] < w->weight[x][y]) {
			return true;
		}
		break;

	case MS_COUNT:
		break;
	}

	return false;
}

void
world_clear_dot(struct World *w,
                const int     x,
                const int     y)
{
	w->dot[x][y] = MAT_NONE;
	w->state[x][y] = MS_STATIC;
	w->thermo[x][y] = 0;
}

static bool
world_collapse_gas_stack(struct World *w,
                         const int     x,
                         const int     y,
                         const int     dx,
                         const int     dy)
{
	if (world_can_displace(w, x, y, dx, dy)) {
		world_swap_dots(w, x, y, dx, dy);
		return false;
	}

	if (MS_STATIC == w->state[dx][dy] ||
	    MS_GRAIN == w->state[dx][dy]) {
		return true;
	}

	return false;
}

static bool
world_collapse_liquid_stack(struct World *w,
                            const int     x,
                            const int     y,
                            const int     dx,
                            const int     dy)
{
	if (world_can_displace(w, x, y, dx, dy)) {
		world_swap_dots(w, x, y, dx, dy);
		return false;
	}

	if (MS_STATIC == w->state[dx][dy] ||
	    MS_GRAIN == w->state[dx][dy]) {
		return true;
	}

	return false;
}

static void
world_drop_gas(struct World *w,
               const int     x,
               const int     y)
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

static void
world_drop_grain(struct World *w,
                 const int     x,
                 const int     y)
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
                  const int     x,
                  const int     y)
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

void
world_sim(struct World *w)
{
	int x, y;

	y = w->h - 1;
	for (x = 1; x <= w->w - 2; x++) {
		if (MAT_NONE == w->dot[x][y]) {
			continue;
		}

		world_sim_th_conduction(w, x, y, x - 1, y);
		world_sim_th_conduction(w, x, y, x + 1, y);
		world_sim_chemical_reaction(w, x, y, x - 1, y);
		world_sim_chemical_reaction(w, x, y, x + 1, y);
		world_sim_chemical_reaction(w, x, y, x, y - 1);
	}

	y = w->h - 2;
	while (1) {
		if (y <= 0) {
			break;
		}
		world_sim_to_right(w, &x, y);
		y -= 1;

		if (y <= 0) {
			break;
		}
		world_sim_to_left(w, &x, y);
		y -= 1;
	}

	y = 0;
	for (x = 1; x <= w->w - 2; x++) {
		if (MAT_NONE == w->dot[x][y]) {
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
		if (MAT_NONE == w->dot[x][y]) {
			continue;
		}

		world_sim_th_conduction(w, x, y, x, y + 1);
		world_sim_chemical_reaction(w, x, y, x, y + 1);
		world_sim_chemical_reaction(w, x, y, x + 1, y);
		world_sim_gravity(w, x, y);
	}

	x = w->w - 1;
	for (y = w->h - 2; y >= 0; y--) {
		if (MAT_NONE == w->dot[x][y]) {
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
                   int          *x,
                   const int     y)
{
	for (*x = 1; *x <= w->w - 2; *x += 1) {
		if (MAT_NONE == w->dot[*x][y]) {
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
}

static void
world_sim_to_left(struct World *w,
                  int          *x,
                  const int     y)
{
	for (*x = w->w - 2; *x >= 1; *x -= 1) {
		if (MAT_NONE == w->dot[*x][y]) {
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
}

static void
world_sim_chemical_reaction(struct World *w,
                            const int     x,
                            const int     y,
                            const int     dx,
                            const int     dy)
{
	float th;

	w->dissol[x][y] += MAT_ACIDITY[w->dot[dx][dy]] *
	                   MAT_ACID_VULN[w->dot[x][y]];
	if (w->dissol[x][y] >= 1.0) {
		w->dissol[x][y] = 0.0;
		world_clear_dot(w, x, y);
	}

	if (MAT_OXID_SPEED[w->dot[x][y]] > 0.0) {
		if (MAT_OXYGEN == w->dot[dx][dy]) {
			if (w->thermo[x][y] > MAT_IGN_P[w->dot[x][y]]) {
				th = MAT_OXID_HEAT[w->dot[x][y]] *
				     MAT_OXID_SPEED[w->dot[x][y]] /
				     2.0;

				w->oxid[x][y] += MAT_OXID_SPEED[w->dot[x][y]];
				w->thermo[x][y] += th;
				w->thermo[dx][dy] += th;

				if (w->oxid[x][y] >= 1.0) {
					mat_oxid_prdcts(w->dot[x][y],
					                &w->dot[x][y],
					                &w->dot[dx][dy]);
					w->oxid[x][y] = 0.0;
				}
			}
		}
	}

	if (MAT_TOUCH_REAGENT[w->dot[x][y]] != MAT_NONE &&
	    MAT_TOUCH_REAGENT[w->dot[x][y]] == w->dot[dx][dy]) {
		mat_touch_prdcts(w->dot[x][y], &w->dot[x][y], &w->dot[dx][dy]);
	}
}

static void
world_sim_gravity(struct World *w,
                  const int     x,
                  const int     y)
{
	switch (w->state[x][y]) {
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
                        const int     x,
                        const int     y,
                        const int     x2,
                        const int     y2)
{
	float c1, c2, combCond;

	if (MAT_NONE == w->dot[x2][y2]) {
		return;
	}

	combCond = (MAT_TH_COND[w->dot[x][y]] + MAT_TH_COND[w->dot[x2][y2]]) / 2;
	c1 = (w->thermo[x2][y2] - w->thermo[x][y]) * combCond;
	c2 = (w->thermo[x][y] - w->thermo[x2][y2]) * combCond;

	w->thermo[x][y] += c1;
	w->thermo[x2][y2] += c2;
}

static void
world_swap_dots(struct World *w,
                const int     x,
                const int     y,
                const int     x2,
                const int     y2)
{
	float         tmp_d = w->dissol[x][y];
	enum Mat      tmp_m = w->dot[x][y];
	float         tmp_o = w->oxid[x][y];
	enum MatState tmp_s = w->state[x][y];
	float         tmp_t = w->thermo[x][y];

	w->dissol[x][y] = w->dissol[x2][y2];
	w->dot[x][y] = w->dot[x2][y2];
	w->oxid[x][y] = w->oxid[x2][y2];
	w->state[x][y] = w->state[x2][y2];
	w->thermo[x][y] = w->thermo[x2][y2];

	w->dissol[x2][y2] = tmp_d;
	w->dot[x2][y2] = tmp_m;
	w->oxid[x2][y2] = tmp_o;
	w->state[x2][y2] = tmp_s;
	w->thermo[x2][y2] = tmp_t;
}

void
world_update(struct World *w,
             const float   spawner_temperature)
{
	int x, y;

	for (x = 0; x < w->w; x++) {
		for (y = 0; y < w->h; y++) {
			if (w->spawner[x][y]) {
				w->dot[x][y] = w->spawner_mat[x][y];
				w->thermo[x][y] = spawner_temperature;
			}

			world_update_dot_from_thermo(w, x, y);
		}
	}
}

static void
world_update_dot_from_thermo(struct World *w,
                             const int     x,
                             const int     y)
{
	if (w->thermo[x][y] < MAT_MELT_P[w->dot[x][y]]) {
		w->state[x][y] = MAT_SOLID_S[w->dot[x][y]];
		w->weight[x][y] = MAT_FULL_WEIGHT[w->dot[x][y]];
	} else if (w->thermo[x][y] < MAT_BOIL_P[w->dot[x][y]]) {
		w->state[x][y] = MS_LIQUID;

		if (MAT_MELT_DECOMP[w->dot[x][y]]) {
			w->dot[x][y] = mat_melt_prdct(w->dot[x][y]);
		}

		w->weight[x][y] = MAT_FULL_WEIGHT[w->dot[x][y]] *
		                  WEIGHT_FACTOR_LIQUID;
	} else {
		w->state[x][y] = MS_GAS;
		w->weight[x][y] = MAT_FULL_WEIGHT[w->dot[x][y]] *
		                  WEIGHT_FACTOR_GAS;
		w->weight[x][y] -= w->weight[x][y] *
		                   (w->thermo[x][y] -
		                    MAT_BOIL_P[w->dot[x][y]]) /
		                   WEIGHTLOSS_LIMIT_GAS;
	}
}

void
world_use_brush(struct World   *w,
                const enum Mat  m,
                const float     t,
                const int       x_c,
                const int       y_c,
                const int       radius)
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
			w->dissol[x][y] = 0.0;
			w->dot[x][y] = m;
			w->oxid[x][y] = 0.0;
			w->thermo[x][y] = t;
		}
	}
}

void
world_use_eraser(struct World *w,
                 const int     x_c,
                 const int     y_c,
                 const int     radius)
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
                 const float   delta,
                 const int     x_c,
                 const int     y_c,
                 const int     radius)
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
                 const float   delta,
                 const int     x_c,
                 const int     y_c,
                 const int     radius)
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
world_free(struct World *w)
{
	if (w->dissol != NULL) {
		free(w->dissol);
		w->dissol = NULL;
	}

	if (w->_dissol != NULL) {
		free(w->_dissol);
		w->_dissol = NULL;
	}

	if (w->dot != NULL) {
		free(w->dot);
		w->dot = NULL;
	}

	if (w->_dot != NULL) {
		free(w->_dot);
		w->_dot = NULL;
	}

	if (w->oxid != NULL) {
		free(w->oxid);
		w->oxid = NULL;
	}

	if (w->_oxid != NULL) {
		free(w->_oxid);
		w->_oxid = NULL;
	}

	if (w->spawner != NULL) {
		free(w->spawner);
		w->spawner = NULL;
	}

	if (w->_spawner != NULL) {
		free(w->_spawner);
		w->_spawner = NULL;
	}

	if (w->spawner_mat != NULL) {
		free(w->spawner_mat);
		w->spawner_mat = NULL;
	}

	if (w->_spawner_mat != NULL) {
		free(w->_spawner_mat);
		w->_spawner_mat = NULL;
	}

	if (w->state != NULL) {
		free(w->state);
		w->state = NULL;
	}

	if (w->_state != NULL) {
		free(w->_state);
		w->_state = NULL;
	}

	if (w->thermo != NULL) {
		free(w->thermo);
		w->thermo = NULL;
	}

	if (w->_thermo != NULL) {
		free(w->_thermo);
		w->_thermo = NULL;
	}

	if (w->weight != NULL) {
		free(w->weight);
		w->weight = NULL;
	}

	if (w->_weight != NULL) {
		free(w->_weight);
		w->_weight = NULL;
	}
}
