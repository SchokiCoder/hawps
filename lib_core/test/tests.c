/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <assert.h>
#include <stdio.h>

#include "hawps_core.h"

#define WORLD_W           10
#define WORLD_H           10
#define WORLD_TEMPERATURE 20

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
test_gravity()
{
	clear_world();
	world_use_brush(&world, MAT_SAND, 0, 0, 0, 0);
	world_update(&world, WORLD_TEMPERATURE);
	world_sim(&world);
	assert(world.dot[0][0] != MAT_SAND);
}

int
main()
{
	hawps_core_init();
	world = world_new(WORLD_W, WORLD_H, WORLD_TEMPERATURE);

	test_gravity();

	world_free(&world);
	printf("All tests passed\n");
	return 0;
}
