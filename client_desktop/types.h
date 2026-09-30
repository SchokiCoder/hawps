/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _TYPES_H
#define _TYPES_H

#include <hawps_extra.h>

#include "config.h"

/* Types
 */

struct Rect {
	int x;
	int y;
	int w;
	int h;
};

enum StatusbarElement {
	SBE_WORLD_NAME,
	SBE_COORDS,
	SBE_VIEW,
	SBE_SPEED,
	SBE_IP_ADDRESS,

	SBE_COUNT
};

enum InputMode {
	IM_NORMAL,
	IM_COMMAND,
};

enum NumberRequirement {
	NR_NONE,
	NR_NOT_NEGATIVE,
	NR_POSITIVE,
};

#endif /* _TYPES_H */
