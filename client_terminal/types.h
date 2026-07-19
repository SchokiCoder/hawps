/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _TYPES_H
#define _TYPES_H

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

struct ToolOptions {
	enum Mat  brush_mat;
	int       brush_radius;
	int       eraser_radius;
	enum Tool sel_tool;
	float     spawn_temperature;
	enum Mat  spawner_mat;
	float     thermo_delta;
	int       thermo_radius;
	int       x;
	int       y;
};

#endif /* _TYPES_H */
