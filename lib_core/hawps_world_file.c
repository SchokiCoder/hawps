/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <assert.h>
#include <stdlib.h>

#include "hawps_world_file.h"

/* Function declarations
 */

void
WorldFileV1_set_invalid(struct WorldFileV1 *wf);

/* Function definitions
 */

struct WorldFileV1
WorldFileV1_new(void)
{
	struct WorldFileV1 ret;

	ret.version = 1;
	ret.width = 0;
	ret.height = 0;
	ret.spawners = 0;
	ret.unused1 = 0;
	ret.unused2 = 0;
	ret.unused3 = 0;
	ret.unused4 = 0;
	ret.dissol = NULL;
	ret.dot = NULL;
	ret.oxid = NULL;
	ret.state = NULL;
	ret.thermo = NULL;
	ret.spawner_x = NULL;
	ret.spawner_y = NULL;
	ret.spawner_mat = NULL;

	return ret;
}

struct WorldFileV1
WorldFileV1_from_file(FILE *f)
{
	uint32_t temp;
	struct WorldFileV1 ret;

	ret = WorldFileV1_new();

	if (!f) {
		WorldFileV1_set_invalid(&ret);
		return ret;
	}

	assert(1 == fread(&temp, sizeof(temp), 1, f));

	if (temp != ret.version) {
		WorldFileV1_set_invalid(&ret);
		return ret;
	}

	assert(1 == fread(&ret.width, sizeof(ret.width), 1, f));
	assert(1 == fread(&ret.height, sizeof(ret.height), 1, f));
	assert(1 == fread(&ret.spawners, sizeof(ret.spawners), 1, f));
	assert(1 == fread(&ret.unused1, sizeof(ret.unused1), 1, f));
	assert(1 == fread(&ret.unused2, sizeof(ret.unused2), 1, f));
	assert(1 == fread(&ret.unused3, sizeof(ret.unused3), 1, f));
	assert(1 == fread(&ret.unused4, sizeof(ret.unused4), 1, f));

	ret.dissol = malloc(sizeof(float) * ret.width * ret.height);
	ret.dot    = malloc(sizeof(uint16_t) * ret.width * ret.height);
	ret.oxid   = malloc(sizeof(float) * ret.width * ret.height);
	ret.state  = malloc(sizeof(uint8_t) * ret.width * ret.height);
	ret.thermo = malloc(sizeof(float) * ret.width * ret.height);
	ret.spawner_x   = malloc(sizeof(uint32_t) * ret.spawners);
	ret.spawner_y   = malloc(sizeof(uint32_t) * ret.spawners);
	ret.spawner_mat = malloc(sizeof(uint16_t) * ret.spawners);

	assert(ret.width * ret.height ==
	       fread(ret.dissol, sizeof(ret.dissol[0]), ret.width * ret.height, f));
	assert(ret.width * ret.height ==
	       fread(ret.dot, sizeof(ret.dot[0]), ret.width * ret.height, f));
	assert(ret.width * ret.height ==
	       fread(ret.oxid, sizeof(ret.oxid[0]), ret.width * ret.height, f));
	assert(ret.width * ret.height ==
	       fread(ret.state, sizeof(ret.state[0]), ret.width * ret.height, f));
	assert(ret.width * ret.height ==
	       fread(ret.thermo, sizeof(ret.thermo[0]), ret.width * ret.height, f));

	assert(ret.spawners ==
	       fread(ret.spawner_x, sizeof(ret.spawner_x[0]), ret.spawners, f));
	assert(ret.spawners ==
	       fread(ret.spawner_y, sizeof(ret.spawner_y[0]), ret.spawners, f));
	assert(ret.spawners ==
	       fread(ret.spawner_mat, sizeof(ret.spawner_mat[0]), ret.spawners, f));

	return ret;
}

struct WorldFileV1
WorldFileV1_from_world(const struct World w)
{
	struct WorldFileV1 ret;
	size_t spawners_alloc = 8;
	uint32_t x, y;

	ret = WorldFileV1_new();
	ret.width = w.w;
	ret.height = w.h;
	ret.dissol = malloc(sizeof(float) * ret.width * ret.height);
	ret.dot    = malloc(sizeof(uint16_t) * ret.width * ret.height);
	ret.oxid   = malloc(sizeof(float) * ret.width * ret.height);
	ret.state  = malloc(sizeof(uint8_t) * ret.width * ret.height);
	ret.thermo = malloc(sizeof(float) * ret.width * ret.height);
	ret.spawner_x   = malloc(sizeof(uint32_t) * spawners_alloc);
	ret.spawner_y   = malloc(sizeof(uint32_t) * spawners_alloc);
	ret.spawner_mat = malloc(sizeof(uint16_t) * spawners_alloc);

	for (x = 0; x < (uint32_t) w.w; x++) {
		for (y = 0; y < (uint32_t) w.h; y++) {
			ret.dissol[x * w.h + y] = w.dissol[x][y];
			ret.dot[x * w.h + y] = w.dot[x][y];
			ret.oxid[x * w.h + y] = w.oxid[x][y];
			ret.state[x * w.h + y] = w.state[x][y];
			ret.thermo[x * w.h + y] = w.thermo[x][y];

			if (!w.spawner[x][y]) {
				continue;
			}

			ret.spawners++;
			if (ret.spawners > spawners_alloc) {
				spawners_alloc *= 2;
				ret.spawner_x = realloc(ret.spawner_x,
				                        sizeof(ret.spawner_x[0]) *
				                        spawners_alloc);
				ret.spawner_y = realloc(ret.spawner_y,
				                        sizeof(ret.spawner_y[0]) *
				                        spawners_alloc);
				ret.spawner_mat = realloc(ret.spawner_mat,
				                          sizeof(ret.spawner_mat[0]) *
				                          spawners_alloc);
			}
			ret.spawner_x[ret.spawners - 1] = x;
			ret.spawner_y[ret.spawners - 1] = y;
			ret.spawner_mat[ret.spawners - 1] = w.spawner_mat[x][y];
		}
	}

	return ret;
}

void
WorldFileV1_set_invalid(struct WorldFileV1 *wf)
{
	*wf = WorldFileV1_new();
	wf->version = 0;
	wf->width = -1;
	wf->height = -1;
	wf->spawners = -1;
}

bool
WorldFileV1_to_file(const struct WorldFileV1  wf,
                    FILE                     *f)
{
	if (!f) {
		return false;
	}

	assert(1 == fwrite(&wf.version, sizeof(wf.version), 1, f));
	assert(1 == fwrite(&wf.width, sizeof(wf.width), 1, f));
	assert(1 == fwrite(&wf.height, sizeof(wf.height), 1, f));
	assert(1 == fwrite(&wf.spawners, sizeof(wf.spawners), 1, f));
	assert(1 == fwrite(&wf.unused1, sizeof(wf.unused1), 1, f));
	assert(1 == fwrite(&wf.unused2, sizeof(wf.unused2), 1, f));
	assert(1 == fwrite(&wf.unused3, sizeof(wf.unused3), 1, f));
	assert(1 == fwrite(&wf.unused4, sizeof(wf.unused4), 1, f));

	assert(wf.width * wf.height ==
	       fwrite(wf.dissol, sizeof(wf.dissol[0]), wf.width * wf.height, f));
	assert(wf.width * wf.height ==
	       fwrite(wf.dot, sizeof(wf.dot[0]), wf.width * wf.height, f));
	assert(wf.width * wf.height ==
	       fwrite(wf.oxid, sizeof(wf.oxid[0]), wf.width * wf.height, f));
	assert(wf.width * wf.height ==
	       fwrite(wf.state, sizeof(wf.state[0]), wf.width * wf.height, f));
	assert(wf.width * wf.height ==
	       fwrite(wf.thermo, sizeof(wf.thermo[0]), wf.width * wf.height, f));

	assert(wf.spawners ==
	       fwrite(wf.spawner_x, sizeof(wf.spawner_x[0]), wf.spawners, f));
	assert(wf.spawners ==
	       fwrite(wf.spawner_y, sizeof(wf.spawner_y[0]), wf.spawners, f));
	assert(wf.spawners ==
	       fwrite(wf.spawner_mat, sizeof(wf.spawner_mat[0]), wf.spawners, f));

	return true;
}

struct World
WorldFileV1_to_world(const struct WorldFileV1 wf)
{
	struct World ret;
	uint32_t i, x, y;

	ret = world_new(wf.width, wf.height);

	for (x = 0; x < wf.width; x++) {
		for (y = 0; y < wf.height; y++) {
			ret.dissol[x][y] = wf.dissol[x * wf.height + y];
			ret.dot[x][y] = wf.dot[x * wf.height + y];
			ret.oxid[x][y] = wf.oxid[x * wf.height + y];
			ret.state[x][y] = wf.state[x * wf.height + y];
			ret.thermo[x][y] = wf.thermo[x * wf.height + y];
		}
	}

	for (i = 0; i < wf.spawners; i++) {
		ret.spawner[wf.spawner_x[i]][wf.spawner_y[i]] = true;
		ret.spawner_mat[wf.spawner_x[i]][wf.spawner_y[i]] = wf.spawner_mat[i];
	}

	return ret;
}

void
WorldFileV1_free(struct WorldFileV1 *wf)
{
	if (wf->dissol) {
		free(wf->dissol);
		wf->dissol = NULL;
	}
	if (wf->dot) {
		free(wf->dot);
		wf->dot = NULL;
	}
	if (wf->oxid) {
		free(wf->oxid);
		wf->oxid = NULL;
	}
	if (wf->state) {
		free(wf->state);
		wf->state = NULL;
	}
	if (wf->thermo) {
		free(wf->thermo);
		wf->thermo = NULL;
	}
	if (wf->spawner_x) {
		free(wf->spawner_x);
		wf->spawner_x = NULL;
	}
	if (wf->spawner_y) {
		free(wf->spawner_y);
		wf->spawner_y = NULL;
	}
	if (wf->spawner_mat) {
		free(wf->spawner_mat);
		wf->spawner_mat = NULL;
	}
	wf->version = 0;
	wf->width = 0;
	wf->height = 0;
	wf->spawners = 0;
}
