/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_GLOWCOLOR_H
#define _HAWPS_GLOWCOLOR_H

struct Rgba {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

void
glowcolor_init(void);

struct Rgba
thermo_to_color(const float thermo);

#endif /* _HAWPS_GLOWCOLOR_H */
