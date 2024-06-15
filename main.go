// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package main

import (
	"fmt"
	"github.com/veandco/go-sdl2/sdl"
	"time"
)

const (
	gfxScale = 10
	tickrate = 60
	worldWidth  = 80
	worldHeight = 60
)

const (
	dotNone = 0
	dotSand
)

type (
	dotMaterial  int
	dotsMaterial [worldWidth * worldHeight]dotMaterial
	dotsX        [worldWidth * worldHeight]float64
	dotsY        [worldWidth * worldHeight]float64
	dotsVelX     [worldWidth * worldHeight]float64
	dotsVelY     [worldWidth * worldHeight]float64
)

func newDotsMaterial() dotsMaterial {
	var ret dotsMaterial
	
	for i := 0; i < len(ret); i++ {
		ret[i] = dotNone
	}

	return ret
}

func drawWorld(dCount *int,
	dMat *dotsMaterial,
	dX *dotsX,
	dY *dotsY,
	surface *sdl.Surface,
	window  *sdl.Window) {

	surface.FillRect(nil, 0)

	for i := 0; i < *dCount; i++ {
		rect := sdl.Rect{
			X: int32(int64(dX[i]) * gfxScale),
			Y: int32(int64(dY[i]) * gfxScale),
			W: int32(gfxScale),
			H: int32(gfxScale),
		}
		pixel := sdl.MapRGB(surface.Format, 150, 150, 20)
		surface.FillRect(&rect, pixel)
		window.UpdateSurface()
	}
}

// Moves every dot.
// A dot is 1 cm big (W and H).
// Velocity is cm/s.
func moveWorld(delta float64,
	dCount *int,
	dX *dotsX,
	dY *dotsY,
	dVelX *dotsVelX,
	dVelY *dotsVelY) {

	for i := 0; i < *dCount; i++ {
		dX[i] += (dVelX[i] * delta)
		dY[i] += (dVelY[i] * delta)
	}
}

func spawnDot(x, y float64,
	mat dotMaterial,
	dCount *int,
	dMat *dotsMaterial,
	dX *dotsX,
	dY *dotsY) {

	dMat[*dCount] = mat
	dX[*dCount] = x
	dY[*dCount] = y
	*dCount += 1
}

func main() {
	var (
		dCount int
		dMat   dotsMaterial = newDotsMaterial()
		dX     dotsX
		dY     dotsY
		dVelX  dotsVelX
		dVelY  dotsVelY

		delta float64
		lastTick time.Time
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

	lastTick = time.Now()

	// TODO: remove manual tomfoolery
	spawnDot(79.999, 1.0, dotSand, &dCount, &dMat, &dX, &dY)
	dVelX[0] = 0.0
	dVelY[0] = 5.0

	mainloop:
	for {
		nanoDelta := time.Since(lastTick)
		if nanoDelta >= (1_000_000_000 / tickrate) {
			delta = float64(nanoDelta) / float64(1_000_000_000)
			drawWorld(&dCount, &dMat, &dX, &dY, surface, window)
			moveWorld(delta, &dCount, &dX, &dY, &dVelX, &dVelY)

			fmt.Printf("f/s %v\n", int(1.0 / delta))

			event := sdl.PollEvent()

			for ; event != nil; event = sdl.PollEvent() {
				switch event.(type) {
				case *sdl.QuitEvent:
					break mainloop
				}
			}

			lastTick = time.Now()
		}
	}
}
