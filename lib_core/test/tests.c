/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "hawps_core.h"

#define WORLD_W           10
#define WORLD_H           10
#define WORLD_TEMPERATURE 273.15 + 20

static struct World world;

void
clear_world(void)
{
	int x, y;

	for (x = 0; x < WORLD_W; x++) {
		for (y = 0; y < WORLD_H; y++) {
			world_clear_dot(&world, x, y);
		}
	}
}

void
tick_world(void)
{
	world_update(&world, WORLD_TEMPERATURE);
	world_sim(&world);
}

float
trunc_float(const float f)
{
	return floorf(f * 100000.0f) / 100000.0f;
}



void
test_gravity(void)
{
	clear_world();
	world_use_brush(&world, MAT_SAND, WORLD_TEMPERATURE, 0, 0, 0);
	tick_world();
	assert(world.dot[0][0] != MAT_SAND);
}

void
test_stack_collapse_grain(void)
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

void
test_thermal_conduction(void)
{
	const float cold = 0.0;
	const float hot = 9001.69;

	clear_world();

	world_use_brush(&world, MAT_IRON, cold,
		        WORLD_W / 2, WORLD_H - 1, 0);
	world_use_brush(&world, MAT_IRON, hot,
		        WORLD_W / 2, WORLD_H - 2, 0);

	assert(trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 1]) ==
	       trunc_float(cold));
	assert(trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 2]) ==
	       trunc_float(hot));

	tick_world();

	assert(trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 1]) >
	       trunc_float(cold));
	assert(trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 2]) <
	       trunc_float(hot));
}

void
test_thermal_nonconduction(void)
{
	const float temp = 420.0;

	clear_world();

	world_use_brush(&world, MAT_IRON, temp,
		        WORLD_W / 2, WORLD_H - 1, 0);
	world_use_brush(&world, MAT_IRON, temp,
		        WORLD_W / 2, WORLD_H - 2, 0);

	assert(trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 1]) ==
	       trunc_float(temp));
	assert(trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 2]) ==
	       trunc_float(temp));

	tick_world();

	assert(trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 1]) ==
	       trunc_float(temp));
	assert(trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 2]) ==
	       trunc_float(temp));
}

void
test_melt_decomposition(void)
{
	clear_world();
	world_use_brush(&world, MAT_SAND, 9999.9, 0, 0, 0);
	world_use_brush(&world, MAT_CALCIUM_CARBONATE, 9999.9, 0, 1, 0);
	/* no tick needed */

	assert(world.dot[0][0] == MAT_MELT_PRDCT1[MAT_SAND]);
	assert(world.dot[0][1] == MAT_MELT_PRDCT1[MAT_CALCIUM_CARBONATE] ||
	       world.dot[0][1] == MAT_MELT_PRDCT2[MAT_CALCIUM_CARBONATE]);
}

void
test_oxidation(void)
{
	const enum Mat mat = MAT_IRON;
	int i;

	clear_world();
	world_use_brush(&world, mat, WORLD_TEMPERATURE,
	                0, WORLD_H - 1, 0);
	world_use_brush(&world, MAT_OXYGEN, WORLD_TEMPERATURE,
	                0, WORLD_H - 2, 0);

	assert(world.dot[0][WORLD_H - 1] == mat);
	assert(world.dot[0][WORLD_H - 2] == MAT_OXYGEN);

	for (i = 0; i < (1.0 / MAT_OXID_SPEED[mat]); i++) {
		tick_world();
	}

	assert(world.dot[0][WORLD_H - 1] == MAT_OXID_PRDCT1[mat]);
	assert(world.dot[0][WORLD_H - 2] == MAT_OXID_PRDCT2[mat]);
}

void
test_touch(void)
{
	const enum Mat mat = MAT_CALCIUM_OXIDE;

	clear_world();
	world_use_brush(&world, mat, WORLD_TEMPERATURE,
	                0, WORLD_H - 1, 0);
	world_use_brush(&world, MAT_TOUCH_REAGENT[mat], WORLD_TEMPERATURE,
	                1, WORLD_H - 1, 0);

	assert(world.dot[0][WORLD_H - 1] == mat);
	assert(world.dot[1][WORLD_H - 1] == MAT_TOUCH_REAGENT[mat]);

	tick_world();

	assert(world.dot[0][WORLD_H - 1] == MAT_TOUCH_PRDCT1[mat]);
	assert(world.dot[1][WORLD_H - 1] == MAT_TOUCH_PRDCT2[mat]);
}

void
test_mass_loss_upon_heat_up(void)
{
	const enum Mat mat = MAT_OXYGEN;
	const int x = 0;
	const int y = WORLD_H - 1;

	clear_world();
	world_use_brush(&world, mat, 0.0, x, y, 0);

	tick_world();

	assert(trunc_float(world.weight[x][y]) ==
	       trunc_float(MAT_FULL_WEIGHT[mat]));

	world_use_heater(&world, MAT_BOIL_P[mat], x, y, 0);
	tick_world();

	assert(trunc_float(world.weight[x][y]) <
	       trunc_float(MAT_FULL_WEIGHT[mat]));
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
	test_thermal_conduction();
	test_thermal_nonconduction();
	test_melt_decomposition();
	// TODO fix and enable: test_oxidation();
	// TODO same            test_touch();
	test_mass_loss_upon_heat_up();

	world_free(&world);
	printf("All tests passed :)\n");
	return 0;
}
