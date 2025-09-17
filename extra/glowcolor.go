// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

// Instead of doing expensive HSV stuff for temperature based glowing,
// we prepare an array of glow colors,
// which we then iterate through based on dot temperature.
package extra

import (
	"image/color"
)

type glowRangeDef struct {
	I int
	R int
	G int
	B int
	A int
}

type rgbaFloat64 struct {
	R float64
	G float64
	B float64
	A float64
}

const (
	glowMaxAlpha   = 200
	// Kelvin / 100
	glowRange    = 78
)

var glowColors [glowRange]color.RGBA

var glowRanges := []glowRangeDef{
	glowRangeDef{I: 0,  R: 0,   G: 0,   B: 0,   A: 0},
	glowRangeDef{I: 7,  R: 255, G: 0,   B: 0,   A: 0}, // draper point
	glowRangeDef{I: 40, R: 255, G: 0,   B: 0,   A: glowMaxAlpha},
	glowRangeDef{I: 44, R: 255, G: 0,   B: 0,   A: glowMaxAlpha},
	glowRangeDef{I: 46, R: 255, G: 127, B: 0,   A: glowMaxAlpha},
	glowRangeDef{I: 48, R: 255, G: 190, B: 0,   A: glowMaxAlpha},
	glowRangeDef{I: 50, R: 255, G: 255, B: 0,   A: glowMaxAlpha},
	glowRangeDef{I: 52, R: 0,   G: 255, B: 0,   A: glowMaxAlpha},
	glowRangeDef{I: 54, R: 0,   G: 255, B: 0,   A: glowMaxAlpha},
	glowRangeDef{I: 57, R: 0,   G: 255, B: 255, A: glowMaxAlpha},
	glowRangeDef{I: 58, R: 0,   G: 255, B: 255, A: glowMaxAlpha},
	glowRangeDef{I: 61, R: 0,   G: 0,   B: 255, A: glowMaxAlpha},
	glowRangeDef{I: 65, R: 0,   G: 0,   B: 255, A: glowMaxAlpha},
	glowRangeDef{I: 72, R: 127, G: 0,   B: 255, A: glowMaxAlpha},
	glowRangeDef{I: 75, R: 127, G: 0,   B: 255, A: glowMaxAlpha},
	glowRangeDef{I: 78, R: 127, G: 0,   B: 255, A: 0},
}

func init() {
	var (
		a int
		b int
		rStep float64
		gStep float64
		bStep float64
		aStep float64
		tempGlowColors [glowRange]rgbaFloat64
	)

	glowRangeDefsToglowColorStep := func(i1, c1, i2, c2 int) float64 {
		return float64(c2 - c1) / float64(i2 - i1)
	}

	a = 1
	b = 0
	for ; a < glowRange; a++ {

		if a > glowRanges[b].I {
			b++
			rStep = glowRangeDefsToglowColorStep(
				glowRanges[b-1].I,
				glowRanges[b-1].R,
				glowRanges[b].I,
				glowRanges[b].R)
			gStep = glowRangeDefsToglowColorStep(
				glowRanges[b-1].I,
				glowRanges[b-1].G,
				glowRanges[b].I,
				glowRanges[b].G)
			bStep = glowRangeDefsToglowColorStep(
				glowRanges[b-1].I,
				glowRanges[b-1].B,
				glowRanges[b].I,
				glowRanges[b].B)
			aStep = glowRangeDefsToglowColorStep(
				glowRanges[b-1].I,
				glowRanges[b-1].A,
				glowRanges[b].I,
				glowRanges[b].A)
		}

		tempGlowColors[a] = rgbaFloat64{
			R: tempGlowColors[a-1].R + rStep,
			G: tempGlowColors[a-1].G + gStep,
			B: tempGlowColors[a-1].B + bStep,
			A: tempGlowColors[a-1].A + aStep,
		}

		glowColors[a].R = uint8(tempGlowColors[a].R)
		glowColors[a].G = uint8(tempGlowColors[a].G)
		glowColors[a].B = uint8(tempGlowColors[a].B)
		glowColors[a].A = uint8(tempGlowColors[a].A)
	}
}

func ThermoToColor(
	thermo float64,
) color.Color {
	var glowIndex int

	glowIndex = int(thermo / 100.0)
	if glowIndex >= glowRange {
		return color.RGBA{0, 0, 0, 0}
	}

	return glowColors[glowIndex]
}
