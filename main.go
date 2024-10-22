// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package main

import (
	"fmt"
	"github.com/veandco/go-sdl2/sdl"
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
	gravity        = earthGravity
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
	frame *sdl.Surface,
	window  *sdl.Window,
) {
	frame.FillRect(nil, 0)

	for i := 0; i < *dCount; i++ {
		rect := sdl.Rect{
			X: int32(int64(dX[i])),
			Y: int32(int64(dY[i])),
			W: dotSize,
			H: dotSize,
		}

		pixel := sdl.MapRGB(frame.Format,
			matR(dMat[i]), matG(dMat[i]), matB(dMat[i]))
		frame.FillRect(&rect, pixel)
	}

	ws, err := window.GetSurface()
	if err != nil {
		panic(err)
	}

	frame.BlitScaled(&frame.ClipRect, ws, &ws.ClipRect)
	window.UpdateSurface()
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

func pointInRect(
	pX, pY, rX, rY, rW, rH float64,
) bool {
	if pX > rX && pX < (rX + rW) &&
	   pY > rY && pY < (rY + rH) {
		return true
	}

	return false
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
	newX     float64,
	newY     float64,
) {
	var (
		iAX, iBX, iCX, iDX float64
		iAY, iBY, iCY, iDY float64
		jRX, jRY, jRW, jRH float64
	)

	iAX = newX - (dotSize / 2.0)
	iAY = newY - (dotSize / 2.0)
	iBX = newX + (dotSize / 2.0)
	iBY = newY - (dotSize / 2.0)
	iCX = newX - (dotSize / 2.0)
	iCY = newY + (dotSize / 2.0)
	iDX = newX + (dotSize / 2.0)
	iDY = newY + (dotSize / 2.0)

	jRX = dX[j] - (dotSize / 2.0)
	jRY = dX[j] - (dotSize / 2.0)
	jRW = dotSize
	jRH = dotSize

	if pointInRect(iAX, iAY, jRX, jRY, jRW, jRH) ||
	   pointInRect(iBX, iBY, jRX, jRY, jRW, jRH) ||
	   pointInRect(iCX, iCY, jRX, jRY, jRW, jRH) ||
	   pointInRect(iDX, iDY, jRX, jRY, jRW, jRH) {
		fmt.Printf("coll\n")
	}

	/* circle v circle, radius check method, force transmission probably faulty
	var (
		a2, b2 float64
		hypot2 float64
		distance float64
		forceIX, forceIY float64
		forceJX, forceJY float64
	)

	a2 = (newX + (dotSize / 2.0)) - (dX[j] + (dotSize / 2.0))
	if a2 < 0.0 {
		a2 *= -1.0
	}

	b2 = (newY + (dotSize / 2.0)) - (dY[j] + (dotSize / 2.0))
	if b2 < 0.0 {
		b2 *= -1.0
	}

	hypot2 = a2 + b2

	distance = math.Sqrt(hypot2)

	if distance < (dotSize / 2.0) {
		forceIX = dVelX[i] * dWeight[i]
		forceIY = dVelY[i] * dWeight[i]
		forceJX = dVelX[j] * dWeight[j]
		forceJY = dVelY[j] * dWeight[j]

		dVelX[i] -= (forceIX + forceJX) / dWeight[i]
		dVelY[i] -= (forceIY + forceJY) / dWeight[i]
		dVelX[j] += (forceIX + forceJX) / dWeight[i]
		dVelY[j] += (forceIY + forceJY) / dWeight[i]
	}*/
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
		newX            float64
		newY            float64
	)

	collCheckRange := func(start, end int) {
		for j := start; j < end; j++ {
			handleDotDotCollision(dX, dY,
				dVelX, dVelY,
				dWeight,
				i, j,
				newX, newY)
		}
	}

	newX = dX[i] + (dVelX[i] * delta)
	newY = dY[i] + (dVelY[i] * delta)

	collCheckRange(0, i)
	collCheckRange(i + 1, dCount)

	dX[i] = newX
	dY[i] = newY

	handleDotBoundsCollision(&dX[i], &dY[i],
		&dVelX[i], &dVelY[i],
		&dGrounded[i])
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
		frame *sdl.Surface
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

	window, err := sdl.CreateWindow(appNameFormal,
		sdl.WINDOWPOS_UNDEFINED,
		sdl.WINDOWPOS_UNDEFINED,
		worldWidth * gfxScale,
		worldHeight * gfxScale,
		sdl.WINDOW_SHOWN)
	if err != nil {
		panic(err)
	}
	defer window.Destroy()

	frame, err = sdl.CreateRGBSurface(0, worldWidth, worldHeight, 32,
	                                  0, 0, 0, 0)
	if err != nil {
		panic(err)
	}

	lastTick = time.Now()

	// TODO: remove manual tomfoolery
	spawnSand := 20
	for i := 0; i < spawnSand; i++ {
		spawnDot(float64(i * 2), worldHeight / 3.0, dotSand, &dCount, dMat[:], dX[:], dY[:], dFric[:], dWeight[:])
		dVelX[i] = 100.0
		dVelY[i] = 200.0
	}
	spawn2 := 10
	for i := 0; i < spawn2; i++ {
		spawnDot(worldWidth / 2.0, worldHeight - 1.0 - float64(i), dotSand, &dCount, dMat[:], dX[:], dY[:], dFric[:], dWeight[:])
	}
	spawnDot(worldWidth / 3.0 * 2.0, worldHeight / 2.0 + 2.0, dotWater, &dCount, dMat[:], dX[:], dY[:], dFric[:], dWeight[:])
	dVelX[dCount -1] = -100.0

	for active {
		rawDelta := time.Since(lastTick)
		if rawDelta >= (1_000_000_000 / tickrate) {
			delta = float64(rawDelta) / float64(1_000_000_000)
			delta *= timescale
			drawWorld(&dCount,
				dMat[:],
				dX[:],
				dY[:],
				frame,
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
