/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#ifndef _TOOL_H
#define _TOOL_H

enum Tool {
	TOOL_BRUSH,
	TOOL_SPAWNER,
	TOOL_ERASER,
	TOOL_HEATER,
	TOOL_COOLER,

	TOOL_COUNT
};

static const char *TOOL_NAME[] = {"Brush", "Spawner", "Eraser", "Heater", "Cooler"};

#endif /* _TOOL_H */
