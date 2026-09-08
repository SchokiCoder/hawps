/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <assert.h>
#include <stdio.h>

#include "hawps_core.h"

#define WORLD_W           10
#define WORLD_H           10
#define WORLD_TEMPERATURE 273.15 + 20

static struct World world;

void
clear_world()
{
	int x, y;

	for (x = 0; x < WORLD_W; x++) {
		for (y = 0; y < WORLD_H; y++) {
			world_clear_dot(&world, x, y);
		}
	}
}

void
tick_world()
{
	world_update(&world, WORLD_TEMPERATURE);
	world_sim(&world);
}

void
test_gravity()
{
	clear_world();
	world_use_brush(&world, MAT_SAND, WORLD_TEMPERATURE, 0, 0, 0);
	tick_world();
	assert(world.dot[0][0] != MAT_SAND);
}

void
test_stack_collapse_grain()
{
	const enum Mat mat = MAT_SAND;
	int i;

	clear_world();
	for (i = 1; i < 5; i++) {
		world_use_brush(&world, mat, WORLD_TEMPERATURE,
			        WORLD_W / 2, WORLD_H - i, 0);
	}
	for (i = 0; i < 100; i++) {
		tick_world();
	}

	assert(world.dot[WORLD_W / 2 - 0][WORLD_H - 4] != mat);

	assert(world.dot[WORLD_W / 2 - 0][WORLD_H - 1] == mat);
	assert(world.dot[WORLD_W / 2 - 1][WORLD_H - 1] == mat);
	assert(world.dot[WORLD_W / 2 + 1][WORLD_H - 1] == mat);
	assert(world.dot[WORLD_W / 2 - 0][WORLD_H - 2] == mat);
}

void
test_stack_collapse_liquid_or_gas(const enum Mat mat)
{
	int i;

	clear_world();
	for (i = 1; i < 5; i++) {
		world_use_brush(&world, mat, WORLD_TEMPERATURE,
			        WORLD_W / 2, WORLD_H - i, 0);
	}
	for (i = 0; i < 4; i++) {
		tick_world();
	}

	assert(world.dot[WORLD_W / 2][WORLD_H - 2] != mat);
	assert(world.dot[WORLD_W / 2][WORLD_H - 3] != mat);
	assert(world.dot[WORLD_W / 2][WORLD_H - 4] != mat);

	assert(world.dot[WORLD_W / 2 - 0][WORLD_H - 1] == mat);
	assert(world.dot[WORLD_W / 2 - 1][WORLD_H - 1] == mat ||
	       world.dot[WORLD_W / 2 + 1][WORLD_H - 1] == mat);
	assert(world.dot[WORLD_W / 2 - 2][WORLD_H - 1] == mat ||
	       world.dot[WORLD_W / 2 + 2][WORLD_H - 1] == mat);
	assert(world.dot[WORLD_W / 2 - 3][WORLD_H - 1] == mat ||
	       world.dot[WORLD_W / 2 + 3][WORLD_H - 1] == mat);
}

int
main()
{
	hawps_core_init();
	world = world_new(WORLD_W, WORLD_H, WORLD_TEMPERATURE);

	test_gravity();
	test_stack_collapse_grain();
	test_stack_collapse_liquid_or_gas(MAT_WATER);
	test_stack_collapse_liquid_or_gas(MAT_OXYGEN);

	world_free(&world);
	printf("All tests passed :)\n");
	return 0;
}
