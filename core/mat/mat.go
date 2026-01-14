// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

//go:generate stringer -type=Mat
package mat

import (
	"math/rand/v2"
)

type Mat int
const (
	None Mat = iota
	Sand
	Glass
	Water
	Iron
	Oxygen
	Hydrogen
	CarbonDioxide
	Methane
	Coal /* Bituminous, but also with some value from Graphite */
	IronOxide
	Aluminum
	AluminumOxide
	IronThermite
	Magnesium
	MagnesiumOxide
	Sulfur
	SulfurTrioxide
	BlackPowder
	SulfuricAcid
	Kaolinite /* Clay */
	Metakaolin /* Ceramic */
	CalciumCarbonate /* Limestone */
	CalciumOxide /* Cement */

	MatCount int = iota
)

type State int
const (
	Static State = iota
	Grain
	Liquid
	Gas

	StateCount int = iota
)

const (
	WeightFactorLiquid = 0.95
	WeightFactorGas    = 0.90
	WeightLossLimitGas = 5000.0
)

var (
	_symbol           = [...]string  {"nil",  "Si",    "Si",    "H2O",  "Fe",      "O2",     "H2",       "CO2",    "CH4",         "C9O",         "FeO",        "Al",          "AlO",    "FeAl",   "Mg",           "MgO",     "S",         "SO3", "SC",           "H2SO", "cly",      "crm",    "CaC",         "CaO"}  /* Simplified chemical formula */
	_acidity          = [...]float64 {0,      0,       0,       0,      0,         0,        0,          0,        0,             0,             0,            0,             0,        0,        0,              0,         0,           0,     0,              0.3334, 0,          0,        0,             0}      /* inflicts dissolution fraction per tick */
	_acidVuln         = [...]float64 {0,      0,       0,       0,      0.5,       0,        0,          0,        0,             0.2,           1.0,          0.5,           1.0,      1.0,      0.5,            1.0,       0.005,       0.05,  0.075,          0,      0.334,      0,        1.0,           1.0}    /* factor at which acid damage is multiplied */
	_weight           = [...]float64 {0.0,    1.5,     1.5,     0.999,  7.874,     0.001323, 0.00008319, 0.001977, 0.000657,      0.833,         5.25,         2.699,         3.987,    0.7,      17.37,          3.6,       1.96,        1.92,  1.7,            1.8302, 1.6,        2.6,      2.7,           3.34}   /* g/cm³ */
	_boilP            = [...]float64 {0,      3223.15, 3223.15, 373.15, 3134.15,   90.19,    27.20,      194.686,  111.65,        3947.65,       9999.9,       2743.0,        3250.0,   3134.15,  1363.0,         3870.0,    717.8,       318.0, 3947.65,        610.0,  9001.69,    9001.69,  9001.69,       3120.0} /* boils at K */
	_ignP             = [...]float64 {0,      0,       0,       0,      0,         0,        858.0,      0,        853.15,        1001.15,       0,            0,             0,        1811.0,   746.0,          0,         0,           0,     737.15,         0,      0,          0,        0,             0}      /* ignites at K */
	_meltP            = [...]float64 {0,      1985.15, 1985.15, 273.15, 1811.15,   54.36,    13.99,      216.589,  90.55,         4200.15,       1812.0,       933.47,        2345.0,   1811.15,  923.0,          3125.0,    388.36,      290.0, 4200.15,        283.46, 823.15,     2053.15,  1098.0,        2886.0} /* melts at K */
	_meltDecomp       = [...]bool    {false,  false,   false,   false,  false,     false,    false,      false,    false,         false,         false,        false,         false,    false,    false,          false,     false,       false, false,          false,  true,       true,     true,          false}  /* Decomposes instead of melting */
	_meltPrdct1Chance = [...]int     {0,      0,       0,       0,      0,         0,        0,          0,        0,             0,             0,            0,             0,        0,        0,              0,         0,           0,     0,              0,      100,        70,       95,            0}      /* Decomposition product 1 chance at 0 - 100 percent */
	_meltPrdct1       = [...]Mat     {None,   Glass,   None,    None,   None,      None,     None,       None,     None,          None,          Iron,         None,          Aluminum, None,     None,           Magnesium, None,        None,  None,           None,   Metakaolin, Glass,    CalciumOxide,  None}   /* Decomposition product 1 */
	_meltPrdct2       = [...]Mat     {None,   Glass,   None,    None,   None,      None,     None,       None,     None,          None,          Iron,         None,          Aluminum, None,     None,           Magnesium, None,        None,  None,           None,   Metakaolin, Aluminum, CarbonDioxide, None}   /* Decomposition product 2 */
	_oxidRandom       = [...]bool    {false,  false,   false,   false,  false,     false,    false,      false,    false,         true,          false,        false,         false,    true,     false,          false,     false,       false, false,          false,  false,      false,    false,         false}  /* Has random oxidation products */
	_oxidPrdct1Chance = [...]int     {0,      0,       0,       0,      0,         0,        0,          0,        0,             5,             0,            0,             0,        67,       0,              0,         0,           0,     50,             0,      0,          0,        0,             0}      /* oxidation product 1 chance at 0 - 100 percent */
	_oxidPrdct1       = [...]Mat     {None,   None,    None,    None,   IronOxide, None,     Water,      None,     Water,         Water,         None,         AluminumOxide, None,     Iron,     MagnesiumOxide, None,      None,        None,  SulfurTrioxide, None,   None,       None,     None,          None}   /* Oxidation product 1 */
	_oxidPrdct2       = [...]Mat     {None,   None,    None,    None,   Oxygen,    None,     Water,      None,     CarbonDioxide, CarbonDioxide, None,         Oxygen,        None,     Aluminum, Oxygen,         None,      None,        None,  CarbonDioxide,  None,   None,       None,     None,          None}   /* Oxidation product 2 */
	_oxidTh           = [...]float64 {0,      0,       0,       0,      0.69,      0,        2130.0,     0,        1963.0,        5400.0,        0,            0.69,          0,        6270.0,   6740.0,         0,         0,           0,     2400.0,         0,      0,          0,        0,             0}      /* K released on oxidation */
	_oxidSpd          = [...]float64 {0,      0,       0,       0,      0.0001112, 0,        0.34,       0,        0.2,           0.005,         0,            0.00666666,    0,        0.05,     0.1,            0,         0,           0,     0.5,            0,      0,          0,        0,             0}      /* oxidation fraction per tick */
	_solidS           = [...]State   {Static, Grain,   Static,  Static, Static,    Static,   Static,     Static,   Static,        Static,        Grain,        Static,        Static,   Grain,    Static,         Grain,     Grain,       Grain, Grain,          Static, Static,     Static,   Static,        Grain}  /* state when solid */
	_thCond           = [...]float64 {0.0,    0.00673, 0.00673, 0.0061, 0.0804,    0.002,    0.0018,     0.00146,  0.003,         0.0033,        0.063,        0.237,         0.03,     0.063,    0.156,          0.0525,    0.000205,    0.011, 0.05,           0.0061, 0.00673,    0.00673,  0.00126,       0.001}  /* W/(m⋅K)/1000 but flattened so that at most two zeroes are after the dot */
	_touchReagent     = [...]Mat     {None,   None,    None,    None,   None,      None,     None,       None,     None,          None,          Aluminum,     IronOxide,     None,     None,     None,           None,      Coal,        None,  None,           None,   None,       None,     None,          None}   /* touching this material causes a reaction */
	_touchPrdct1      = [...]Mat     {None,   None,    None,    None,   None,      None,     None,       None,     None,          None,          IronThermite, IronThermite,  None,     None,     None,           None,      BlackPowder, None,  None,           None,   None,       None,     None,          None}   /* Touch product 1 */
	_touchPrdct2      = [...]Mat     {None,   None,    None,    None,   None,      None,     None,       None,     None,          None,          IronThermite, IronThermite,  None,     None,     None,           None,      BlackPowder, None,  None,           None,   None,       None,     None,          None}   /* Touch product 2 */
	_r                = [...]uint8   {0,      238,     237,     150,    185,       200,      200,        200,      65,            30,            62,           200,           225,      112,      200,            240,       181,         240,   60,             255,    154,        212,      227,           240}    /* R */
	_g                = [...]uint8   {0,      217,     237,     150,    175,       200,      200,        200,      65,            30,            9,            200,           225,      59,       200,            240,       169,         240,   60,             255,    139,        191,      223,           240}    /* G */
	_b                = [...]uint8   {0,      86,      237,     255,    175,       255,      255,        255,      65,            30,            0,            210,           225,      65,       200,            240,       49,          240,   60,             255,    123,        169,      194,           240}    /* B */
	_a                = [...]uint8   {0,      255,     128,     205,    255,       100,      100,        100,      150,           255,           255,          255,           255,      255,      255,            255,       215,         255,   255,            30,     255,        255,      255,           255}    /* A */
)

func Acidity(
	i Mat,
) float64 {
	return _acidity[i]
}

func AcidVulnerability(
	i Mat,
) float64 {
	return _acidVuln[i]
}

func Symbol(
	i Mat,
) string {
	return _symbol[i]
}

func SolidWeight(
	i Mat,
) float64 {
	return _weight[i]
}

func BoilP(
	i Mat,
) float64 {
	return _boilP[i]
}

func IgnP(
	i Mat,
) float64 {
	return _ignP[i]
}

func MeltP(
	i Mat,
) float64 {
	return _meltP[i]
}

func MeltDecomposition(
	i Mat,
) bool {
	return _meltDecomp[i]
}

func MeltPrdct(
	i Mat,
) Mat {
	if (rand.Int() % 100) <= _meltPrdct1Chance[i] {
		return _meltPrdct1[i]
	} else {
		return _meltPrdct2[i]
	}
}

func OxidPrdcts(
	i Mat,
) (Mat, Mat) {
	var ret1, ret2 Mat

	if _oxidRandom[i] {
		if (rand.Int() % 100) <= _oxidPrdct1Chance[i] {
			ret1 = _oxidPrdct1[i]
		} else {
			ret1 = _oxidPrdct2[i]
		}
		if (rand.Int() % 100) <= _oxidPrdct1Chance[i] {
			ret2 = _oxidPrdct1[i]
		} else {
			ret2 = _oxidPrdct2[i]
		}
	} else {
		ret1 = _oxidPrdct1[i]
		ret2 = _oxidPrdct2[i]
	}

	return ret1, ret2
}

func OxidSpd(
	i Mat,
) float64 {
	return _oxidSpd[i]
}

func OxidTh(
	i Mat,
) float64 {
	return _oxidTh[i]
}

func ThCond(
	i Mat,
) float64 {
	return _thCond[i]
}

func TouchReagent(
	i Mat,
) Mat {
	return _touchReagent[i]
}

func TouchPrdcts(
	i Mat,
) (Mat, Mat) {
	return _touchPrdct1[i], _touchPrdct2[i]
}

func SolidS(
	i Mat,
) State {
	return _solidS[i]
}

func R(
	i Mat,
) uint8 {
	return _r[i]
}

func G(
	i Mat,
) uint8 {
	return _g[i]
}

func B(
	i Mat,
) uint8 {
	return _b[i]
}

func A(
	i Mat,
) uint8 {
	return _a[i]
}

func ThermoToState(
	mat Mat,
	thermo float64,
) State {
	var ret State

	if thermo < MeltP(mat) {
		ret = SolidS(mat)
	} else if thermo < BoilP(mat) {
		ret = Liquid
	} else {
		ret = Gas
	}

	return ret
}
