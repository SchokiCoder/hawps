/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <stdlib.h>
#include <string.h>

#include "hawps_mat.h"

bool
mat_from_string(const char *str,
                enum Mat   *mat)
{
	size_t i;

	for (i = 0; i < MAT_COUNT; i++) {
		if (strcmp(str, MAT_NAME[i]) == 0) {
			*mat = i;
			return true;
		}
	}

	return false;
}

enum Mat
mat_melt_prdct(const enum Mat mat)
{
	if ((rand() % 100) < MAT_MELT_PRDCT1_CHANCE[mat]) {
		return MAT_MELT_PRDCT1[mat];
	} else {
		return MAT_MELT_PRDCT2[mat];
	}
}

void
mat_oxid_prdcts(const enum Mat     mat,
                enum Mat *restrict out1,
                enum Mat *restrict out2)
{
	if (MAT_OXID_RANDOM[mat]) {
		if ((rand() % 100) <= MAT_OXID_PRDCT1_CHANCE[mat]) {
			*out1 = MAT_OXID_PRDCT1[mat];
		} else {
			*out1 = MAT_OXID_PRDCT2[mat];
		}
		if ((rand() % 100) <= MAT_OXID_PRDCT1_CHANCE[mat]) {
			*out2 = MAT_OXID_PRDCT1[mat];
		} else {
			*out2 = MAT_OXID_PRDCT2[mat];
		}
	} else {
		*out1 = MAT_OXID_PRDCT1[mat];
		*out2 = MAT_OXID_PRDCT2[mat];
	}
}

enum MatState
mat_thermo_to_state(const enum Mat mat,
                    const float    thermo)
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

void
mat_touch_prdcts(const enum Mat     mat,
                 enum Mat *restrict out1,
                 enum Mat *restrict out2)
{
	*out1 = MAT_TOUCH_PRDCT1[mat];

	if ((rand() % 100) <= MAT_TOUCH_ALTPRDCT2_CHANCE[mat]) {
		*out2 = MAT_TOUCH_ALTPRDCT2[mat];
	} else {
		*out2 = MAT_TOUCH_PRDCT2[mat];
	}
}
