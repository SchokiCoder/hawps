// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package main

import (
	"github.com/veandco/go-sdl2/sdl"
)

const (
	gfxScale = 10
	worldWidth  = 80
	worldHeight = 60
)

const (
	dotNone = 0
	dotSand
)

type (
	dotMaterial  int
	world        [worldWidth][worldHeight]int
	dotsMaterial [worldWidth * worldHeight]dotMaterial
	dotsX        [worldWidth * worldHeight]int
	dotsY        [worldWidth * worldHeight]int
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

func spawnDot(x, y int,
	mat dotMaterial,
	dCount *int,
	dMat *dotsMaterial,
	dX *dotsX,
	dY *dotsY,
	wld *world) {

	dMat[*dCount] = mat
	dX[*dCount] = x
	dY[*dCount] = y
	wld[x][y] = *dCount
	*dCount += 1
}

func main() {
	var (
		dCount int
		dMat   dotsMaterial = newDotsMaterial()
		dX     dotsX
		dY     dotsY
		wld    world        = newWorld()
	)

	if err := sdl.Init(sdl.INIT_EVERYTHING); err != nil {
		panic(err)
	}
	defer sdl.Quit()

	window, err := sdl.CreateWindow("test",
		sdl.WINDOWPOS_UNDEFINED,
		sdl.WINDOWPOS_UNDEFINED,
		worldWidth * gfxScale,
		worldHeight * gfxScale,
		sdl.WINDOW_SHOWN)
	if err != nil {
		panic(err)
	}
	defer window.Destroy()

	surface, err := window.GetSurface()
	if err != nil {
		panic(err)
	}
	surface.FillRect(nil, 0)

	spawnDot(2, 2, dotSand, &dCount, &dMat, &dX, &dY, &wld)

	for i := 0; i < dCount; i++ {
		rect := sdl.Rect{
			X: int32(dX[i] * gfxScale),
			Y: int32(dY[i] * gfxScale),
			W: int32(gfxScale),
			H: int32(gfxScale),
		}
		pixel := sdl.MapRGB(surface.Format, 150, 150, 20)
		surface.FillRect(&rect, pixel)
		window.UpdateSurface()
	}

	mainloop:
	for {
		event := sdl.PollEvent()

		for ; event != nil; event = sdl.PollEvent() {
			switch event.(type) {
			case *sdl.QuitEvent:
				break mainloop
			}
		}
	}
}
