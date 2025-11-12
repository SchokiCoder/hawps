// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

//go:generate stringer -type=Mat
package mat

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
	_weight     = [...]float64 {0.0,    1.5,     1.5,     0.999,  7.874,   0.001323, 0.00008319, 0.001977, 0.000657}  /* g/cm³ */
	_boilP      = [...]float64 {0,      3223.15, 3223.15, 373.15, 3134.15, 90.19,    27.20,      194.686,  111.65}    /* K */
	_ignP       = [...]float64 {0,      0,       0,       0,      0,       0,        858.0,      0,        853.15}    /* K */
	_meltP      = [...]float64 {0,      1985.15, 1985.15, 273.15, 1811.15, 54.36,    13.99,      216.589,  90.55}     /* K */
	_oxidPrdct1 = [...]Mat     {None,   None,    None,    None,   None,    None,     Water,      None,     Water}
	_oxidPrdct2 = [...]Mat     {None,   None,    None,    None,   None,    None,     Water,      None,     CarbonDioxide}
	_oxidTh     = [...]float64 {0,      0,       0,       0,      0,       0,        2130.0,     0,        1963.0}    /* K released on oxidation */
	_oxidSpd    = [...]float64 {0,      0,       0,       0,      0,       0,        0.34,       0,        0.2}       /* fraction per tick */
	_solidS     = [...]State   {Static, Grain,   Static,  Static, Static,  Static,   Static,     Static,   Static} /* state when solid */
	_thCond     = [...]float64 {0.0,    0.00673, 0.00673, 0.0061, 0.084,   0.002,    0.0018,     0.00146,  0.003}     /* W/(m⋅K)/1000 but flattened so that at most two zeroes are after the dot */
	_r          = [...]uint8   {0,      238,     237,     150,    200,     200,      200,        200,      65}
	_g          = [...]uint8   {0,      217,     237,     150,    200,     200,      200,        200,      65}
	_b          = [...]uint8   {0,      86,      237,     255,    200,     255,      255,        255,      65}
	_a          = [...]uint8   {0,      255,     128,     205,    255,     100,      100,        100,      150}
)

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

func OxidPrdcts(
	i Mat,
) (Mat, Mat) {
	return _oxidPrdct1[i], _oxidPrdct2[i]
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
