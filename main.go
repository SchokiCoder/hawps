// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package main

import (
	"fmt"
	"github.com/veandco/go-sdl2/sdl"
	"os"
	"time"
)

type (
	dotMaterial  int
	world [worldWidth][worldHeight]dotMaterial
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
	gfxScale       = 10
	tickrate       = 24
	worldWidth     = 80
	worldHeight    = 60
)

const (
	dotNone = iota
	dotSand
	dotWater
)

func matR(index dotMaterial) uint8 {
	var protectedArray = [...]uint8{
		0,
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
		0,
		86,
		255,
	}

	return protectedArray[index]
}

func applyGravity(
	dots *world,
) {
	for x := 0; x < worldWidth; x++ {
		for y := worldHeight - 2; y >= 0; y-- {
			switch dots[x][y] {
			case dotSand:
				fallthrough
			case dotWater:
				if y >= worldHeight {
					continue
				}
				if y + 1 >= worldHeight ||
				   dots[x][y + 1] != dotNone {
					continue
				}
				dots[x][y +1] = dots[x][y]
				dots[x][y] = dotNone
				break
			}
		}
	}
}

func drawWorld(
	dots   *world,
	frame  *sdl.Surface,
	window *sdl.Window,
) {
	for x := 0; x < worldWidth; x++ {
		for y := 0; y < worldHeight; y++ {
			rect := sdl.Rect{
				X: int32(x),
				Y: int32(y),
				W: 1,
				H: 1,
			}

			pixel := sdl.MapRGB(frame.Format,
				            matR(dots[x][y]),
				            matG(dots[x][y]),
				            matB(dots[x][y]))
			frame.FillRect(&rect, pixel)
		}
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

func main() {
	var (
		active   bool
		dots     world
		frame    *sdl.Surface
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
		x := i * 2
		y := int(worldHeight / 3.0)
		dots[x][y] = dotSand
	}
	spawn2 := 10
	for i := 0; i < spawn2; i++ {
		x := int(worldWidth / 2.0)
		y := int(worldHeight - 1.0 - float64(i))
		dots[x][y] = dotSand
	}
	x := int(worldWidth) / 3 * 2
	y := int(worldHeight / 2 + 2)
	dots[x][y] = dotWater

	for active {
		delta := time.Since(lastTick)
		if delta >= (1_000_000_000 / tickrate) {
			drawWorld(&dots, frame, window)
			applyGravity(&dots)

			handleEvents(&active)

			lastTick = time.Now()
		}
	}
}
