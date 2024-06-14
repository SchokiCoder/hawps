// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package main

const (
	worldWidth  = 80
	worldHeight = 60
)

const (
	dotNone = 0
	dotSand
)

type (
	dotId        int
	dotMaterial  int
	world        [worldWidth][worldHeight]dotId
	dotsMaterial [worldWidth * worldHeight]dotMaterial
)

func newWorld() world {
	var ret world

	for x := 0; x < len(ret); x++ {
		for y := 0; y < len(ret[x]); y++ {
			ret[x][y] = -1
		}
	}

	return ret
}

func newDotsMaterial() dotsMaterial {
	var ret dotsMaterial
	
	for i := 0; i < len(ret); i++ {
		ret[i] = dotNone
	}

	return ret
}

func manuallySpawnSand(dMat dotsMaterial, wld world) {
	dMat[0] = dotSand
	wld[2][2] = 0
}

func main() {
	var (
		dMat dotsMaterial = newDotsMaterial()
		wld  world        = newWorld()
	)

	manuallySpawnSand(dMat, wld)
}
