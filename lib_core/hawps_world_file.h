/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_WORLD_FILE_H
#define _HAWPS_WORLD_FILE_H

#include <stdint.h>
#include <stdio.h>

#include "hawps_world.h"

/* This struct represents the content of a world file.
 */
struct WorldFileV1 {
	uint32_t  version;
	uint32_t  width;
	uint32_t  height;
	uint32_t  spawners;
	uint32_t  unused1;
	uint32_t  unused2;
	uint32_t  unused3;
	uint32_t  unused4;
	float    *dissol;
	uint16_t *dot;
	float    *oxid;
	uint8_t  *state;
	float    *thermo;
	/* weight is calculated, omitted */
	uint32_t *spawner_x;
	uint32_t *spawner_y;
	uint16_t *spawner_mat;
};

struct WorldFileV1
WorldFileV1_new(void);

/* @f: File pointer to read from.
 *
 * Returns a struct with invalid version number upon failure.
 */
struct WorldFileV1
WorldFileV1_from_file(FILE *f);

struct WorldFileV1
WorldFileV1_from_world(const struct World w);

bool
WorldFileV1_to_file(const struct WorldFileV1  wf,
                    FILE                     *f);

struct World
WorldFileV1_to_world(const struct WorldFileV1 wf);

void
WorldFileV1_free(struct WorldFileV1 *pw);

#endif /* _HAWPS_WORLD_FILE_H */
