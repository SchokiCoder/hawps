/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#ifndef _MAT_H
#define _MAT_H

enum Mat {
	MAT_NONE,
	MAT_SAND,
	MAT_GLASS,
	MAT_WATER,
	MAT_IRON,
	MAT_OXYGEN,
	MAT_HYDROGEN,
	MAT_CARBON_DIOXIDE,
	MAT_METHANE,

	MAT_COUNT
};

enum MatState {
	MS_STATIC,
	MS_GRAIN,
	MS_LIQUID,
	MS_GAS,

	MS_COUNT
};

static const char *MAT_NAME[]            = {"None",    "Sand",   "Glass",   "Water",   "Iron",    "Oxygen",  "Hydrogen", "Carbon Dioxide", "Methane"};
static const float MAT_FULL_WEIGHT[]     = {0.0,       1.5,      1.5,       0.999,     7.874,     0.001323,  0.00008319, 0.001977,         0.000657};  /* g/cm³ */
static const float MAT_BOIL_P[]          = {0,         3223.15,  3223.15,   373.15,    3134.15,   90.19,     27.20,      194.686,          111.65};    /* K */
static const float MAT_IGN_P[]           = {0,         0,        0,         0,         0,         0,         858.0,      0,                853.15};    /* K */
static const float MAT_MELT_P[]          = {0,         1985.15,  1985.15,   273.15,    1811.15,   54.36,     13.99,      216.589,          90.55};     /* K */
static const enum Mat MAT_OXID_PRDCT1[]  = {MAT_NONE,  MAT_NONE, MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_WATER,  MAT_NONE,         MAT_WATER};
static const enum Mat MAT_OXID_PRDCT2[]  = {MAT_NONE,  MAT_NONE, MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_WATER,  MAT_NONE,         MAT_CARBON_DIOXIDE};
static const float MAT_OXID_TH[]         = {0,         0,        0,         0,         0,         0,         2130.0,     0,                1963.0};    /* K released on oxidation */
static const float MAT_OXID_SPD[]        = {0,         0,        0,         0,         0,         0,         0.34,       0,                0.2};       /* fraction per tick */
static const enum MatState MAT_SOLID_S[] = {MS_STATIC, MS_GRAIN, MS_STATIC, MS_STATIC, MS_STATIC, MS_STATIC, MS_STATIC,  MS_STATIC,        MS_STATIC}; /* state when solid */
static const float MAT_TH_COND[]         = {0.0,       0.00673,  0.00673,   0.0061,    0.084,     0.002,     0.0018,     0.00146,          0.003};     /* W/(m⋅K)/1000 but flattened so that at most two zeroes are after the dot */
static const float MAT_R[]               = {0,         238,      237,       150,       200,       200,       200,        200,              65};
static const float MAT_G[]               = {0,         217,      237,       150,       200,       200,       200,        200,              65};
static const float MAT_B[]               = {0,         86,       237,       255,       200,       255,       255,        255,              65};
static const float MAT_A[]               = {0,         255,      128,       205,       255,       100,       100,        100,              150};

#endif /* _MAT_H */
