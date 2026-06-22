/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <stdlib.h>
#include <time.h>

#include "hawps_core.h"

void
hawps_core_init(void)
{
	srand(clock());
}
