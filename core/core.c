/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#include "core.h"

float
color_int8_to_float(const int color)
{
	return (float) color / 255.0;
}
