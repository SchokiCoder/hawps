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
}

func NewWorld(
	w, h int,
) World {
	var ret = World{
		W:     w,
		H:     h,
		_dots: make([]Mat, w * h),
		Dots:  make([][]Mat, w),
		_spawner: make([]Mat, w * h),
		Spawner:  make([][]Mat, w),
	}

	for i := 0; i < w; i++ {
		ret.Dots[i] = ret._dots[i * h:h + (i * h)]
		ret.Spawner[i] = ret._spawner[i * h:h + (i * h)]
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
	for y := w.H - 2; y >= 0; y-- {
		for x := 0; x < w.W; x++ {
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
	}
}

func (w *World) dropGas(
	x, y int,
) {
	var (
		bx     int
		below *Mat
		cur   *Mat
		tmp    Mat
	)

	canGasDisplace := func(src, dest Mat) bool {
		switch (States(dest)) {
		case MsNone: fallthrough
		case MsGas:
			if (Weights(dest) < Weights(src)) {
				return true
			}

		default:
		}

		return false
	}

	cur = &w.Dots[x][y]
	below = &w.Dots[x][y + 1]

	if canGasDisplace(*cur, *below) {
		tmp = *below
		*below = *cur
		*cur = tmp
		return
	}

	for bx = x - 1; bx >= 0; bx-- {
		below = &w.Dots[bx][y + 1]
		if canGasDisplace(*cur, *below) {
			tmp = *below
			*below = *cur
			*cur = tmp
			return
		}

		if States(*below) == MsStatic ||
		   States(*below) == MsGrain {
			break
		}
	}
	for bx = x + 1; bx < w.W; bx++ {
		below = &w.Dots[bx][y + 1]
		if canGasDisplace(*cur, *below) {
			tmp = *below
			*below = *cur
			*cur = tmp
			return
		}

		if States(*below) == MsStatic ||
		   States(*below) == MsGrain {
			break
		}
	}
}

func (w *World) dropGrain(
	x, y int,
) {
	var (
		below *Mat
		cur *Mat
		tmp Mat
	)

	canGrainDisplace := func(src, dest Mat) bool {
		switch (States(dest)) {
		case MsNone: fallthrough
		case MsGas:
			return true

		case MsLiquid:
			if (Weights(dest) < Weights(src)) {
				return true
			}

		default:
		}

		return false
	}

	cur = &w.Dots[x][y]
	below = &w.Dots[x][y + 1]

	if canGrainDisplace(*cur, *below) {
		tmp = *below
		*below = *cur
		*cur = tmp
		return
	}

	if x - 1 >= 0 {
		below = &w.Dots[x - 1][y + 1]

		if canGrainDisplace(*cur, *below) {
			tmp = *below
			*below = *cur
			*cur = tmp
			return
		}
	}
	if x + 1 < w.W {
		below = &w.Dots[x + 1][y + 1]

		if canGrainDisplace(*cur, *below) {
			tmp = *below
			*below = *cur
			*cur = tmp
			return
		}
	}
}

func (w *World) dropLiquid(
	x, y int,
) {
	var (
		bx int
		below *Mat
		cur *Mat
		tmp Mat
	)

	canLiquidDisplace := func(src, dest Mat) bool {
		switch States(dest) {
		case MsNone: fallthrough
		case MsGas:
			return true

		default:
			if (Weights(dest) < Weights(src)) {
				return true
			}
		}

		return false
	}

	cur = &w.Dots[x][y]
	below = &w.Dots[x][y + 1]

	if canLiquidDisplace(*cur, *below) {
		tmp = *below
		*below = *cur
		*cur = tmp
		return
	}

	for bx = x - 1; bx >= 0; bx-- {
		below = &w.Dots[bx][y + 1]
		if canLiquidDisplace(*cur, *below) {
			tmp = *below
			*below = *cur
			*cur = tmp
			return
		}

		if States(*below) == MsStatic ||
		   States(*below) == MsGrain {
			break
		}
	}
	for bx = x + 1; bx < w.W; bx++ {
		below = &w.Dots[bx][y + 1]
		if canLiquidDisplace(*cur, *below) {
			tmp = *below
			*below = *cur
			*cur = tmp
			return
		}

		if States(*below) == MsStatic ||
		   States(*below) == MsGrain {
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
