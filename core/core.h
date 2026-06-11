/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#ifndef _CORE_H
#define _CORE_H

#include "mat.h"
#include "world.h"

#define ARRSIZE(a) (sizeof(a) / sizeof(*(a)))

struct Rgba {
	int r;
	int g;
	int b;
	int a;
};

float
color_int8_to_float(const int color);

#endif /* _CORE_H */
