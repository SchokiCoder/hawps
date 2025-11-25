// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

package core

import "github.com/SchokiCoder/hawps/core/mat"

type World struct {
	bodySim func(*int, int)

	W        int
	H        int

	_spawner []bool
	Spawner  [][]bool
	_spwnMat []mat.Mat
	SpwnMat  [][]mat.Mat

	_dot     []mat.Mat
	Dot      [][]mat.Mat
	_oxid    []float64
	Oxid     [][]float64
	_state   []mat.State
	State    [][]mat.State
	_thermo  []float64
	Thermo   [][]float64
	_weight  []float64
	Weight   [][]float64
}

func NewWorld(
	w, h int,
	temperature float64,
) World {
	var ret = World{
		W:        w,
		H:        h,
		_dot:     make([]mat.Mat, w * h),
		Dot:      make([][]mat.Mat, w),
		_oxid:    make([]float64, w * h),
		Oxid:     make([][]float64, w),
		_spawner: make([]bool, w * h),
		Spawner:  make([][]bool, w),
		_spwnMat: make([]mat.Mat, w * h),
		SpwnMat:  make([][]mat.Mat, w),
		_state:   make([]mat.State, w * h),
		State:    make([][]mat.State, w),
		_thermo:  make([]float64, w * h),
		Thermo:   make([][]float64, w),
		_weight:  make([]float64, w * h),
		Weight:   make([][]float64, w),
	}

	for x := 0; x < w; x++ {
		ret.Dot[x] = ret._dot[x * h:h + (x * h)]
		ret.Oxid[x] = ret._oxid[x * h:h + (x * h)]
		ret.Spawner[x] = ret._spawner[x * h:h + (x * h)]
		ret.SpwnMat[x] = ret._spwnMat[x * h:h + (x * h)]
		ret.State[x] = ret._state[x * h:h + (x * h)]
		ret.Thermo[x] = ret._thermo[x * h:h + (x * h)]
		ret.Weight[x] = ret._weight[x * h:h + (x * h)]

		for y := 0; y < h; y++ {
			ret.Thermo[x][y] = temperature
		}
	}

	return ret
}

func (w World) CanDisplace(x, y, dx, dy int) bool {
	if mat.None == w.Dot[dx][dy] {
		return true
	}

	if mat.Static == w.State[dx][dy] {
		return false
	}

	if w.Weight[dx][dy] < w.Weight[x][y] {
		return true
	}

	return false
}

func (w *World) clearDot(
	x, y int,
) {
	w.Dot[x][y] = mat.None
	w.State[x][y] = mat.Static
	w.Thermo[x][y] = 0
}

// You may want to call World.Simulate() after this.
func (w *World) Update(
	spawnerTemperature float64,
) {
	var (
		x, y int
	)

	for x = 0; x < w.W; x++ {
		for y = 0; y < w.H; y++ {
			if w.Spawner[x][y] {
				w.Dot[x][y] = w.SpwnMat[x][y]
				w.Thermo[x][y] = spawnerTemperature
			}

			w.updateDotFromThermo(x, y)
		}
	}
}

func (w *World) updateDotFromThermo(
	x, y int,
) {
	var meltPrdct mat.Mat

	if w.Thermo[x][y] < mat.MeltP(w.Dot[x][y]) {
		w.State[x][y] = mat.SolidS(w.Dot[x][y])
		w.Weight[x][y] = mat.SolidWeight(w.Dot[x][y])
	} else if w.Thermo[x][y] < mat.BoilP(w.Dot[x][y]) {
		w.State[x][y] = mat.Liquid

		meltPrdct = mat.MeltPrdct(w.Dot[x][y])
		if meltPrdct != mat.None {
			w.Dot[x][y] = meltPrdct
		}

		w.Weight[x][y] = mat.SolidWeight(w.Dot[x][y]) *
			mat.WeightFactorLiquid
	} else {
		w.State[x][y] = mat.Gas
		w.Weight[x][y] = mat.SolidWeight(w.Dot[x][y]) *
			mat.WeightFactorGas
		w.Weight[x][y] -= w.Weight[x][y] *
			(w.Thermo[x][y] - mat.BoilP(w.Dot[x][y])) /
			mat.WeightLossLimitGas
	}
}

func (w *World) UseBrush(
	m mat.Mat,
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
			w.Dot[x][y] = m
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
	)

	y = w.H - 1;
	for x = 1; x <= w.W - 2; x++ {
		if mat.None == w.Dot[x][y] {
			continue
		}

		w.simThConduction(x, y, x - 1, y)
		w.simThConduction(x, y, x + 1, y)
		w.simChemicalReaction(x, y, x - 1, y)
		w.simChemicalReaction(x, y, x + 1, y)
		w.simChemicalReaction(x, y, x, y - 1)
	}

	w.bodySim = w.simToRight
	for y = w.H - 2; y > 0; y-- {
		w.bodySim(&x, y)
	}

	y = 0;
	for x = 1; x <= w.W - 2; x++ {
		if mat.None == w.Dot[x][y] {
			continue
		}

		w.simThConduction(x, y, x, y + 1)
		w.simThConduction(x, y, x - 1, y)
		w.simThConduction(x, y, x + 1, y)
		w.simChemicalReaction(x, y, x, y + 1)
		w.simChemicalReaction(x, y, x - 1, y)
		w.simChemicalReaction(x, y, x + 1, y)
		w.simGravity(x, y)
	}

	x = 0
	for y = w.H - 2; y >= 0; y-- {
		if mat.None == w.Dot[x][y] {
			continue
		}

		w.simThConduction(x, y, x, y + 1)
		w.simChemicalReaction(x, y, x, y + 1)
		w.simChemicalReaction(x, y, x + 1, y)
		w.simGravity(x, y)
	}

	x = w.W - 1;
	for y = w.H - 2; y >= 0; y-- {
		if mat.None == w.Dot[x][y] {
			continue
		}

		w.simThConduction(x, y, x, y + 1)
		w.simChemicalReaction(x, y, x, y + 1)
		w.simChemicalReaction(x, y, x - 1, y)
		w.simGravity(x, y)
	}
}

func (w *World) simChemicalReaction(
	x, y, dx, dy int,
) {
	if mat.OxidTh(w.Dot[x][y]) > 0.0 {
		if mat.Oxygen == w.Dot[dx][dy] {
			if w.Thermo[x][y] > mat.IgnP(w.Dot[x][y]) {
				th := mat.OxidTh(w.Dot[x][y]) *
					mat.OxidSpd(w.Dot[x][y]) /
					2.0

				w.Oxid[x][y] += mat.OxidSpd(w.Dot[x][y])
				w.Thermo[x][y] += th
				w.Thermo[dx][dy] += th

				if w.Oxid[x][y] >= 1.0 {
					op1, op2 := mat.OxidPrdcts(w.Dot[x][y])

					w.Dot[x][y] = op1
					w.Dot[dx][dy] = op2
					w.Oxid[x][y] = 0.0
				}
			}
		}
	}
}

func (w *World) simGravity(
	x, y int,
) {
	switch w.State[x][y] {
	case mat.Gas:
		w.dropGas(x, y)

	case mat.Grain:
		w.dropGrain(x, y)

	case mat.Liquid:
		w.dropLiquid(x, y)

	default:
	}
}

func (w *World) simThConduction(
	x, y, x2, y2 int,
) {
	if mat.None == w.Dot[x2][y2] {
		return
	}

	combCond := ((mat.ThCond(w.Dot[x][y]) + mat.ThCond(w.Dot[x2][y2])) / 2)
	c1 := (w.Thermo[x2][y2] - w.Thermo[x][y]) * combCond
	c2 := (w.Thermo[x][y] - w.Thermo[x2][y2]) * combCond

	w.Thermo[x][y] += c1
	w.Thermo[x2][y2] += c2
}

func (w *World) simToRight(
	x *int,
	y int,
) {
	for *x = 1; *x <= w.W - 2; *x++ {
		if mat.None == w.Dot[*x][y] {
			continue
		}

		w.simThConduction(*x, y, *x, y + 1)
		w.simThConduction(*x, y, *x - 1, y)
		w.simThConduction(*x, y, *x + 1, y)
		w.simChemicalReaction(*x, y, *x, y + 1)
		w.simChemicalReaction(*x, y, *x, y - 1)
		w.simChemicalReaction(*x, y, *x - 1, y)
		w.simChemicalReaction(*x, y, *x + 1, y)
		w.simGravity(*x, y)
	}
	w.bodySim = w.simToLeft
}

func (w *World) simToLeft(
	x *int,
	y int,
) {
	for *x = w.W - 2; *x >= 1; *x-- {
		if mat.None == w.Dot[*x][y] {
			continue
		}

		w.simThConduction(*x, y, *x, y + 1)
		w.simThConduction(*x, y, *x - 1, y)
		w.simThConduction(*x, y, *x + 1, y)
		w.simChemicalReaction(*x, y, *x, y + 1)
		w.simChemicalReaction(*x, y, *x, y - 1)
		w.simChemicalReaction(*x, y, *x - 1, y)
		w.simChemicalReaction(*x, y, *x + 1, y)
		w.simGravity(*x, y)
	}
	w.bodySim = w.simToRight
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

		if w.State[dx][dy] == mat.Static ||
		   w.State[dx][dy] == mat.Grain {
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

		if w.State[dx][dy] == mat.Static ||
		   w.State[dx][dy] == mat.Grain {
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
		tmpM = w.Dot[x][y]
		tmpO = w.Oxid[x][y]
		tmpS = w.State[x][y]
		tmpT = w.Thermo[x][y]
	)

	w.Dot[x][y] = w.Dot[x2][y2]
	w.Oxid[x][y] = w.Oxid[x2][y2]
	w.State[x][y] = w.State[x2][y2]
	w.Thermo[x][y] = w.Thermo[x2][y2]

	w.Dot[x2][y2] = tmpM
	w.Oxid[x2][y2] = tmpO
	w.State[x2][y2] = tmpS
	w.Thermo[x2][y2] = tmpT
}
