/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <stdlib.h>

#include "hawps_mat.h"

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
