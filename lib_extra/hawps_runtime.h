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

#define NANOS_PER_SEC ((NSecs) 1000000000)

/* Types
 */

typedef uint64_t NSecs;

/* Function declarations
 */

NSecs
get_now();

/* Function definitions
 */

#ifdef HAWPS_IMPL

NSecs
get_now()
{
	NSecs           ret;
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	ret = ((NSecs) ts.tv_sec * NANOS_PER_SEC) + (NSecs) ts.tv_nsec;

	return ret;
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_RUNTIME_H */
