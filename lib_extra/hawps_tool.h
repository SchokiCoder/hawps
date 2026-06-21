/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_TOOL_H
#define _HAWPS_TOOL_H

enum Tool {
	TOOL_BRUSH,
	TOOL_SPAWNER,
	TOOL_ERASER,
	TOOL_HEATER,
	TOOL_COOLER,

	TOOL_COUNT
};

static const char *TOOL_NAME[] = {"Brush", "Spawner", "Eraser", "Heater", "Cooler"};

#endif /* _HAWPS_TOOL_H */
