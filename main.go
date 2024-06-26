// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package main

import (
	"fmt"
	"github.com/veandco/go-sdl2/sdl"
	"time"
)

const (
	gfxScale       = 10
	gravity        = 980
	groundFriction = 0.5
	tickrate       = 60
	timescale      = 0.1
	worldWidth     = 80.0
	worldHeight    = 60.0
)

const (
	dotNone = iota
	dotSand
)

func matFriction(index dotMaterial) float64 {
	var protectedArray = [...]float64{
		0.0,
		0.7,
	}

	return protectedArray[index]
}

func matWeight(index dotMaterial) float64 {
	var protectedArray = [...]float64{
		0.0,
		1.5,
	}

	return protectedArray[index]
}

type (
	dotMaterial  int
	dotsMaterial [worldWidth * worldHeight]dotMaterial
	dotsX        [worldWidth * worldHeight]float64
	dotsY        [worldWidth * worldHeight]float64
	dotsVelX     [worldWidth * worldHeight]float64
	dotsVelY     [worldWidth * worldHeight]float64
	dotsGrounded [worldWidth * worldHeight]bool
	dotsFriction [worldWidth * worldHeight]float64
	dotsWeight   [worldWidth * worldHeight]float64
)

func newDotsMaterial() dotsMaterial {
	var ret dotsMaterial
	
	for i := 0; i < len(ret); i++ {
		ret[i] = dotNone
	}

	return ret
}

// Friction is the average of ground friction and dot friction,
// and is given as a percentage of velocity loss.
func applyDotGroundFriction(velX *float64, delta, fric, weight float64) {
	var velLoss = gravity * weight * ((groundFriction + fric) / 2.0) * delta

	if *velX > 0.0 {
		if velLoss >= *velX {
			*velX = 0.0
		} else {
			*velX -= velLoss
		}
	} else {
		if velLoss <= *velX {
			*velX = 0.0
		} else {
			*velX += velLoss
		}
	}
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
		pixel := sdl.MapRGB(surface.Format, 238, 217, 86)
		surface.FillRect(&rect, pixel)
		window.UpdateSurface()
	}
}

func handleDotBoundsCollision(x, y, velX, velY *float64, grounded *bool) {
	if *x < 0.0 {
		*x = 0.0
		*velX = 0.0
	} else if *x >= worldWidth {
		*x = worldWidth - 1.0
		*velX = 0.0
	}

	if *y < 0.0 {
		*y = 0.0
		*velY = 0.0
	} else if *y >= worldHeight {
		*y = worldHeight - 1.0
		*velY = 0.0
		*grounded = true
	}
}

func handleDotDotCollision(i int,
	newX, newY float64,
	dX *dotsX,
	dY *dotsY,
	dVelX *dotsVelX,
	dVelY *dotsVelY,
	dWeight *dotsWeight) {

	// TODO: MAAAGIIC
	// (instead of just changing the values flat out (like before))
	dX[i] = newX
	dY[i] = newY
}

func moveDot(i int,
	delta float64,
	dCount int,
	dX *dotsX,
	dY *dotsY,
	dVelX *dotsVelX,
	dVelY *dotsVelY,
	dWeight *dotsWeight) {
	var (
		newX, newY float64
	)

	newX = dX[i] + (dVelX[i] * delta)
	newY = dY[i] + (dVelY[i] * delta)

	// check for collisions with other dots
	for j := 0; j < i; j++ {
		handleDotDotCollision(i, newX, newY,
			dX, dY, dVelX, dVelY, dWeight)
	}
	for j := i + 1; j < dCount; j++ {
		handleDotDotCollision(i, newX, newY,
			dX, dY, dVelX, dVelY, dWeight)
	}
}

// Moves every dot.
// A dot is 1 cm big (W and H).
// Velocity is cm/s.
func moveWorld(delta float64,
	dCount int,
	dX *dotsX,
	dY *dotsY,
	dVelX *dotsVelX,
	dVelY *dotsVelY,
	dGrounded *dotsGrounded,
	dFric *dotsFriction,
	dWeight *dotsWeight) {

	for i := 0; i < dCount; i++ {
		// gravity
		dVelY[i] += (gravity * delta)

		moveDot(i, delta, dCount, dX, dY, dVelX, dVelY, dWeight)

		handleDotBoundsCollision(&dX[i], &dY[i],
			&dVelX[i], &dVelY[i],
			&dGrounded[i])

		if dGrounded[i] && dVelX[i] != 0.0 {
			applyDotGroundFriction(&dVelX[i],
				delta, dFric[i], dWeight[i])
		}
	}
}

func spawnDot(x, y float64,
	mat dotMaterial,
	dCount *int,
	dMat *dotsMaterial,
	dX *dotsX,
	dY *dotsY,
	dFric *dotsFriction,
	dWeight *dotsWeight) {

	dMat[*dCount] = mat
	dX[*dCount] = x
	dY[*dCount] = y
	dFric[*dCount] = matFriction(mat)
	dWeight[*dCount] = matWeight(mat)
	*dCount += 1
}

func main() {
	var (
		dCount    int
		dMat      dotsMaterial = newDotsMaterial()
		dX        dotsX
		dY        dotsY
		dVelX     dotsVelX
		dVelY     dotsVelY
		dGrounded dotsGrounded
		dFric     dotsFriction
		dWeight   dotsWeight

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
	spawnDot(1.0, worldHeight / 2, dotSand, &dCount, &dMat, &dX, &dY, &dFric, &dWeight)
	dVelX[0] = 250.0
	dVelY[0] = 200.0
	spawnDot(worldWidth / 3.0 * 2.0, worldHeight - 2.0, dotSand, &dCount, &dMat, &dX, &dY, &dFric, &dWeight)

	mainloop:
	for {
		rawDelta := time.Since(lastTick)
		if rawDelta >= (1_000_000_000 / tickrate) {
			delta = float64(rawDelta) / float64(1_000_000_000)
			delta *= timescale
			drawWorld(&dCount, &dMat, &dX, &dY, surface, window)
			moveWorld(delta,
				dCount,
				&dX, &dY,
				&dVelX, &dVelY,
				&dGrounded, &dFric, &dWeight)

			fmt.Printf("delta %v\n", delta)

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
