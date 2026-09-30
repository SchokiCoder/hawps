/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_CORE_H
#define _HAWPS_CORE_H

#include <stdlib.h>
#include <time.h>

#include "hawps_mat.h"
#include "hawps_world.h"

/* Macros
 */

#ifndef ARRSIZE
#define ARRSIZE(a) (sizeof(a) / sizeof(*(a)))
#endif

/* Constant defines
 */

/* Function declarations
 */

void
hawps_core_init(void);

/* Function definitions
 */

#ifdef HAWPS_IMPL

void
hawps_core_init(void)
{
	srand(clock());
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_CORE_H */
