/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <stdlib.h>
#include <time.h>

#include "core.h"

void
core_init(void)
{
	srand(clock());
}

float
color_int8_to_float(const int color)
{
	return (float) color / 255.0;
}
