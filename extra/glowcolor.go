// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

// This package contains things that are optional for a client.
package extra

import (
	"image/color"
)

const (
	glowMaxAlpha   = 200
	// Kelvin / 100
	glowRange    = 77
	noGlowRange  = 7  // draper point
	dimGlowRange = 40
)

var glowColors [glowRange]color.RGBA

func init(
) {
	/* Instead of doing expensive HSV stuff for temperature based glowing,
	 * we prepare an array of glow colors,
	 * which we then iterate through based on dot temperature.
	 */
	for i := 0; i < noGlowRange; i++ {
		glowColors[i] = color.RGBA{0, 0, 0, 0}
	}
	for i := noGlowRange; i < dimGlowRange; i++ {
		glowColors[i] = color.RGBA{255, 0, 0,
			uint8(glowMaxAlpha /
				100.0 *
				(100.0 /
					float64(dimGlowRange - noGlowRange) *
					float64(i - noGlowRange)))}
	}
	chromaGlow := [...]color.RGBA{
		color.RGBA{255, 0, 0, glowMaxAlpha},
		color.RGBA{255, 0, 0, glowMaxAlpha},
		color.RGBA{255, 0, 0, glowMaxAlpha},
		color.RGBA{255, 0, 0, glowMaxAlpha},
		color.RGBA{255, 0, 0, glowMaxAlpha},

		color.RGBA{255, 63, 0, glowMaxAlpha},
		color.RGBA{255, 127, 0, glowMaxAlpha},
		color.RGBA{255, 127, 0, glowMaxAlpha},
		color.RGBA{255, 190, 0, glowMaxAlpha},

		color.RGBA{255, 127, 0, glowMaxAlpha},
		color.RGBA{255, 255, 0, glowMaxAlpha},
		color.RGBA{127, 255, 0, glowMaxAlpha},

		color.RGBA{0, 255, 0, glowMaxAlpha},
		color.RGBA{0, 255, 0, glowMaxAlpha},
		color.RGBA{0, 255, 0, glowMaxAlpha},
		color.RGBA{0, 255, 85, glowMaxAlpha},
		color.RGBA{0, 255, 170, glowMaxAlpha},

		color.RGBA{0, 255, 255, glowMaxAlpha},
		color.RGBA{0, 255, 255, glowMaxAlpha},
		color.RGBA{0, 170, 255, glowMaxAlpha},

		color.RGBA{0, 85, 255, glowMaxAlpha},
		color.RGBA{0, 0, 255, glowMaxAlpha},
		color.RGBA{0, 0, 255, glowMaxAlpha},
		color.RGBA{0, 0, 255, glowMaxAlpha},
		color.RGBA{0, 0, 255, glowMaxAlpha},
		color.RGBA{0, 0, 255, glowMaxAlpha},
		color.RGBA{19, 0, 255, glowMaxAlpha},
		color.RGBA{37, 0, 255, glowMaxAlpha},
		color.RGBA{55, 0, 255, glowMaxAlpha},

		color.RGBA{73, 0, 255, glowMaxAlpha},
		color.RGBA{91, 0, 255, glowMaxAlpha},
		color.RGBA{109, 0, 255, glowMaxAlpha},
		color.RGBA{127, 0, 255, glowMaxAlpha},
		color.RGBA{127, 0, 255, glowMaxAlpha},
		color.RGBA{127, 0, 255, glowMaxAlpha},
		color.RGBA{127, 0, 255, glowMaxAlpha},
		color.RGBA{127, 0, 255, glowMaxAlpha / 3 * 2},
		color.RGBA{127, 0, 255, glowMaxAlpha / 3 * 1},
	}
	for i := dimGlowRange; i < glowRange; i++ {
		glowColors[i] = chromaGlow[i - dimGlowRange]
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
