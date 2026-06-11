/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#include <stdlib.h>

#include "mat.h"

enum Mat
mat_melt_prdct(const enum Mat mat)
{
	if ((rand() % 100) < MAT_MELT_PRDCT1_CHANCE[mat]) {
		return MAT_MELT_PRDCT1[mat];
	} else {
		return MAT_MELT_PRDCT2[mat];
	}
}

enum MatState
mat_thermo_to_state(const enum Mat mat,
                    const float thermo)
{
	enum MatState ret;

	if (thermo < MAT_MELT_P[mat]) {
		ret = MAT_SOLID_S[mat];
	} else if (thermo < MAT_BOIL_P[mat]) {
		ret = MS_LIQUID;
	} else {
		ret = MS_GAS;
	}

	return ret;
}
