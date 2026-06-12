/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _WORLD_H
#define _WORLD_H

#include <stdbool.h>

#include "mat.h"

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

struct World
world_new(const int   w,
          const int   h,
          const float temperature);

bool
world_can_displace(struct World *w,
                   const int     x,
                   const int     y,
                   const int     dx,
                   const int     dy);

/* You may want to call world_sim after this.
 */
void
world_update(struct World *w,
             const float   spawner_temperature);

void
world_use_brush(struct World   *w,
                const enum Mat  m,
                const float     t,
                const int       x_c,
                const int       y_c,
                const int       radius);

void
world_use_eraser(struct World *w,
                 const int     x_c,
                 const int     y_c,
                 const int     radius);

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

/* You SHOULD call world_update before this.
 */
void
world_sim(struct World *w);

void
world_free(struct World *w);

#endif /* _WORLD_H */
