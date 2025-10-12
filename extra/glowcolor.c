/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

/* Instead of doing expensive HSV stuff for temperature based glowing,
 * we prepare an array of glow colors,
 * which we then iterate through based on dot temperature.
 */

#include "glowcolor.h"

#define GLOW_MAX_ALPHA 200
// Kelvin / 100
#define GLOW_RANGE 78

struct GlowRangeDef {
	int i;
	int r;
	int g;
	int b;
	int a;
};

struct RgbaFloat {
	float r;
	float g;
	float b;
	float a;
};

static float
glow_range_def_to_glow_color_step(const float i1,
                                  const float c1,
                                  const float i2,
                                  const float c2);

struct Rgba glow_colors[GLOW_RANGE];

static const struct GlowRangeDef glow_ranges[] = {
	{.i = 0, .r = 0, .g = 0, .b = 0, .a = 0},
	{.i = 7,  .r = 255, .g = 0,   .b = 0,   .a = 0}, // draper point
	{.i = 40, .r = 255, .g = 0,   .b = 0,   .a = GLOW_MAX_ALPHA},
	{.i = 44, .r = 255, .g = 0,   .b = 0,   .a = GLOW_MAX_ALPHA},
	{.i = 46, .r = 255, .g = 127, .b = 0,   .a = GLOW_MAX_ALPHA},
	{.i = 48, .r = 255, .g = 190, .b = 0,   .a = GLOW_MAX_ALPHA},
	{.i = 50, .r = 255, .g = 255, .b = 0,   .a = GLOW_MAX_ALPHA},
	{.i = 52, .r = 0,   .g = 255, .b = 0,   .a = GLOW_MAX_ALPHA},
	{.i = 54, .r = 0,   .g = 255, .b = 0,   .a = GLOW_MAX_ALPHA},
	{.i = 57, .r = 0,   .g = 255, .b = 255, .a = GLOW_MAX_ALPHA},
	{.i = 58, .r = 0,   .g = 255, .b = 255, .a = GLOW_MAX_ALPHA},
	{.i = 61, .r = 0,   .g = 0,   .b = 255, .a = GLOW_MAX_ALPHA},
	{.i = 65, .r = 0,   .g = 0,   .b = 255, .a = GLOW_MAX_ALPHA},
	{.i = 72, .r = 127, .g = 0,   .b = 255, .a = GLOW_MAX_ALPHA},
	{.i = 75, .r = 127, .g = 0,   .b = 255, .a = GLOW_MAX_ALPHA},
	{.i = 78, .r = 127, .g = 0,   .b = 255, .a = 0}
};

void
glowcolor_init(void)
{
	int a;
	int b;
	float rStep;
	float gStep;
	float bStep;
	float aStep;
	struct RgbaFloat temp_glow_colors[GLOW_RANGE];

	glow_colors[0].r = 0;
	glow_colors[0].g = 0;
	glow_colors[0].b = 0;
	glow_colors[0].a = 0;

	temp_glow_colors[0].r = 0.0;
	temp_glow_colors[0].g = 0.0;
	temp_glow_colors[0].b = 0.0;
	temp_glow_colors[0].a = 0.0;

	a = 1;
	b = 0;
	for (; a < GLOW_RANGE; a++) {

		if (a > glow_ranges[b].i) {
			b++;
			rStep = glow_range_def_to_glow_color_step(
				glow_ranges[b - 1].i,
				glow_ranges[b - 1].r,
				glow_ranges[b].i,
				glow_ranges[b].r);
			gStep = glow_range_def_to_glow_color_step(
				glow_ranges[b - 1].i,
				glow_ranges[b - 1].g,
				glow_ranges[b].i,
				glow_ranges[b].g);
			bStep = glow_range_def_to_glow_color_step(
				glow_ranges[b - 1].i,
				glow_ranges[b - 1].b,
				glow_ranges[b].i,
				glow_ranges[b].b);
			aStep = glow_range_def_to_glow_color_step(
				glow_ranges[b - 1].i,
				glow_ranges[b - 1].a,
				glow_ranges[b].i,
				glow_ranges[b].a);
		}

		temp_glow_colors[a].r = temp_glow_colors[a - 1].r + rStep;
		temp_glow_colors[a].g = temp_glow_colors[a - 1].g + gStep;
		temp_glow_colors[a].b = temp_glow_colors[a - 1].b + bStep;
		temp_glow_colors[a].a = temp_glow_colors[a - 1].a + aStep;

		glow_colors[a].r = temp_glow_colors[a].r;
		glow_colors[a].g = temp_glow_colors[a].g;
		glow_colors[a].b = temp_glow_colors[a].b;
		glow_colors[a].a = temp_glow_colors[a].a;
	}
}

static float
glow_range_def_to_glow_color_step(const float i1,
                                  const float c1,
                                  const float i2,
                                  const float c2)
{
	return (c2 - c1) / (i2 - i1);
}

struct Rgba
thermo_to_color(const float thermo)
{
	struct Rgba ret = {.r = 0, .g = 0, .b = 0, .a = 0};
	int glow_index;

	glow_index = (int)(thermo / 100.0);
	if (glow_index < GLOW_RANGE) {
		ret = glow_colors[glow_index];
	}

	return ret;
}
