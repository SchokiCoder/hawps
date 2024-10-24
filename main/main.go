// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package main

import (
	"fmt"
	"os"
	"time"

	"github.com/SchokiCoder/hawps/materals"
	"github.com/veandco/go-sdl2/sdl"
)

type (
	world [worldWidth][worldHeight]mats.Mat
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

func applyGravity(
	dots *world,
) {
	var (
		below *mats.Mat
		cur   *mats.Mat
		tmp    mats.Mat
	)

	for x := 0; x < worldWidth; x++ {
		for y := worldHeight - 2; y >= 0; y-- {
			below = &dots[x][y + 1]
			cur = &dots[x][y]

			if mats.Weight(*below) >= mats.Weight(*cur) {
				continue
			}

			tmp = *below
			*below = *cur
			*cur = tmp
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
				            mats.R(dots[x][y]),
				            mats.G(dots[x][y]),
				            mats.B(dots[x][y]))
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
	defer frame.Free()

	lastTick = time.Now()

	// TODO: remove manual tomfoolery
	spawnSand := 20
	for i := 0; i < spawnSand; i++ {
		x := i * 2
		y := int(worldHeight / 3.0)
		dots[x][y] = mats.Sand
	}
	spawn2 := 10
	for i := 0; i < spawn2; i++ {
		x := int(worldWidth / 2.0)
		y := int(worldHeight - 10.0 - float64(i))
		dots[x][y] = mats.Sand
	}
	x := int(worldWidth) / 2
	y := int(worldHeight - 1)
	dots[x][y] = mats.Water

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
