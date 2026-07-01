/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_MAT_H
#define _HAWPS_MAT_H

#include <stdbool.h>

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
	MAT_COAL, /* Bituminous, but also with some values from Graphite */
	MAT_IRON_OXIDE,
	MAT_ALUMINUM,
	MAT_ALUMINUM_OXIDE,
	MAT_IRON_THERMITE,
	MAT_MAGNESIUM,
	MAT_MAGNESIUM_OXIDE,
	MAT_SULFUR,
	MAT_SULFUR_TRIOXIDE,
	MAT_BLACK_POWDER,
	MAT_SULFURIC_ACID,
	MAT_KAOLINITE, /* Clay */
	MAT_METAKAOLIN, /* Ceramic */
	MAT_CALCIUM_CARBONATE, /* Limestone */
	MAT_CALCIUM_OXIDE, /* Cement */

	MAT_COUNT
};

enum MatState {
	MS_STATIC,
	MS_GRAIN,
	MS_LIQUID,
	MS_GAS,

	MS_COUNT
};

static const char *MAT_NAME[]             = {"None",    "Sand",    "Glass",   "Water",   "Iron",         "Oxygen",  "Hydrogen", "Carbon Dioxide", "Methane",          "Coal",             "Iron Oxide",      "Aluminum",         "Aluminum Oxide", "Thermite",   "Magnesium",         "Magnesium Oxide", "Sulfur",         "Sulfur Trioxide", "Black Powder",      "Sulfuric Acid", "Clay",         "Ceramic",    "Limestone",        "Cement"};
static const float MAT_ACIDITY[]          = {0,         0,         0,         0,         0,              0,         0,          0,                0,                  0,                  0,                 0,                  0,                0,            0,                   0,                 0,                0,                 0,                   0.3334,          0,              0,            0,                  0};        /* inflicts dissolution fraction per tick */
static const float MAT_ACID_VULN[]        = {0,         0,         0,         0,         0.5,            0,         0,          0,                0,                  0.2,                1.0,               0.5,                1.0,              1.0,          0.5,                 1.0,               0.005,            0.05,              0.075,               0,               0.334,          0,            1.0,                1.0};      /* factor at which acid damage is multiplied */
static const float MAT_FULL_WEIGHT[]      = {0.0,       1.5,       1.5,       0.999,     7.874,          0.001323,  0.00008319, 0.001977,         0.000657,           0.833,              5.25,              2.699,              3.987,            0.7,          17.37,               3.6,               1.96,             1.92,              1.7,                 1.8302,          1.6,            2.6,          2.7,                3.34};     /* g/cm³ */
static const float MAT_BOIL_P[]           = {0,         3223.15,   3223.15,   373.15,    3134.15,        90.19,     27.20,      194.686,          111.65,             3947.65,            9999.9,            2743.0,             3250.0,           3134.15,      1363.0,              3870.0,            717.8,            318.0,             3947.65,             610.0,           9001.69,        9001.69,      9001.69,            3120.0};   /* boils at K */
static const float MAT_IGN_P[]            = {0,         0,         0,         0,         0,              0,         858.0,      0,                853.15,             1001.15,            0,                 0,                  0,                1811.0,       746.0,               0,                 0,                0,                 737.15,              0,               0,              0,            0,                  0};        /* ignites at K */
static const float MAT_MELT_P[]           = {0,         1985.15,   1985.15,   273.15,    1811.15,        54.36,     13.99,      216.589,          90.55,              4200.15,            1812.0,            933.47,             2345.0,           1811.15,      923.0,               3125.0,            388.36,           290.0,             4200.15,             283.46,          823.15,         2053.15,      1098.0,             2886.0};   /* melts at K */
static const bool MAT_MELT_DECOMP[]       = {false,     true,      false,     false,     false,          false,     false,      false,            false,              false,              true,              false,              true,             false,        false,               true,              false,            false,             false,               false,           true,           true,         true,               false};    /* Decomposes instead of melting */
static const int MAT_MELT_PRDCT1_CHANCE[] = {0,         0,         0,         0,         0,              0,         0,          0,                0,                  0,                  0,                 0,                  0,                0,            0,                   0,                 0,                0,                 0,                   0,               100,            70,           95,                 0};        /* Decomposition product 1 chance at 0 - 100 percent */
static const enum Mat MAT_MELT_PRDCT1[]   = {MAT_NONE,  MAT_GLASS, MAT_NONE,  MAT_NONE,  MAT_NONE,       MAT_NONE,  MAT_NONE,   MAT_NONE,         MAT_NONE,           MAT_NONE,           MAT_IRON,          MAT_NONE,           MAT_ALUMINUM,     MAT_NONE,     MAT_NONE,            MAT_MAGNESIUM,     MAT_NONE,         MAT_NONE,          MAT_NONE,            MAT_NONE,        MAT_METAKAOLIN, MAT_GLASS,    MAT_CALCIUM_OXIDE,  MAT_NONE}; /* Decomposition product 1 */
static const enum Mat MAT_MELT_PRDCT2[]   = {MAT_NONE,  MAT_GLASS, MAT_NONE,  MAT_NONE,  MAT_NONE,       MAT_NONE,  MAT_NONE,   MAT_NONE,         MAT_NONE,           MAT_NONE,           MAT_IRON,          MAT_NONE,           MAT_ALUMINUM,     MAT_NONE,     MAT_NONE,            MAT_MAGNESIUM,     MAT_NONE,         MAT_NONE,          MAT_NONE,            MAT_NONE,        MAT_METAKAOLIN, MAT_ALUMINUM, MAT_CARBON_DIOXIDE, MAT_NONE}; /* Decomposition product 2 */
static const bool MAT_OXID_RANDOM[]       = {false,     false,     false,     false,     false,          false,     false,      false,            false,              true,               false,             false,              false,            true,         false,               false,             false,            false,             false,               false,           false,          false,        false,              false};    /* Has random oxidation products */
static const int MAT_OXID_PRDCT1_CHANCE[] = {0,         0,         0,         0,         0,              0,         0,          0,                0,                  5,                  0,                 0,                  0,                67,           0,                   0,                 0,                0,                 50,                  0,               0,              0,            0,                  0};        /* oxidation product 1 chance at 0 - 100 percent */
static const enum Mat MAT_OXID_PRDCT1[]   = {MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_IRON_OXIDE, MAT_NONE,  MAT_WATER,  MAT_NONE,         MAT_WATER,          MAT_WATER,          MAT_NONE,          MAT_ALUMINUM_OXIDE, MAT_NONE,         MAT_IRON,     MAT_MAGNESIUM_OXIDE, MAT_NONE,          MAT_NONE,         MAT_NONE,          MAT_SULFUR_TRIOXIDE, MAT_NONE,        MAT_NONE,       MAT_NONE,     MAT_NONE,           MAT_NONE}; /* Oxidation product 1 */
static const enum Mat MAT_OXID_PRDCT2[]   = {MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_OXYGEN,     MAT_NONE,  MAT_WATER,  MAT_NONE,         MAT_CARBON_DIOXIDE, MAT_CARBON_DIOXIDE, MAT_NONE,          MAT_OXYGEN,         MAT_NONE,         MAT_ALUMINUM, MAT_OXYGEN,          MAT_NONE,          MAT_NONE,         MAT_NONE,          MAT_CARBON_DIOXIDE,  MAT_NONE,        MAT_NONE,       MAT_NONE,     MAT_NONE,           MAT_NONE}; /* Oxidation product 2 */
static const float MAT_OXID_HEAT[]        = {0,         0,         0,         0,         0.69,           0,         2130.0,     0,                1963.0,             5400.0,             0,                 0.69,               0,                6270.0,       6740.0,              0,                 0,                0,                 2400.0,              0,               0,              0,            0,                  0};        /* K released on oxidation */
static const float MAT_OXID_SPEED[]       = {0,         0,         0,         0,         0.0001112,      0,         0.34,       0,                0.2,                0.005,              0,                 0.00666666,         0,                0.05,         0.1,                 0,                 0,                0,                 0.5,                 0,               0,              0,            0,                  0};        /* oxidation fraction per tick */
static const enum MatState MAT_SOLID_S[]  = {MS_STATIC, MS_GRAIN,  MS_STATIC, MS_STATIC, MS_STATIC,      MS_STATIC, MS_STATIC,  MS_STATIC,        MS_STATIC,          MS_STATIC,          MS_GRAIN,          MS_STATIC,          MS_STATIC,        MS_GRAIN,     MS_STATIC,           MS_GRAIN,          MS_GRAIN,         MS_GRAIN,          MS_GRAIN,            MS_STATIC,       MS_STATIC,      MS_STATIC,    MS_STATIC,          MS_GRAIN}; /* state when solid */
static const float MAT_TH_COND[]          = {0.0,       0.00673,   0.00673,   0.0061,    0.0804,         0.002,     0.0018,     0.00146,          0.003,              0.0033,             0.063,             0.237,              0.03,             0.063,        0.156,               0.0525,            0.000205,         0.011,             0.05,                0.0061,          0.00673,        0.00673,      0.00126,            0.001};    /* W/(m⋅K)/1000 but flattened so that at most two zeroes are after the dot */
static const enum Mat MAT_TOUCH_REAGENT[] = {MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,       MAT_NONE,  MAT_NONE,   MAT_NONE,         MAT_NONE,           MAT_NONE,           MAT_ALUMINUM,      MAT_IRON_OXIDE,     MAT_NONE,         MAT_NONE,     MAT_NONE,            MAT_NONE,          MAT_COAL,         MAT_NONE,          MAT_NONE,            MAT_NONE,        MAT_NONE,       MAT_NONE,     MAT_NONE,           MAT_NONE}; /* touching this material causes a reaction */
static const enum Mat MAT_TOUCH_PRDCT1[]  = {MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,       MAT_NONE,  MAT_NONE,   MAT_NONE,         MAT_NONE,           MAT_NONE,           MAT_IRON_THERMITE, MAT_IRON_THERMITE,  MAT_NONE,         MAT_NONE,     MAT_NONE,            MAT_NONE,          MAT_BLACK_POWDER, MAT_NONE,          MAT_NONE,            MAT_NONE,        MAT_NONE,       MAT_NONE,     MAT_NONE,           MAT_NONE}; /* Touch product 1 */
static const enum Mat MAT_TOUCH_PRDCT2[]  = {MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,  MAT_NONE,       MAT_NONE,  MAT_NONE,   MAT_NONE,         MAT_NONE,           MAT_NONE,           MAT_IRON_THERMITE, MAT_IRON_THERMITE,  MAT_NONE,         MAT_NONE,     MAT_NONE,            MAT_NONE,          MAT_BLACK_POWDER, MAT_NONE,          MAT_NONE,            MAT_NONE,        MAT_NONE,       MAT_NONE,     MAT_NONE,           MAT_NONE}; /* Touch product 2 */
static const short MAT_R[]                = {0,         238,       237,       150,       185,            200,       200,        200,              65,                 30,                 62,                200,                225,              112,         200,                  240,               181,              240,               60,                  255,             154,            212,          227,                240};      /* R */
static const short MAT_G[]                = {0,         217,       237,       150,       175,            200,       200,        200,              65,                 30,                 9,                 200,                225,              59,          200,                  240,               169,              240,               60,                  255,             139,            191,          223,                240};      /* G */
static const short MAT_B[]                = {0,         86,        237,       255,       175,            255,       255,        255,              65,                 30,                 0,                 210,                225,              65,          200,                  240,               49,               240,               60,                  255,             123,            169,          194,                240};      /* B */

enum Mat
mat_melt_prdct(const enum Mat mat);

void
mat_oxid_prdcts(const enum Mat     mat,
                enum Mat *restrict out1,
                enum Mat *restrict out2);

enum MatState
mat_thermo_to_state(const enum Mat mat,
                    const float    thermo);

#endif /* _HAWPS_MAT_H */
