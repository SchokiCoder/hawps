/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_WORLD_H
#define _HAWPS_WORLD_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hawps_mat.h"

/* Macros
 */

/* Constant defines
 */

#define WEIGHT_FACTOR_LIQUID 0.95
#define WEIGHT_FACTOR_GAS    0.90
#define WEIGHTLOSS_LIMIT_GAS 5000.0

/* Types
 */
struct World {
	int w;
	int h;

	bool      *_spawner;
	bool     **spawner;
	enum Mat  *_spawner_mat;
	enum Mat **spawner_mat;

	float          *_dissol;
	float         **dissol;
	enum Mat       *_dot;
	enum Mat      **dot;
	float          *_oxid;
	float         **oxid;
	enum MatState  *_state;
	enum MatState **state;
	float          *_thermo;
	float         **thermo;
	float          *_weight;
	float         **weight;
};

/* This struct represents the content of a world file.
 */
struct WorldFileV1 {
	uint32_t  version;
	uint32_t  width;
	uint32_t  height;
	uint32_t  spawners;
	uint32_t  unused1;
	uint32_t  unused2;
	uint32_t  unused3;
	uint32_t  unused4;
	float    *dissol;
	uint16_t *dot;
	float    *oxid;
	uint8_t  *state;
	float    *thermo;
	/* weight is calculated, omitted */
	uint32_t *spawner_x;
	uint32_t *spawner_y;
	uint16_t *spawner_mat;
};

/* Function declarations
 */

struct World
world_new(const int w,
          const int h);

bool
world_can_displace(struct World *w,
                   const int     x,
                   const int     y,
                   const int     dx,
                   const int     dy);

void
world_clear_dot(struct World *w,
                const int     x,
                const int     y);

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

void
world_free(struct World *w);

struct World
world_load(FILE *f);

void
world_save(const struct World  w,
           FILE               *f);

/* You SHOULD call world_update before this.
 */
void
world_sim(struct World *w);

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

static void
world_sim_to_right(struct World *w,
                   int          *x,
                   const int     y);

static void
world_sim_to_left(struct World *w,
                  int          *x,
                  const int     y);

/* Swaps all properties of two coordinates.
 */
static void
world_swap_dots(struct World *w,
                const int     x,
                const int     y,
                const int     x2,
                const int     y2);

/* You may want to call world_sim after this.
 */
void
world_update(struct World *w,
             const float   spawner_temperature);

static void
world_update_dot_from_thermo(struct World *w,
                             const int     x,
                             const int     y);

void
world_use_brush(struct World   *w,
                const enum Mat  m,
                const float     t,
                const int       x_c,
                const int       y_c,
                const int       radius);

/* Using this to increase temperature, by giving a negative delta,
 * is inefficient. Cooling requires an additional check.
 * To heat, see world_use_heater
 */
void
world_use_cooler(struct World *w,
                 const float   delta,
                 const int     x_c,
                 const int     y_c,
                 const int     radius);

void
world_use_eraser(struct World *w,
                 const int     x_c,
                 const int     y_c,
                 const int     radius);

/* Using this to decrease temperature, by giving a negative delta,
 * may cause issues, as soon as the temperature of a dot goes negative.
 * To cool, see world_use_cooler
 */
void
world_use_heater(struct World *w,
                 const float   delta,
                 const int     x_c,
                 const int     y_c,
                 const int     radius);

struct WorldFileV1
WorldFileV1_new(void);

/* @f: File pointer to read from.
 *
 * Returns a struct with invalid version number upon failure.
 */
struct WorldFileV1
WorldFileV1_from_file(FILE *f);

struct WorldFileV1
WorldFileV1_from_world(const struct World w);

void
WorldFileV1_set_invalid(struct WorldFileV1 *wf);

bool
WorldFileV1_to_file(const struct WorldFileV1  wf,
                    FILE                     *f);

struct World
WorldFileV1_to_world(const struct WorldFileV1 wf);

void
WorldFileV1_free(struct WorldFileV1 *pw);

/* Function definitions
 */

#ifdef HAWPS_IMPL

struct World
world_new(const int w,
          const int h)
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

struct World
world_load(FILE *f)
{
	struct World       ret;
	struct WorldFileV1 wf;

	wf = WorldFileV1_from_file(f);
	if (0 == wf.version) {
		ret.w = 0;
		ret.h = 0;
		return ret;
	}

	ret = WorldFileV1_to_world(wf);

	WorldFileV1_free(&wf);

	return ret;
}

void
world_save(const struct World  w,
           FILE               *f)
{
	struct WorldFileV1 wf;

	wf = WorldFileV1_from_world(w);
	WorldFileV1_to_file(wf, f);

	WorldFileV1_free(&wf);
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

	world_sim_chemical_reaction(w, 0, w->h - 1, 1, w->h - 1);
	world_sim_chemical_reaction(w, w->w - 1, w->h - 1, w->w - 2, w->h - 1);

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

	world_sim_chemical_reaction(w, 0, 0, 1, 0);
	world_sim_chemical_reaction(w, w->w - 1, 0, w->w - 2, 0);

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

	if (MAT_OXYGEN == w->dot[dx][dy] &&
	    MAT_OXID_SPEED[w->dot[x][y]] > 0.0 &&
	    w->thermo[x][y] >= MAT_OXID_P[w->dot[x][y]]) {
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

			if (w->thermo[x][y] >= MAT_BOIL_P[w->dot[x][y]]) {
				if (MAT_MELT_DECOMP[w->dot[x][y]]) {
					w->dot[x][y] = mat_melt_prdct(w->dot[x][y]);
				}
			}
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

struct WorldFileV1
WorldFileV1_new(void)
{
	struct WorldFileV1 ret;

	ret.version = 1;
	ret.width = 0;
	ret.height = 0;
	ret.spawners = 0;
	ret.unused1 = 0;
	ret.unused2 = 0;
	ret.unused3 = 0;
	ret.unused4 = 0;
	ret.dissol = NULL;
	ret.dot = NULL;
	ret.oxid = NULL;
	ret.state = NULL;
	ret.thermo = NULL;
	ret.spawner_x = NULL;
	ret.spawner_y = NULL;
	ret.spawner_mat = NULL;

	return ret;
}

struct WorldFileV1
WorldFileV1_from_file(FILE *f)
{
	uint32_t temp;
	struct WorldFileV1 ret;

	ret = WorldFileV1_new();

	if (!f) {
		WorldFileV1_set_invalid(&ret);
		return ret;
	}

	assert(1 == fread(&temp, sizeof(temp), 1, f));

	if (temp != ret.version) {
		WorldFileV1_set_invalid(&ret);
		return ret;
	}

	assert(1 == fread(&ret.width, sizeof(ret.width), 1, f));
	assert(1 == fread(&ret.height, sizeof(ret.height), 1, f));
	assert(1 == fread(&ret.spawners, sizeof(ret.spawners), 1, f));
	assert(1 == fread(&ret.unused1, sizeof(ret.unused1), 1, f));
	assert(1 == fread(&ret.unused2, sizeof(ret.unused2), 1, f));
	assert(1 == fread(&ret.unused3, sizeof(ret.unused3), 1, f));
	assert(1 == fread(&ret.unused4, sizeof(ret.unused4), 1, f));

	ret.dissol = malloc(sizeof(float) * ret.width * ret.height);
	ret.dot    = malloc(sizeof(uint16_t) * ret.width * ret.height);
	ret.oxid   = malloc(sizeof(float) * ret.width * ret.height);
	ret.state  = malloc(sizeof(uint8_t) * ret.width * ret.height);
	ret.thermo = malloc(sizeof(float) * ret.width * ret.height);
	ret.spawner_x   = malloc(sizeof(uint32_t) * ret.spawners);
	ret.spawner_y   = malloc(sizeof(uint32_t) * ret.spawners);
	ret.spawner_mat = malloc(sizeof(uint16_t) * ret.spawners);

	assert(ret.width * ret.height ==
	       fread(ret.dissol, sizeof(ret.dissol[0]), ret.width * ret.height, f));
	assert(ret.width * ret.height ==
	       fread(ret.dot, sizeof(ret.dot[0]), ret.width * ret.height, f));
	assert(ret.width * ret.height ==
	       fread(ret.oxid, sizeof(ret.oxid[0]), ret.width * ret.height, f));
	assert(ret.width * ret.height ==
	       fread(ret.state, sizeof(ret.state[0]), ret.width * ret.height, f));
	assert(ret.width * ret.height ==
	       fread(ret.thermo, sizeof(ret.thermo[0]), ret.width * ret.height, f));

	assert(ret.spawners ==
	       fread(ret.spawner_x, sizeof(ret.spawner_x[0]), ret.spawners, f));
	assert(ret.spawners ==
	       fread(ret.spawner_y, sizeof(ret.spawner_y[0]), ret.spawners, f));
	assert(ret.spawners ==
	       fread(ret.spawner_mat, sizeof(ret.spawner_mat[0]), ret.spawners, f));

	return ret;
}

struct WorldFileV1
WorldFileV1_from_world(const struct World w)
{
	struct WorldFileV1 ret;
	size_t spawners_alloc = 8;
	uint32_t x, y;

	ret = WorldFileV1_new();
	ret.width = w.w;
	ret.height = w.h;
	ret.dissol = malloc(sizeof(float) * ret.width * ret.height);
	ret.dot    = malloc(sizeof(uint16_t) * ret.width * ret.height);
	ret.oxid   = malloc(sizeof(float) * ret.width * ret.height);
	ret.state  = malloc(sizeof(uint8_t) * ret.width * ret.height);
	ret.thermo = malloc(sizeof(float) * ret.width * ret.height);
	ret.spawner_x   = malloc(sizeof(uint32_t) * spawners_alloc);
	ret.spawner_y   = malloc(sizeof(uint32_t) * spawners_alloc);
	ret.spawner_mat = malloc(sizeof(uint16_t) * spawners_alloc);

	for (x = 0; x < (uint32_t) w.w; x++) {
		for (y = 0; y < (uint32_t) w.h; y++) {
			ret.dissol[x * w.h + y] = w.dissol[x][y];
			ret.dot[x * w.h + y] = w.dot[x][y];
			ret.oxid[x * w.h + y] = w.oxid[x][y];
			ret.state[x * w.h + y] = w.state[x][y];
			ret.thermo[x * w.h + y] = w.thermo[x][y];

			if (!w.spawner[x][y]) {
				continue;
			}

			ret.spawners++;
			if (ret.spawners > spawners_alloc) {
				spawners_alloc *= 2;
				ret.spawner_x = realloc(ret.spawner_x,
				                        sizeof(ret.spawner_x[0]) *
				                        spawners_alloc);
				ret.spawner_y = realloc(ret.spawner_y,
				                        sizeof(ret.spawner_y[0]) *
				                        spawners_alloc);
				ret.spawner_mat = realloc(ret.spawner_mat,
				                          sizeof(ret.spawner_mat[0]) *
				                          spawners_alloc);
			}
			ret.spawner_x[ret.spawners - 1] = x;
			ret.spawner_y[ret.spawners - 1] = y;
			ret.spawner_mat[ret.spawners - 1] = w.spawner_mat[x][y];
		}
	}

	return ret;
}

void
WorldFileV1_set_invalid(struct WorldFileV1 *wf)
{
	*wf = WorldFileV1_new();
	wf->version = 0;
	wf->width = -1;
	wf->height = -1;
	wf->spawners = -1;
}

bool
WorldFileV1_to_file(const struct WorldFileV1  wf,
                    FILE                     *f)
{
	if (!f) {
		return false;
	}

	assert(1 == fwrite(&wf.version, sizeof(wf.version), 1, f));
	assert(1 == fwrite(&wf.width, sizeof(wf.width), 1, f));
	assert(1 == fwrite(&wf.height, sizeof(wf.height), 1, f));
	assert(1 == fwrite(&wf.spawners, sizeof(wf.spawners), 1, f));
	assert(1 == fwrite(&wf.unused1, sizeof(wf.unused1), 1, f));
	assert(1 == fwrite(&wf.unused2, sizeof(wf.unused2), 1, f));
	assert(1 == fwrite(&wf.unused3, sizeof(wf.unused3), 1, f));
	assert(1 == fwrite(&wf.unused4, sizeof(wf.unused4), 1, f));

	assert(wf.width * wf.height ==
	       fwrite(wf.dissol, sizeof(wf.dissol[0]), wf.width * wf.height, f));
	assert(wf.width * wf.height ==
	       fwrite(wf.dot, sizeof(wf.dot[0]), wf.width * wf.height, f));
	assert(wf.width * wf.height ==
	       fwrite(wf.oxid, sizeof(wf.oxid[0]), wf.width * wf.height, f));
	assert(wf.width * wf.height ==
	       fwrite(wf.state, sizeof(wf.state[0]), wf.width * wf.height, f));
	assert(wf.width * wf.height ==
	       fwrite(wf.thermo, sizeof(wf.thermo[0]), wf.width * wf.height, f));

	assert(wf.spawners ==
	       fwrite(wf.spawner_x, sizeof(wf.spawner_x[0]), wf.spawners, f));
	assert(wf.spawners ==
	       fwrite(wf.spawner_y, sizeof(wf.spawner_y[0]), wf.spawners, f));
	assert(wf.spawners ==
	       fwrite(wf.spawner_mat, sizeof(wf.spawner_mat[0]), wf.spawners, f));

	return true;
}

struct World
WorldFileV1_to_world(const struct WorldFileV1 wf)
{
	struct World ret;
	uint32_t i, x, y;

	ret = world_new(wf.width, wf.height);

	for (x = 0; x < wf.width; x++) {
		for (y = 0; y < wf.height; y++) {
			ret.dissol[x][y] = wf.dissol[x * wf.height + y];
			ret.dot[x][y] = wf.dot[x * wf.height + y];
			ret.oxid[x][y] = wf.oxid[x * wf.height + y];
			ret.state[x][y] = wf.state[x * wf.height + y];
			ret.thermo[x][y] = wf.thermo[x * wf.height + y];
		}
	}

	for (i = 0; i < wf.spawners; i++) {
		ret.spawner[wf.spawner_x[i]][wf.spawner_y[i]] = true;
		ret.spawner_mat[wf.spawner_x[i]][wf.spawner_y[i]] = wf.spawner_mat[i];
	}

	return ret;
}

void
WorldFileV1_free(struct WorldFileV1 *wf)
{
	if (wf->dissol) {
		free(wf->dissol);
		wf->dissol = NULL;
	}
	if (wf->dot) {
		free(wf->dot);
		wf->dot = NULL;
	}
	if (wf->oxid) {
		free(wf->oxid);
		wf->oxid = NULL;
	}
	if (wf->state) {
		free(wf->state);
		wf->state = NULL;
	}
	if (wf->thermo) {
		free(wf->thermo);
		wf->thermo = NULL;
	}
	if (wf->spawner_x) {
		free(wf->spawner_x);
		wf->spawner_x = NULL;
	}
	if (wf->spawner_y) {
		free(wf->spawner_y);
		wf->spawner_y = NULL;
	}
	if (wf->spawner_mat) {
		free(wf->spawner_mat);
		wf->spawner_mat = NULL;
	}
	wf->version = 0;
	wf->width = 0;
	wf->height = 0;
	wf->spawners = 0;
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_WORLD_H */
