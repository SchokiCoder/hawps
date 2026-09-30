/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_EXTRA_H
#define _HAWPS_EXTRA_H

#include "hawps_color.h"
#include "hawps_tool.h"

/* Macros
 */

/* Constant defines
 */

/* Types
 */

/* Function declarations
 */

void
hawps_extra_init(void);

/* Function definitions
 */

#ifdef HAWPS_IMPL

void
hawps_extra_init(void)
{
	glowcolor_init();
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_EXTRA_H */
