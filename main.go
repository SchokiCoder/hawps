// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package main

import (
	"fmt"
	"github.com/veandco/go-sdl2/sdl"
	"math"
	"os"
	"time"
)

const (
	appName       = "hawps"
	appNameFormal = "Half Assed Wannabe Physics Simulator"
	appVersion    = "v0.1"
	appLicense    = "GPL-2.0-or-later"
	appRepository = "https://github.com/SchokiCoder/hawps"
	appLicenseUrl = "https://www.gnu.org/licenses/gpl-2.0.html"
)

const (
	earthGravity = 980
	moonGravity  = 162
)

const (
	dotSize        = 1.0
	gfxScale       = 10
	gravity        = 0
	groundFriction = 0.5
	tickrate       = 60
	timescale      = 0.1
	worldWidth     = 80.0
	worldHeight    = 60.0
)

const (
	dotNone = iota
	dotSand
	dotWater
)

func matFriction(index dotMaterial) float64 {
	var protectedArray = [...]float64{
		0.0,
		0.7,
		0.3,
	}

	return protectedArray[index]
}

func matWeight(index dotMaterial) float64 {
	var protectedArray = [...]float64{
		0.0,
		1.5,
		1.0,
	}

	return protectedArray[index]
}

func matR(index dotMaterial) uint8 {
	var protectedArray = [...]uint8{
		255,
		238,
		0,
	}

	return protectedArray[index]
}

func matG(index dotMaterial) uint8 {
	var protectedArray = [...]uint8{
		0,
		217,
		253,
	}

	return protectedArray[index]
}

func matB(index dotMaterial) uint8 {
	var protectedArray = [...]uint8{
		255,
		86,
		255,
	}

	return protectedArray[index]
}

type (
	dotMaterial  int
)

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

func drawWorld(
	dCount  *int,
	dMat    []dotMaterial,
	dX      []float64,
	dY      []float64,
	surface *sdl.Surface,
	window  *sdl.Window,
) {
	surface.FillRect(nil, 0)

	for i := 0; i < *dCount; i++ {
		rect := sdl.Rect{
			X: int32(int64(dX[i]) * gfxScale),
			Y: int32(int64(dY[i]) * gfxScale),
			W: int32(dotSize * gfxScale),
			H: int32(dotSize * gfxScale),
		}

		pixel := sdl.MapRGB(surface.Format,
			matR(dMat[i]), matG(dMat[i]), matB(dMat[i]))
		surface.FillRect(&rect, pixel)
		window.UpdateSurface()
	}
}

func handleArgs() bool {
	for i := 1; i < len(os.Args); i++ {
		switch os.Args[i] {
		case "-a":
			fallthrough
		case "--about":
			fmt.Printf("The source code of \"%v\" aka %v %v is available, "+
				"licensed under the %v at:\n"+
				"%v\n\n"+
				"If you did not receive a copy of the license, "+
				"see below:\n"+
				"%v\n",
				appNameFormal, appName, appVersion,
				appLicense,
				appRepository,
				appLicenseUrl)
			return false

		default:
			fmt.Fprintf(os.Stderr,
				"Argument \"%v\" is not recognized.\n",
				os.Args[i])
		}
	}

	return true
}

func handleDotBoundsCollision(x, y, velX, velY *float64, grounded *bool) {
	if *x < 0.0 {
		*x = 0.0
		*velX = 0.0
	} else if *x + dotSize >= worldWidth {
		*x = worldWidth - dotSize
		*velX = 0.0
	}

	if *y < 0.0 {
		*y = 0.0
		*velY = 0.0
	} else if *y + dotSize >= worldHeight {
		*y = worldHeight - dotSize
		*velY = 0.0
		*grounded = true
	}
}

// Changes velocity on collision.
// Dots are considered to be circles for collision detection.
// Returns true on collision.
func handleDotDotCollision(
	dX       []float64,
	dY       []float64,
	dVelX    []float64,
	dVelY    []float64,
	dWeight  []float64,
	i        int,
	j        int,
) bool {
	var (
		deltaX, deltaY   float64
		distance         float64
		forceIX, forceIY float64
		forceJX, forceJY float64
	)

	deltaX = dX[i] - dX[j]
	deltaY = dY[i] - dY[j]
	distance = math.Sqrt(deltaX * deltaX + deltaY * deltaY)

	if (distance < dotSize) {
		forceIX = dVelX[i] * dWeight[i]
		forceIY = dVelY[i] * dWeight[i]
		forceJX = dVelX[j] * dWeight[j]
		forceJY = dVelY[j] * dWeight[j]

		dVelX[i] -= (forceIX + forceJX) / dWeight[i]
		dVelY[i] -= (forceIY + forceJY) / dWeight[i]
		dVelX[j] += (forceIX + forceJX) / dWeight[i]
		dVelY[j] += (forceIY + forceJY) / dWeight[i]

		return true
	}

	return false
}

func handleEvents(active *bool) {
	event := sdl.PollEvent()

	for ; event != nil; event = sdl.PollEvent() {
		switch event.(type) {
		case *sdl.KeyboardEvent:
			event := event.(*sdl.KeyboardEvent)
			if event.GetType() == sdl.KEYUP {
				switch event.Keysym.Sym {
				case sdl.K_ESCAPE:
					*active = false
				}
			}

		case *sdl.QuitEvent:
			*active = false
		}
	}
}

// You have found the source of evil. Good.
func moveDot(
	i         int,
	delta     float64,
	dCount    int,
	dX        []float64,
	dY        []float64,
	dVelX     []float64,
	dVelY     []float64,
	dGrounded []bool,
	dWeight   []float64,
) {
	var (
		alreadyCollided [64]int
		collisions      int
		deltaX          float64
		deltaY          float64
		newX            float64
		newY            float64
		steps           int
		stepX           float64
		stepY           float64
		xNegative       bool
		yNegative       bool
	)

	collCheckRange := func(start, end int) {
		jloop:
		for j := start; j < end; j++ {
			for c := 0; c < collisions; c++ {
				if j == alreadyCollided[c] {
					continue jloop
				}
			}

			coll := handleDotDotCollision(dX, dY,
				dVelX, dVelY,
				dWeight,
				i, j)
			if coll {
				alreadyCollided[collisions] = j
				collisions++
			}
		}
	}

	newX = dX[i] + (dVelX[i] * delta)
	newY = dY[i] + (dVelY[i] * delta)

	deltaX = newX - dX[i]
	deltaY = newY - dY[i]

	if deltaX < 1.0 && deltaY < 1.0 {
		dX[i] = newX
		dY[i] = newY

		collCheckRange(0, i)
		collCheckRange(i + 1, dCount)

		handleDotBoundsCollision(&dX[i], &dY[i],
			&dVelX[i], &dVelY[i],
			&dGrounded[i])
		return
	}

	if deltaX < 0.0 {
		xNegative = true
		deltaX *= -1.0
	}
	if deltaY < 0.0{
		yNegative = true
		deltaY *= -1.0
	}

	if deltaX > deltaY {
		steps = int(deltaX)
		stepX = 1.0
		stepY = deltaY / deltaX
	} else {
		steps = int(deltaY)
		stepX = deltaX / deltaY
		stepY = 1.0
	}

	if xNegative {
		stepX *= -1.0
	}
	if yNegative {
		stepY *= -1.0
	}

	for count := 0; count < steps; count++ {
		dX[i] += stepX
		dY[i] += stepY

		collCheckRange(0, i)
		collCheckRange(i + 1, dCount)

		handleDotBoundsCollision(&dX[i], &dY[i],
			&dVelX[i], &dVelY[i],
			&dGrounded[i])
	}
}

// Moves every dot.
// A dot is 1 cm big (W and H).
// Velocity is cm/s.
func moveWorld(
	delta     float64,
	dCount    int,
	dX        []float64,
	dY        []float64,
	dVelX     []float64,
	dVelY     []float64,
	dGrounded []bool,
	dFric     []float64,
	dWeight   []float64,
) {
	for i := 0; i < dCount; i++ {
		// gravity
		dVelY[i] += (gravity * delta)

		moveDot(i, delta,
			dCount, dX, dY, dVelX, dVelY, dGrounded, dWeight)

		if dGrounded[i] && dVelX[i] != 0.0 {
			applyDotGroundFriction(&dVelX[i],
				delta, dFric[i], dWeight[i])
		}
	}
}

func spawnDot(
	x       float64,
	y       float64,
	mat     dotMaterial,
	dCount  *int,
	dMat    []dotMaterial,
	dX      []float64,
	dY      []float64,
	dFric   []float64,
	dWeight []float64,
) {
	dMat[*dCount] = mat
	dX[*dCount] = x
	dY[*dCount] = y
	dFric[*dCount] = matFriction(mat)
	dWeight[*dCount] = matWeight(mat)
	*dCount += 1
}

func main() {
	var (
		active    bool
		dCount    int
		dMat      [worldWidth * worldHeight]dotMaterial
		dX        [worldWidth * worldHeight]float64
		dY        [worldWidth * worldHeight]float64
		dVelX     [worldWidth * worldHeight]float64
		dVelY     [worldWidth * worldHeight]float64
		dGrounded [worldWidth * worldHeight]bool
		dFric     [worldWidth * worldHeight]float64
		dWeight   [worldWidth * worldHeight]float64

		delta float64
		lastTick time.Time
	)

	active = true

	if handleArgs() == false {
		return
	}

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
	spawnDot(worldWidth / 3.0 + 1.0, worldHeight / 3.0, dotSand, &dCount, dMat[:], dX[:], dY[:], dFric[:], dWeight[:])
	dVelX[0] = 100.0
	dVelY[0] = 100.0
	spawnDot(worldWidth / 3.0 * 2.0, worldHeight / 2.0 + 2.0, dotWater, &dCount, dMat[:], dX[:], dY[:], dFric[:], dWeight[:])
	dVelX[1] = -100.0

	for active {
		rawDelta := time.Since(lastTick)
		if rawDelta >= (1_000_000_000 / tickrate) {
			delta = float64(rawDelta) / float64(1_000_000_000)
			delta *= timescale
			drawWorld(&dCount,
				dMat[:],
				dX[:],
				dY[:],
				surface,
				window)
			moveWorld(delta,
				dCount,
				dX[:],
				dY[:],
				dVelX[:],
				dVelY[:],
				dGrounded[:],
				dFric[:],
				dWeight[:])

			handleEvents(&active)

			lastTick = time.Now()
		}
	}
}
