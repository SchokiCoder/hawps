/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_CORE_H
#define _HAWPS_CORE_H

#include "hawps_mat.h"
#include "hawps_world.h"

#ifndef ARRSIZE
#define ARRSIZE(a) (sizeof(a) / sizeof(*(a)))
#endif

void
hawps_core_init(void);

#endif /* _HAWPS_CORE_H */
