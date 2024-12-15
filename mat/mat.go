// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

//go:generate stringer -type=Mat
package mat

type Mat int
const (
	None Mat = iota
	Sand
	Water
	Iron
	Oxygen
	Hydrogen
	FirstReal = Sand
	Last = Hydrogen

	Count int = iota
)

type State int
const (
	MsNone State = iota
	MsStatic
	MsGrain
	MsLiquid
	MsGas

	MsCount int = iota
)

// Don't touch that darn thing
var (
	_weights = [...]float64 {0.0,    1.5,     0.999,    7.874,    0.001323, 0.00008319} /* g/cm³ */
	_states  = [...]State   {MsNone, MsGrain, MsLiquid, MsStatic, MsGas,    MsGas}
	_rs      = [...]uint8   {0,      238,     100,      200,      5,        5}
	_gs      = [...]uint8   {0,      217,     100,      200,      5,        5}
	_bs      = [...]uint8   {0,      86,      255,      200,      40,       40}
)

func Weights(
	i Mat,
) float64 {
	return _weights[i]
}

func States(
	i Mat,
) State {
	return _states[i]
}

func Rs(
	i Mat,
) uint8 {
	return _rs[i]
}

func Gs(
	i Mat,
) uint8 {
	return _gs[i]
}

func Bs(
	i Mat,
) uint8 {
	return _bs[i]
}

type World struct {
	W        int
	H        int
	_dots    []Mat
	Dots     [][]Mat
	_spawner []Mat
	Spawner  [][]Mat
	_thermo  []int
	Thermo   [][]int
}

func NewWorld(
	w, h int,
	temperature int,
) World {
	var ret = World{
		W:     w,
		H:     h,
		_dots: make([]Mat, w * h),
		Dots:  make([][]Mat, w),
		_spawner: make([]Mat, w * h),
		Spawner:  make([][]Mat, w),
		_thermo: make([]int, w * h),
		Thermo:  make([][]int, w),
	}

	for x := 0; x < w; x++ {
		ret.Dots[x] = ret._dots[x * h:h + (x * h)]
		ret.Spawner[x] = ret._spawner[x * h:h + (x * h)]
		ret.Thermo[x] = ret._thermo[x * h:h + (x * h)]

		for y := 0; y < h; y++ {
			ret.Thermo[x][y] = temperature
		}
	}

	return ret
}

func (w *World) UseBrush(
	material Mat,
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
			w.Dots[x][y] = material
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
			w.Dots[x][y] = None
			w.Spawner[x][y] = None
		}
	}
}

func (w *World) Tick(
) {
	w.applyChemReactions()
	w.applyGravity()
	w.runSpawners()
}

func (w *World) applyChemReactions(
) {
	var x, y int

	react := func(src, dest *Mat) {
		switch *src {
		case Oxygen:
			if Hydrogen == *dest {
				*src = None
				*dest = Water
			}

		default:
		}
	}

	y = w.H - 1;
	for x = 1; x < w.W - 2; x++ {
		react(&w.Dots[x][y], &w.Dots[x - 1][y])
		react(&w.Dots[x][y], &w.Dots[x + 1][y])
	}

	for x = 1; x < w.W - 2; x++ {
		for y = w.H - 2; y >= 0; y-- {
			react(&w.Dots[x][y], &w.Dots[x][y + 1])
			react(&w.Dots[x][y], &w.Dots[x - 1][y])
			react(&w.Dots[x][y], &w.Dots[x + 1][y])
		}
	}

	x = 0
	for y = w.H - 2; y >= 0; y-- {
		react(&w.Dots[x][y], &w.Dots[x][y + 1])
	}

	x = w.W - 1;
	for y = w.H - 2; y >= 0; y-- {
		react(&w.Dots[x][y], &w.Dots[x][y + 1])
	}
}

func (w *World) applyGravity(
) {
	action := func(x, y int) {
		switch (States(w.Dots[x][y])) {
		case MsGas:
			w.dropGas(x, y)

		case MsGrain:
			w.dropGrain(x, y)

		case MsLiquid:
			w.dropLiquid(x, y)

		default:
		}
	}

	for y := w.H - 2; y >= 0; y-- {
		if y % 2 == 0 {
			for x := 0; x < w.W; x++ {
				action(x, y)
			}
		} else {
			for x := w.W - 1; x > 0; x-- {
				action(x, y)
			}
		}
	}
}

func (w *World) dropGas(
	x, y int,
) {
	var (
		dx, dy int
	)

	canGasDisplace := func(x, y, dx, dy int) bool {
		switch States(w.Dots[dx][dy]) {
		case MsNone: fallthrough
		case MsGas:
			if Weights(w.Dots[dx][dy]) < Weights(w.Dots[x][y]) {
				return true
			}

		default:
		}

		return false
	}

	dx = x
	dy = y + 1
	if canGasDisplace(x, y, dx, dy) {
		w.swapDots(x, y, dx, dy)
		return
	}

	stackCollapse := func(
		x, y, dx, dy int,
	) bool {
		if canGasDisplace(x, y, dx, dy) {
			w.swapDots(x, y, dx, dy)
			return false
		}

		if States(w.Dots[dx][dy]) == MsStatic ||
		   States(w.Dots[dx][dy]) == MsGrain {
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

	canGrainDisplace := func(x, y, dx, dy int) bool {
		switch (States(w.Dots[dx][dy])) {
		case MsNone: fallthrough
		case MsGas:
			return true

		case MsLiquid:
			if Weights(w.Dots[dx][dy]) < Weights(w.Dots[x][y]) {
				return true
			}

		default:
		}

		return false
	}

	dx = x
	dy = y + 1
	if canGrainDisplace(x, y, dx, dy) {
		w.swapDots(x, y, dx, dy)
		return
	}

	if x - 1 >= 0 {
		dx = x - 1
		dy = y + 1

		if canGrainDisplace(x, y, dx, dy) {
			w.swapDots(x, y, dx, dy)
			return
		}
	}
	if x + 1 < w.W {
		dx = x + 1
		dy = y + 1

		if canGrainDisplace(x, y, dx, dy) {
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

	canLiquidDisplace := func(x, y, dx, dy int) bool {
		switch States(w.Dots[dx][dy]) {
		case MsNone: fallthrough
		case MsGas:
			return true

		default:
			if Weights(w.Dots[dx][dy]) < Weights(w.Dots[x][y]) {
				return true
			}
		}

		return false
	}

	dx = x
	dy = y + 1
	if canLiquidDisplace(x, y, dx, dy) {
		w.swapDots(x, y, dx, dy)
		return
	}

	stackCollapse := func(
		x, y, dx, dy int,
	) bool {
		if canLiquidDisplace(x, y, dx, dy) {
			w.swapDots(x, y, dx, dy)
			return false
		}

		if States(w.Dots[dx][dy]) == MsStatic ||
		   States(w.Dots[dx][dy]) == MsGrain {
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

func (w *World) runSpawners(
) {
	for x := 0; x < w.W; x++ {
		for y := 0; y < w.H; y++ {
			if w.Spawner[x][y] != None {
				w.Dots[x][y] = w.Spawner[x][y]
			}
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
		tmpT = w.Thermo[x][y]
	)

	w.Dots[x][y] = w.Dots[x2][y2]
	w.Thermo[x][y] = w.Thermo[x2][y2]

	w.Dots[x2][y2] = tmpM
	w.Thermo[x2][y2] = tmpT
}
