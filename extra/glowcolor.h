/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#ifndef _GLOWCOLOR_H
#define _GLOWCOLOR_H

#include "core/core.h"

void
glowcolor_init(void);

struct Rgba
thermo_to_color(const float thermo);

#endif /* _GLOWCOLOR_H */
