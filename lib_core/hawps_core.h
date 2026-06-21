/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_CORE_H
#define _HAWPS_CORE_H

#include "hawps_mat.h"
#include "hawps_world.h"

#define ARRSIZE(a) (sizeof(a) / sizeof(*(a)))

struct Rgba {
	int r;
	int g;
	int b;
	int a;
};

void
hawps_core_init(void);

float
color_int8_to_float(const int color);

#endif /* _HAWPS_CORE_H */
