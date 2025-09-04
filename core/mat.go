// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

//go:generate stringer -type=Mat
package core

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

	Count int = iota
)

type State int
const (
	MsStatic State = iota
	MsGrain
	MsLiquid
	MsGas

	MsCount int = iota
)

const (
	WeightFactorLiquid = 0.95
	WeightFactorGas    = 0.90
	WeightLossLimitGas = 5000.0
)

var (
	_weights    = [...]float64 {0.0,      1.5,     1.5,      0.999,    7.874,    0.001323, 0.00008319, 0.001977,   0.000657} /* g/cm³ */
	_boilPs     = [...]float64 {0,        3223.15, 3223.15,  373.15,   3134.15,  90.19,    27.20,      194.686,    111.65}   /* K */
	_ignPs      = [...]float64 {0,        0,       0,        0,        0,        0,        858.0,      0,          853.15}   /* K */
	_meltPs     = [...]float64 {0,        1985.15, 1985.15,  273.15,   1811.15,  54.36,    13.99,      216.589,    90.55}    /* K */
	_oxidPrdct1 = [...]Mat     {None,     None,    None,     None,     None,     None,     Water,      None,       Water}
	_oxidPrdct2 = [...]Mat     {None,     None,    None,     None,     None,     None,     Water,      None,       CarbonDioxide}
	_oxidTh     = [...]float64 {0,        0,       0,        0,        0,        0,        2130.0,     0,          1963.0}   /* K released on oxidation */
	_oxidSpd    = [...]float64 {0,        0,       0,        0,        0,        0,        0.34,       0,          0.2}      /* fraction per tick */
	_solidSs    = [...]State   {MsStatic, MsGrain, MsStatic, MsStatic, MsStatic, MsStatic, MsStatic,   MsStatic,   MsStatic} /* state when solid */
	_thCond     = [...]float64 {0.0,      0.00673, 0.00673,  0.0061,   0.084,    0.002,    0.0018,     0.00146,    0.003}    /* W/(m⋅K)/1000 but flattened so that at most two zeroes are after the dot */
	Rs          = [...]uint8   {0,        238,     237,      150,      200,      200,      200,        200,        65}
	Gs          = [...]uint8   {0,        217,     237,      150,      200,      200,      200,        200,        65}
	Bs          = [...]uint8   {0,        86,      237,      255,      200,      255,      255,        255,        65}
	As          = [...]uint8   {0,        255,     128,      205,      255,      100,      100,        100,        150}
)

func SolidWeights(
	i Mat,
) float64 {
	return _weights[i]
}

func BoilPs(
	i Mat,
) float64 {
	return _boilPs[i]
}

func IgnPs(
	i Mat,
) float64 {
	return _ignPs[i]
}

func MeltPs(
	i Mat,
) float64 {
	return _meltPs[i]
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

func SolidSs(
	i Mat,
) State {
	return _solidSs[i]
}

type World struct {
	W        int
	H        int

	_spawner []bool
	Spawner  [][]bool
	_spwnMat []Mat
	SpwnMat  [][]Mat

	_dots    []Mat
	Dots     [][]Mat
	_oxid    []float64
	Oxid     [][]float64
	_states  []State
	States   [][]State
	_thermo  []float64
	Thermo   [][]float64
	_weights []float64
	Weights  [][]float64
}

func NewWorld(
	w, h int,
	temperature float64,
) World {
	var ret = World{
		W:        w,
		H:        h,
		_dots:    make([]Mat, w * h),
		Dots:     make([][]Mat, w),
		_oxid:    make([]float64, w * h),
		Oxid:     make([][]float64, w),
		_spawner: make([]bool, w * h),
		Spawner:  make([][]bool, w),
		_spwnMat: make([]Mat, w * h),
		SpwnMat:  make([][]Mat, w),
		_states:  make([]State, w * h),
		States:   make([][]State, w),
		_thermo:  make([]float64, w * h),
		Thermo:   make([][]float64, w),
		_weights: make([]float64, w * h),
		Weights:  make([][]float64, w),
	}

	for x := 0; x < w; x++ {
		ret.Dots[x] = ret._dots[x * h:h + (x * h)]
		ret.Oxid[x] = ret._oxid[x * h:h + (x * h)]
		ret.Spawner[x] = ret._spawner[x * h:h + (x * h)]
		ret.SpwnMat[x] = ret._spwnMat[x * h:h + (x * h)]
		ret.States[x] = ret._states[x * h:h + (x * h)]
		ret.Thermo[x] = ret._thermo[x * h:h + (x * h)]
		ret.Weights[x] = ret._weights[x * h:h + (x * h)]

		for y := 0; y < h; y++ {
			ret.Thermo[x][y] = temperature
		}
	}

	return ret
}

func (w World) CanDisplace(x, y, dx, dy int) bool {
	if None == w.Dots[dx][dy] {
		return true
	}

	if MsStatic == w.States[dx][dy] {
		return false
	}

	if w.Weights[dx][dy] < w.Weights[x][y] {
		return true
	}

	return false
}

func (w *World) clearDot(
	x, y int,
) {
	w.Dots[x][y] = None
	w.States[x][y] = MsStatic
	w.Thermo[x][y] = 0
}

// You may want to call World.Simulate() after this.
func (w *World) Update(
	spawnerTemperature float64,
) {
	var (
		x, y int
	)

	updateDotFromThermo := func(
		x, y int,
	) {
		if w.Thermo[x][y] < MeltPs(w.Dots[x][y]) {
			w.States[x][y] = SolidSs(w.Dots[x][y])
			w.Weights[x][y] = SolidWeights(w.Dots[x][y])
		} else if w.Thermo[x][y] < BoilPs(w.Dots[x][y]) {
			w.States[x][y] = MsLiquid

			if w.Dots[x][y] == Sand {
				w.Dots[x][y] = Glass
			}

			w.Weights[x][y] = SolidWeights(w.Dots[x][y]) *
				WeightFactorLiquid
		} else {
			w.States[x][y] = MsGas
			w.Weights[x][y] = SolidWeights(w.Dots[x][y]) *
				WeightFactorGas
			w.Weights[x][y] -= w.Weights[x][y] *
				(w.Thermo[x][y] - BoilPs(w.Dots[x][y])) /
				WeightLossLimitGas
		}
	}

	for x = 0; x < w.W; x++ {
		for y = 0; y < w.H; y++ {
			if w.Spawner[x][y] {
				w.Dots[x][y] = w.SpwnMat[x][y]
				w.Thermo[x][y] = spawnerTemperature
			}

			updateDotFromThermo(x, y)
		}
	}
}

func (w *World) UseBrush(
	m Mat,
	t float64,
	xC, yC int,
	radius int,
) {
	var (
		x1 = xC - radius
		x2 = xC + radius
		y1 = yC - radius
		y2 = yC + radius
	)

	if x1 < 0 {
		x1 = 0
	}
	if x2 >= w.W {
		x2 = w.W - 1
	}
	if y1 < 0 {
		y1 = 0
	}
	if y2 >= w.H {
		y2 = w.H - 1
	}

	for x := x1; x <= x2; x++ {
		for y := y1; y <= y2; y++ {
			w.Dots[x][y] = m
			w.Thermo[x][y] = t
		}
	}
}

func (w *World) UseEraser(
	xC, yC int,
	radius int,
) {
	var (
		x1 = xC - radius
		x2 = xC + radius
		y1 = yC - radius
		y2 = yC + radius
	)

	if x1 < 0 {
		x1 = 0
	}
	if x2 >= w.W {
		x2 = w.W - 1
	}
	if y1 < 0 {
		y1 = 0
	}
	if y2 >= w.H {
		y2 = w.H - 1
	}

	for x := x1; x <= x2; x++ {
		for y := y1; y <= y2; y++ {
			w.clearDot(x, y)
			w.Spawner[x][y] = false
		}
	}
}

// Using this to increase temperature, by giving a negative delta,
// is inefficient. Cooling requires an additional check.
// To heat, see UseHeater
func (w *World) UseCooler(
	delta float64,
	xC, yC int,
	radius int,
) {
	var (
		x1 = xC - radius
		x2 = xC + radius
		y1 = yC - radius
		y2 = yC + radius
	)

	if x1 < 0 {
		x1 = 0
	}
	if x2 >= w.W {
		x2 = w.W - 1
	}
	if y1 < 0 {
		y1 = 0
	}
	if y2 >= w.H {
		y2 = w.H - 1
	}

	for x := x1; x <= x2; x++ {
		for y := y1; y <= y2; y++ {
			w.Thermo[x][y] -= delta

			if w.Thermo[x][y] < 0.0 {
				w.Thermo[x][y] = 0.0
			}
		}
	}
}

// Using this to decrease temperature, by giving a negative delta,
// may cause issues, as soon as the temperature of a dot goes negative.
// To cool, see UseCooler
func (w *World) UseHeater(
	delta float64,
	xC, yC int,
	radius int,
) {
	var (
		x1 = xC - radius
		x2 = xC + radius
		y1 = yC - radius
		y2 = yC + radius
	)

	if x1 < 0 {
		x1 = 0
	}
	if x2 >= w.W {
		x2 = w.W - 1
	}
	if y1 < 0 {
		y1 = 0
	}
	if y2 >= w.H {
		y2 = w.H - 1
	}

	for x := x1; x <= x2; x++ {
		for y := y1; y <= y2; y++ {
			w.Thermo[x][y] += delta
		}
	}
}

// You SHOULD call World.Update() before this.
func (w *World) Simulate(
) {
	var (
		x, y int
		bodySim, bodySimToRight, bodySimToLeft func()
	)

	doChemicalReaction := func(x, y, dx, dy int) {
		if OxidTh(w.Dots[x][y]) > 0.0 {
			if Oxygen == w.Dots[dx][dy] {
				if w.Thermo[x][y] > IgnPs(w.Dots[x][y]) {
					th := OxidTh(w.Dots[x][y]) *
						OxidSpd(w.Dots[x][y]) /
						2.0

					w.Oxid[x][y] += OxidSpd(w.Dots[x][y])
					w.Thermo[x][y] += th
					w.Thermo[dx][dy] += th

					if w.Oxid[x][y] >= 1.0 {
						op1, op2 := OxidPrdcts(w.Dots[x][y])

						w.Dots[x][y] = op1
						w.Dots[dx][dy] = op2
						w.Oxid[x][y] = 0.0
					}
				}
			}
		}
	}

	doGravity := func(x, y int) {
		switch w.States[x][y] {
		case MsGas:
			w.dropGas(x, y)

		case MsGrain:
			w.dropGrain(x, y)

		case MsLiquid:
			w.dropLiquid(x, y)

		default:
		}
	}

	doThConduction := func(
		x, y, x2, y2 int,
	) {
		if None == w.Dots[x2][y2] {
			return
		}

		combCond := ((ThCond(w.Dots[x][y]) + ThCond(w.Dots[x2][y2])) / 2)
		c1 := (w.Thermo[x2][y2] - w.Thermo[x][y]) * combCond
		c2 := (w.Thermo[x][y] - w.Thermo[x2][y2]) * combCond

		w.Thermo[x][y] += c1
		w.Thermo[x2][y2] += c2
	}

	bodySimToRight = func() {
		for x = 1; x <= w.W - 2; x++ {
			if None == w.Dots[x][y] {
				continue
			}

			doThConduction(x, y, x, y + 1)
			doThConduction(x, y, x - 1, y)
			doThConduction(x, y, x + 1, y)
			doChemicalReaction(x, y, x, y + 1)
			doChemicalReaction(x, y, x, y - 1)
			doChemicalReaction(x, y, x - 1, y)
			doChemicalReaction(x, y, x + 1, y)
			doGravity(x, y)
		}
		bodySim = bodySimToLeft
	}

	bodySimToLeft = func() {
		for x = w.W - 2; x >= 1; x-- {
			if None == w.Dots[x][y] {
				continue
			}

			doThConduction(x, y, x, y + 1)
			doThConduction(x, y, x - 1, y)
			doThConduction(x, y, x + 1, y)
			doChemicalReaction(x, y, x, y + 1)
			doChemicalReaction(x, y, x, y - 1)
			doChemicalReaction(x, y, x - 1, y)
			doChemicalReaction(x, y, x + 1, y)
			doGravity(x, y)
		}
		bodySim = bodySimToRight
	}

	y = w.H - 1;
	for x = 1; x <= w.W - 2; x++ {
		if None == w.Dots[x][y] {
			continue
		}

		doThConduction(x, y, x - 1, y)
		doThConduction(x, y, x + 1, y)
		doChemicalReaction(x, y, x - 1, y)
		doChemicalReaction(x, y, x + 1, y)
		doChemicalReaction(x, y, x, y - 1)
	}

	bodySim = bodySimToRight
	for y = w.H - 2; y > 0; y-- {
		bodySim()
	}

	y = 0;
	for x = 1; x <= w.W - 2; x++ {
		if None == w.Dots[x][y] {
			continue
		}

		doThConduction(x, y, x, y + 1)
		doThConduction(x, y, x - 1, y)
		doThConduction(x, y, x + 1, y)
		doChemicalReaction(x, y, x, y + 1)
		doChemicalReaction(x, y, x - 1, y)
		doChemicalReaction(x, y, x + 1, y)
		doGravity(x, y)
	}

	x = 0
	for y = w.H - 2; y >= 0; y-- {
		if None == w.Dots[x][y] {
			continue
		}

		doThConduction(x, y, x, y + 1)
		doChemicalReaction(x, y, x, y + 1)
		doChemicalReaction(x, y, x + 1, y)
		doGravity(x, y)
	}

	x = w.W - 1;
	for y = w.H - 2; y >= 0; y-- {
		if None == w.Dots[x][y] {
			continue
		}

		doThConduction(x, y, x, y + 1)
		doChemicalReaction(x, y, x, y + 1)
		doChemicalReaction(x, y, x - 1, y)
		doGravity(x, y)
	}
}

func (w *World) dropGas(
	x, y int,
) {
	var (
		dx, dy int
	)

	dx = x
	dy = y + 1
	if w.CanDisplace(x, y, dx, dy) {
		w.swapDots(x, y, dx, dy)
		return
	}

	stackCollapse := func(
		x, y, dx, dy int,
	) bool {
		if w.CanDisplace(x, y, dx, dy) {
			w.swapDots(x, y, dx, dy)
			return false
		}

		if w.States[dx][dy] == MsStatic ||
		   w.States[dx][dy] == MsGrain {
			return true
		}

		return false
	}

	dy = y + 1
	for dx = x - 1; dx >= 0; dx-- {
		if stackCollapse(x, y, dx, dy) {
			break
		}
	}
	for dx = x + 1; dx < w.W; dx++ {
		if stackCollapse(x, y, dx, dy) {
			break
		}
	}
}

func (w *World) dropGrain(
	x, y int,
) {
	var (
		dx, dy int
	)

	dx = x
	dy = y + 1
	if w.CanDisplace(x, y, dx, dy) {
		w.swapDots(x, y, dx, dy)
		return
	}

	if x - 1 >= 0 {
		dx = x - 1
		dy = y + 1

		if w.CanDisplace(x, y, dx, dy) {
			w.swapDots(x, y, dx, dy)
			return
		}
	}
	if x + 1 < w.W {
		dx = x + 1
		dy = y + 1

		if w.CanDisplace(x, y, dx, dy) {
			w.swapDots(x, y, dx, dy)
			return
		}
	}
}

func (w *World) dropLiquid(
	x, y int,
) {
	var (
		dx, dy int
	)

	dx = x
	dy = y + 1
	if w.CanDisplace(x, y, dx, dy) {
		w.swapDots(x, y, dx, dy)
		return
	}

	stackCollapse := func(
		x, y, dx, dy int,
	) bool {
		if w.CanDisplace(x, y, dx, dy) {
			w.swapDots(x, y, dx, dy)
			return false
		}

		if w.States[dx][dy] == MsStatic ||
		   w.States[dx][dy] == MsGrain {
			return true
		}

		return false
	}

	dy = y + 1
	for dx = x - 1; dx >= 0; dx-- {
		if stackCollapse(x, y, dx, dy) {
			break
		}
	}
	for dx = x + 1; dx < w.W; dx++ {
		if stackCollapse(x, y, dx, dy) {
			break
		}
	}
}

// Swaps all properties of two coordinates.
func (w *World) swapDots(
	x, y int,
	x2, y2 int,
) {
	var (
		tmpM = w.Dots[x][y]
		tmpO = w.Oxid[x][y]
		tmpS = w.States[x][y]
		tmpT = w.Thermo[x][y]
	)

	w.Dots[x][y] = w.Dots[x2][y2]
	w.Oxid[x][y] = w.Oxid[x2][y2]
	w.States[x][y] = w.States[x2][y2]
	w.Thermo[x][y] = w.Thermo[x2][y2]

	w.Dots[x2][y2] = tmpM
	w.Oxid[x2][y2] = tmpO
	w.States[x2][y2] = tmpS
	w.Thermo[x2][y2] = tmpT
}
