// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

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
	ThermalVisionMinT  = -75
	WeightFactorLiquid = 0.95
	WeightFactorGas    = 0.90
	WeightLossLimitGas = 5000.0
)

var (
	_weights = [...]float64 {0.0,      1.5,     1.5,      0.999,    7.874,    0.001323, 0.00008319, 0.001977,   0.000657} /* g/cm³ */
	_boilPs  = [...]float64 {0,        2950,    2950,     100,      2861,     -182.96,  -252.88,    -78.464,    -182.50}  /* °C */
	_ignPs   = [...]float64 {0,        0,       0,        0,        0,        0,        0,          0,          580.0}    /* °C */
	_meltPs  = [...]float64 {0,        1713,    1713,     0,        1538,     -218.79,  -259.16,    -56.561,    -161.5}   /* °C */
	_oxidTh  = [...]float64 {0,        0,       0,        0,        0,        0,        0,          0,          1963.0}   /* °C released on oxidation */
	_oxidSpd = [...]float64 {0,        0,       0,        0,        0,        0,        0,          0,          0.2}      /* fraction per tick */
	_solidSs = [...]State   {MsStatic, MsGrain, MsStatic, MsStatic, MsStatic, MsStatic, MsStatic,   MsStatic,   MsStatic} /* state when solid */
	_thCond  = [...]float64 {0.0,      0.00673, 0.00673,  0.0061,   0.084,    0.002,    0.0018,     0.00146,    0.003}    /* W/(m⋅K)/1000 but flattened so that at most two zeroes are after the dot */

	_solRs   = [...]uint8   {0,        238,     237,      150,      200,      45,       45,         45,         65}
	_solGs   = [...]uint8   {0,        217,     237,      150,      200,      45,       45,         45,         65}
	_solBs   = [...]uint8   {0,        86,      237,      255,      200,      80,       80,         80,         65}

	_liqRs   = [...]uint8   {0,        255,     255,      100,      255,      25,       25,         25,         45}
	_liqGs   = [...]uint8   {0,        110,     110,      100,      25,       25,       25,         25,         45}
	_liqBs   = [...]uint8   {0,        56,      56,       255,      25,       60,       60,         60,         45}

	_gasRs   = [...]uint8   {0,        255,     255,      200,      255,      5,        5,          5,          25}
	_gasGs   = [...]uint8   {0,        0,       0,        200,      0,        5,        5,          5,          25}
	_gasBs   = [...]uint8   {0,        0,       0,        255,      0,        40,       40,         40,         25}
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

func SolRs(
	i Mat,
) uint8 {
	return _solRs[i]
}

func SolGs(
	i Mat,
) uint8 {
	return _solGs[i]
}

func SolBs(
	i Mat,
) uint8 {
	return _solBs[i]
}

func LiqRs(
	i Mat,
) uint8 {
	return _liqRs[i]
}

func LiqGs(
	i Mat,
) uint8 {
	return _liqGs[i]
}

func LiqBs(
	i Mat,
) uint8 {
	return _liqBs[i]
}

func GasRs(
	i Mat,
) uint8 {
	return _gasRs[i]
}

func GasGs(
	i Mat,
) uint8 {
	return _gasGs[i]
}

func GasBs(
	i Mat,
) uint8 {
	return _gasBs[i]
}

type World struct {
	W        int
	H        int

	_rs      []uint8
	Rs       [][]uint8
	_gs      []uint8
	Gs       [][]uint8
	_bs      []uint8
	Bs       [][]uint8
	ThVision bool

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
		_rs:      make([]uint8, w * h),
		Rs:       make([][]uint8, w),
		_gs:      make([]uint8, w * h),
		Gs:       make([][]uint8, w),
		_bs:      make([]uint8, w * h),
		Bs:       make([][]uint8, w),
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
		ret.Rs[x] = ret._rs[x * h:h + (x * h)]
		ret.Gs[x] = ret._gs[x * h:h + (x * h)]
		ret.Bs[x] = ret._bs[x * h:h + (x * h)]
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
	w.Rs[x][y] = 0
	w.Gs[x][y] = 0
	w.Bs[x][y] = 0
	w.Dots[x][y] = None
	w.States[x][y] = MsStatic
	w.Thermo[x][y] = 0
}

// You may want to call World.Simulate() after this.
func (w *World) Update(
	spawnerTemperature float64,
) {
	var (
		rgbOverride func(int, int)
		x, y int
	)

	doThVision := func(
		x, y int,
	) {
		if None == w.Dots[x][y] {
			return
		}

		visT := w.Thermo[x][y]
		if visT > ThermalVisionMinT + 255 {
			visT = ThermalVisionMinT + 255
		} else if visT < ThermalVisionMinT {
			visT = ThermalVisionMinT
		}

		gray := uint8(visT - ThermalVisionMinT)
		w.Rs[x][y] = gray
		w.Gs[x][y] = gray
		w.Bs[x][y] = gray
	}

	updateDotFromThermo := func(
		x, y int,
	) {
		if w.Thermo[x][y] < MeltPs(w.Dots[x][y]) {
			w.States[x][y] = SolidSs(w.Dots[x][y])
			w.Weights[x][y] = SolidWeights(w.Dots[x][y])
			w.Rs[x][y] = SolRs(w.Dots[x][y])
			w.Gs[x][y] = SolGs(w.Dots[x][y])
			w.Bs[x][y] = SolBs(w.Dots[x][y])
		} else if w.Thermo[x][y] < BoilPs(w.Dots[x][y]) {
			w.States[x][y] = MsLiquid

			if w.Dots[x][y] == Sand {
				w.Dots[x][y] = Glass
			}

			w.Weights[x][y] = SolidWeights(w.Dots[x][y]) *
				WeightFactorLiquid
			w.Rs[x][y] = LiqRs(w.Dots[x][y])
			w.Gs[x][y] = LiqGs(w.Dots[x][y])
			w.Bs[x][y] = LiqBs(w.Dots[x][y])
		} else {
			w.States[x][y] = MsGas
			w.Weights[x][y] = SolidWeights(w.Dots[x][y]) *
				WeightFactorGas
			w.Weights[x][y] -= w.Weights[x][y] *
				(w.Thermo[x][y] - BoilPs(w.Dots[x][y])) /
				WeightLossLimitGas
			w.Rs[x][y] = GasRs(w.Dots[x][y])
			w.Gs[x][y] = GasGs(w.Dots[x][y])
			w.Bs[x][y] = GasBs(w.Dots[x][y])
		}
	}

	if w.ThVision {
		rgbOverride = doThVision
	} else {
		rgbOverride = func(int, int){}
	}

	for x = 0; x < w.W; x++ {
		for y = 0; y < w.H; y++ {
			if w.Spawner[x][y] {
				w.Dots[x][y] = w.SpwnMat[x][y]
				w.Thermo[x][y] = spawnerTemperature
			}

			updateDotFromThermo(x, y)

			rgbOverride(x, y)
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

func (w *World) UseThermoChanger(
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
	)

	doChemicalReaction := func(x, y, dx, dy int) {
		switch w.Dots[x][y] {
		case Oxygen:
			switch w.Dots[dx][dy] {
			case Hydrogen:
				w.clearDot(x, y)
				w.Dots[dx][dy] = Water

			case Methane:
				if w.Thermo[dx][dy] > IgnPs(w.Dots[dx][dy]) {
					w.Oxid[dx][dy] += OxidSpd(w.Dots[dx][dy])
					w.Thermo[x][y] += OxidTh(w.Dots[dx][dy]) *
						OxidSpd(w.Dots[dx][dy])

					if w.Oxid[dx][dy] >= 1.0 {
						w.Dots[x][y] = CarbonDioxide
						w.Dots[dx][dy] = Water
						w.Oxid[dx][dy] = 0.0
					}
				}
			}

		default:
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

	y = w.H - 1;
	for x = 1; x < w.W - 2; x++ {
		if None == w.Dots[x][y] {
			continue
		}

		doThConduction(x, y, x - 1, y)
		doThConduction(x, y, x + 1, y)
		doChemicalReaction(x, y, x - 1, y)
		doChemicalReaction(x, y, x + 1, y)
		doChemicalReaction(x, y, x, y - 1)
	}

	for y = w.H - 2; y > 0; y-- {
		if y % 2 == 0 {
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
		} else {
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
		}
	}

	y = 0;
	for x = 1; x < w.W - 2; x++ {
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
		doChemicalReaction(x, y, x, y - 1)
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
		doChemicalReaction(x, y, x, y - 1)
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
		tmpR = w.Rs[x][y]
		tmpG = w.Gs[x][y]
		tmpB = w.Bs[x][y]
		tmpM = w.Dots[x][y]
		tmpO = w.Oxid[x][y]
		tmpS = w.States[x][y]
		tmpT = w.Thermo[x][y]
	)

	w.Rs[x][y] = w.Rs[x2][y2]
	w.Gs[x][y] = w.Gs[x2][y2]
	w.Bs[x][y] = w.Bs[x2][y2]
	w.Dots[x][y] = w.Dots[x2][y2]
	w.Oxid[x][y] = w.Oxid[x2][y2]
	w.States[x][y] = w.States[x2][y2]
	w.Thermo[x][y] = w.Thermo[x2][y2]

	w.Rs[x2][y2] = tmpR
	w.Gs[x2][y2] = tmpG
	w.Bs[x2][y2] = tmpB
	w.Dots[x2][y2] = tmpM
	w.Oxid[x2][y2] = tmpO
	w.States[x2][y2] = tmpS
	w.Thermo[x2][y2] = tmpT
}

func ChangeBgColor(
	r, g, b uint8,
) {
	// change the color of mat None
	_solRs[0] = r
	_solGs[0] = g
	_solBs[0] = b
	_liqRs[0] = r
	_liqGs[0] = g
	_liqBs[0] = b
	_gasRs[0] = r
	_gasGs[0] = g
	_gasBs[0] = b
}
