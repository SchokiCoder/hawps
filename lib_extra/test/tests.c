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

int
main()
{
	hawps_core_init();
	hawps_extra_init();

	test_rgba_blend();

	printf("All tests passed :)\n");
	return 0;
}
