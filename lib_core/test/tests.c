/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <assert.h>
#include <hawps_core.h>
#include <math.h>
#include <stdio.h>

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
test_trunc_float(void)
{
	assert(trunc_float(0.123456789f) == 0.123450000f);
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

	assert(mat == world.dot[WORLD_W / 2 - 0][WORLD_H - 1]);
	assert(mat == world.dot[WORLD_W / 2 - 1][WORLD_H - 1]);
	assert(mat == world.dot[WORLD_W / 2 + 1][WORLD_H - 1]);
	assert(mat == world.dot[WORLD_W / 2 - 0][WORLD_H - 2]);
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

	assert(mat == world.dot[WORLD_W / 2 - 0][WORLD_H - 1]);
	assert(mat == world.dot[WORLD_W / 2 - 1][WORLD_H - 1] ||
	       mat == world.dot[WORLD_W / 2 + 1][WORLD_H - 1]);
	assert(mat == world.dot[WORLD_W / 2 - 2][WORLD_H - 1] ||
	       mat == world.dot[WORLD_W / 2 + 2][WORLD_H - 1]);
	assert(mat == world.dot[WORLD_W / 2 - 3][WORLD_H - 1] ||
	       mat == world.dot[WORLD_W / 2 + 3][WORLD_H - 1]);
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

	assert(trunc_float(cold) ==
	       trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 1]));
	assert(trunc_float(hot) ==
	       trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 2]));

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

	assert(trunc_float(temp) ==
	       trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 1]));
	assert(trunc_float(temp) ==
	       trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 2]));

	tick_world();

	assert(trunc_float(temp) ==
	       trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 1]));
	assert(trunc_float(temp) ==
	       trunc_float(world.thermo[WORLD_W / 2][WORLD_H - 2]));
}

void
test_melt_decomposition(void)
{
	clear_world();
	world_use_brush(&world, MAT_SAND, 9999.9, 0, 0, 0);
	world_use_brush(&world, MAT_CALCIUM_CARBONATE, 9999.9, 0, 1, 0);
	/* no tick needed */

	assert(MAT_MELT_PRDCT1[MAT_SAND] == world.dot[0][0]);
	assert(MAT_MELT_PRDCT1[MAT_CALCIUM_CARBONATE] == world.dot[0][1] ||
	       MAT_MELT_PRDCT2[MAT_CALCIUM_CARBONATE] == world.dot[0][1]);
}

void
test_oxidation(void)
{
	const enum Mat mat = MAT_IRON;
	const int x = 0;
	const int y = WORLD_H - 1;
	const int ox = x + 1;
	const int oy = y;
	int i;

	clear_world();
	world_use_brush(&world, mat, WORLD_TEMPERATURE, x, y, 0);
	world_use_brush(&world, MAT_OXYGEN, WORLD_TEMPERATURE, ox, oy, 0);

	assert(mat        == world.dot[x][y]);
	assert(MAT_OXYGEN == world.dot[ox][oy]);

	for (i = 0; i < (1.0 / MAT_OXID_SPEED[mat]); i++) {
		tick_world();
	}

	assert(MAT_OXID_PRDCT1[mat] == world.dot[x][y]);
	assert(MAT_OXID_PRDCT2[mat] == world.dot[ox][oy]);
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

	assert(mat                    == world.dot[0][WORLD_H - 1]);
	assert(MAT_TOUCH_REAGENT[mat] == world.dot[1][WORLD_H - 1]);

	tick_world();

	assert(MAT_TOUCH_PRDCT1[mat] == world.dot[0][WORLD_H - 1]);
	assert(MAT_TOUCH_PRDCT2[mat] == world.dot[1][WORLD_H - 1]);
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

	assert(trunc_float(MAT_FULL_WEIGHT[mat]) ==
	       trunc_float(world.weight[x][y]));

	world_use_heater(&world, MAT_BOIL_P[mat], x, y, 0);
	tick_world();

	assert(trunc_float(world.weight[x][y]) <
	       trunc_float(MAT_FULL_WEIGHT[mat]));
}

void
test_spawner(void)
{
	const enum Mat mat = MAT_OXYGEN;
	const int x = 0;
	const int y = 0;

	clear_world();

	assert(MAT_NONE == world.dot[x][y]);
	assert(MAT_NONE == world.dot[x][y + 1]);

	world.spawner[x][y] = true;
	world.spawner_mat[x][y] = mat;
	tick_world();

	assert(mat == world.dot[x][y] ||
	       mat == world.dot[x][y + 1]);
}

void
test_acidity(void)
{
	const int ax = 0;
	const int ay = WORLD_H - 1;
	const int vx = ax + 1;
	const int vy = ay;

	enum Mat acid = 0;
	bool     acid_found = false;
	int      i;
	enum Mat vuln = 0;
	bool     vuln_found = false;

	while (vuln < MAT_COUNT) {
		if (MAT_ACID_VULN[vuln] <= 0.0f) {
			vuln++;
		} else {
			vuln_found = true;
			break;
		}

	}
	assert(vuln_found);

	while (acid < MAT_COUNT) {
		if (MAT_ACIDITY[acid] <= 0.0f) {
			acid++;
		} else {
			acid_found = true;
			break;
		}
	}

	assert(acid_found);

	clear_world();

	world_use_brush(&world, vuln, WORLD_TEMPERATURE, vx, vy, 0);
	world_use_brush(&world, acid, WORLD_TEMPERATURE, ax, ay, 0);

	for (i = 0; i < (1.0 / (MAT_ACIDITY[acid] * MAT_ACID_VULN[vuln])); i++) {
		tick_world();
	}

	assert(MAT_NONE == world.dot[vx][vy]);
	assert(acid     == world.dot[ax][ay]);
}

void
test_mat_property_table_len(void)
{
	assert(MAT_COUNT == ARRSIZE(MAT_NAME));
	assert(MAT_COUNT == ARRSIZE(MAT_ACIDITY));
	assert(MAT_COUNT == ARRSIZE(MAT_ACID_VULN));
	assert(MAT_COUNT == ARRSIZE(MAT_FULL_WEIGHT));
	assert(MAT_COUNT == ARRSIZE(MAT_BOIL_P));
	assert(MAT_COUNT == ARRSIZE(MAT_IGN_P));
	assert(MAT_COUNT == ARRSIZE(MAT_MELT_P));
	assert(MAT_COUNT == ARRSIZE(MAT_MELT_DECOMP));
	assert(MAT_COUNT == ARRSIZE(MAT_MELT_PRDCT1_CHANCE));
	assert(MAT_COUNT == ARRSIZE(MAT_MELT_PRDCT1));
	assert(MAT_COUNT == ARRSIZE(MAT_MELT_PRDCT2));
	assert(MAT_COUNT == ARRSIZE(MAT_OXID_RANDOM));
	assert(MAT_COUNT == ARRSIZE(MAT_OXID_PRDCT1_CHANCE));
	assert(MAT_COUNT == ARRSIZE(MAT_OXID_PRDCT1));
	assert(MAT_COUNT == ARRSIZE(MAT_OXID_PRDCT2));
	assert(MAT_COUNT == ARRSIZE(MAT_OXID_HEAT));
	assert(MAT_COUNT == ARRSIZE(MAT_OXID_SPEED));
	assert(MAT_COUNT == ARRSIZE(MAT_SOLID_S));
	assert(MAT_COUNT == ARRSIZE(MAT_TH_COND));
	assert(MAT_COUNT == ARRSIZE(MAT_TOUCH_REAGENT));
	assert(MAT_COUNT == ARRSIZE(MAT_TOUCH_ALTPRDCT2_CHANCE));
	assert(MAT_COUNT == ARRSIZE(MAT_TOUCH_PRDCT1));
	assert(MAT_COUNT == ARRSIZE(MAT_TOUCH_PRDCT2));
	assert(MAT_COUNT == ARRSIZE(MAT_TOUCH_ALTPRDCT2));
	assert(MAT_COUNT == ARRSIZE(MAT_R));
	assert(MAT_COUNT == ARRSIZE(MAT_G));
	assert(MAT_COUNT == ARRSIZE(MAT_B));
}

int
main()
{
	hawps_core_init();
	world = world_new(WORLD_W, WORLD_H, WORLD_TEMPERATURE);

	test_trunc_float();

	test_gravity();
	test_stack_collapse_grain();
	test_stack_collapse_liquid_or_gas(MAT_WATER);
	test_stack_collapse_liquid_or_gas(MAT_OXYGEN);
	test_thermal_conduction();
	test_thermal_nonconduction();
	test_melt_decomposition();
	test_oxidation();
	test_touch();
	test_mass_loss_upon_heat_up();
	test_spawner();
	test_acidity();
	test_mat_property_table_len();

	world_free(&world); /* goodbye, cruel world */
	printf("All tests passed :)\n");
	return 0;
}
