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
	_symbol           = [...]string  {"nil",  "Si",    "Si",    "H2O",  "Fe",      "O2",     "H2",       "CO2",    "CH4",         "C9O",         "FeO"}
	_weight           = [...]float64 {0.0,    1.5,     1.5,     0.999,  7.874,     0.001323, 0.00008319, 0.001977, 0.000657,      0.833,         5.25}   /* g/cm³ */
	_boilP            = [...]float64 {0,      3223.15, 3223.15, 373.15, 3134.15,   90.19,    27.20,      194.686,  111.65,        3947.65,       9999.9} /* boils at K */
	_ignP             = [...]float64 {0,      0,       0,       0,      0,         0,        858.0,      0,        853.15,        1001.15,       0}      /* ignites at K */
	_meltP            = [...]float64 {0,      1985.15, 1985.15, 273.15, 1811.15,   54.36,    13.99,      216.589,  90.55,         4200.15,       1812.0} /* melts at K */
	_meltPrdct        = [...]Mat     {None,   Glass,   None,    None,   None,      None,     None,       None,     None,          None,          Iron}   /* becomes mat upon melting */
	_oxidPrdct1Chance = [...]int     {0,      0,       0,       0,      49,        0,        50,         0,        50,            5,             0}      /* 0 - 100 percent */
	_oxidPrdct1       = [...]Mat     {None,   None,    None,    None,   IronOxide, None,     Water,      None,     Water,         Water,         None}
	_oxidPrdct2       = [...]Mat     {None,   None,    None,    None,   Oxygen,    None,     Water,      None,     CarbonDioxide, CarbonDioxide, None}
	_oxidTh           = [...]float64 {0,      0,       0,       0,      0.69,      0,        2130.0,     0,        1963.0,        5400.0,        0}      /* K released on oxidation */
	_oxidSpd          = [...]float64 {0,      0,       0,       0,      0.0001112, 0,        0.34,       0,        0.2,           0.005,         0}      /* fraction per tick */
	_solidS           = [...]State   {Static, Grain,   Static,  Static, Static,    Static,   Static,     Static,   Static,        Static,        Grain}  /* state when solid */
	_thCond           = [...]float64 {0.0,    0.00673, 0.00673, 0.0061, 0.084,     0.002,    0.0018,     0.00146,  0.003,         0.0033,        0.063}  /* W/(m⋅K)/1000 but flattened so that at most two zeroes are after the dot */
	_r                = [...]uint8   {0,      238,     237,     150,    200,       200,      200,        200,      65,            30,            62}
	_g                = [...]uint8   {0,      217,     237,     150,    200,       200,      200,        200,      65,            30,            9}
	_b                = [...]uint8   {0,      86,      237,     255,    200,       255,      255,        255,      65,            30,            0}
	_a                = [...]uint8   {0,      255,     128,     205,    255,       100,      100,        100,      150,           255,           255}
)

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

func MeltPrdct(
	i Mat,
) Mat {
	return _meltPrdct[i]
}

func OxidPrdcts(
	i Mat,
) (Mat, Mat) {
	var ret1, ret2 Mat

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
