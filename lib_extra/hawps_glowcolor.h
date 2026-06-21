/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_GLOWCOLOR_H
#define _HAWPS_GLOWCOLOR_H

#include <hawps_core.h>

void
glowcolor_init(void);

struct Rgba
thermo_to_color(const float thermo);

#endif /* _HAWPS_GLOWCOLOR_H */
