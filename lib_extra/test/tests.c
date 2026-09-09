/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <assert.h>
#include <hawps_core.h>
#include <hawps_extra.h>
#include <stdio.h>

#define WORLD_W           10
#define WORLD_H           10
#define WORLD_TEMPERATURE 273.15 + 20

static struct World world;

void
test_rgba_blend(void)
{
	const struct Rgba src = {
		.r = 30,
		.g = 40,
		.b = 50,
		.a = 100,
	};
	const struct Rgba dest = {
		.r = 100,
		.g = 125,
		.b = 75,
		.a = 255,
	};
	const struct Rgba expected = {
		.r = 72,
		.g = 91,
		.b = 64,
		.a = 255,
	};
	struct Rgba res;

	res = rgba_blend(src, dest);

	assert(expected.r == res.r);
	assert(expected.g == res.g);
	assert(expected.b == res.b);
	assert(expected.a == res.a);
}

void
test_heat_glow(void)
{
	struct Rgba cold;
	struct Rgba draper;
	struct Rgba hot;
	struct Rgba super_hot;

	cold = thermo_to_color(0.0);
	draper = thermo_to_color(800.0);
	hot = thermo_to_color(3000.0);
	super_hot = thermo_to_color(9001.0);

	assert(0 == cold.a);
	assert(draper.a > 0);
	assert(draper.a < 64);
	assert(hot.a > 128);
	assert(hot.a < 256);
	assert(0 == super_hot.a);
}

int
main()
{
	hawps_core_init();
	hawps_extra_init();

	test_rgba_blend();
	test_heat_glow();

	printf("All tests passed :)\n");
	return 0;
}
