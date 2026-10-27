/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_RUNTIME_H
#define _HAWPS_RUNTIME_H

#include <stdint.h>
#include <time.h>

/* Macros
 */

/* Constant defines
 */

#define NANOS_PER_SEC ((uint64_t) 1000000000)

/* Types
 */

/* Function declarations
 */

uint64_t
get_now();

/* Function definitions
 */

#ifdef HAWPS_IMPL

uint64_t
get_now()
{
	uint64_t        ret;
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	ret = ((uint64_t) ts.tv_sec * NANOS_PER_SEC) + (uint64_t) ts.tv_nsec;

	return ret;
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_RUNTIME_H */
