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

func dropGrain(
	dots *world,
	x int,
	y int,
) {
	var (
		below  *mats.Mat
		belowL *mats.Mat
		belowR *mats.Mat
		cur    *mats.Mat
		tmp     mats.Mat
	)

	cur = &dots[x][y]
	below = &dots[x][y + 1]

	if mats.None == *below ||
	   (mats.Weight(*below) < mats.Weight(*cur) &&
	    mats.Structure(*below) == mats.MsLiquid) {
		tmp = *below
		*below = *cur
		*cur = tmp
		return
	}

	if x - 1 >= 0 {
		belowL = &dots[x - 1][y + 1]

		if mats.None == *belowL {
			*belowL = *cur
			*cur = mats.None
			return
		}
	}
	if x + 1 < worldWidth {
		belowR = &dots[x + 1][y + 1]

		if mats.None == *belowR {
			*belowR = *cur
			*cur = mats.None
			return
		}
	}
}

func dropLiquid(
	dots *world,
	x int,
	y int,
) {
	var (
		below *mats.Mat
		cur   *mats.Mat
	)

	cur = &dots[x][y]
	below = &dots[x][y + 1]

	if mats.None == *below {
		*below = *cur
		*cur = mats.None
		return
	}

	if mats.Structure(*below) != mats.MsLiquid {
		return
	}

	for cX := x - 1; cX >= 0; cX-- {
		if mats.Structure(dots[cX][y - 1]) == mats.MsLiquid {
			dots[cX][y - 1] = *below
			*below = *cur
			*cur = mats.None
			return
		}
	}
	for cX := x + 1; cX < worldWidth; cX++ {
		if mats.Structure(dots[cX][y - 1]) == mats.MsLiquid {
			dots[cX][y - 1] = *below
			*below = *cur
			*cur = mats.None
			return
		}
	}
}

func applyGravity(
	dots *world,
) {
	for x := 0; x < worldWidth; x++ {
		for y := worldHeight - 2; y >= 0; y-- {
			switch mats.Structure(dots[x][y]) {
			case mats.MsStatic:
				break

			case mats.MsGrain:
				dropGrain(dots, x, y)
				break

			case mats.MsLiquid:
				dropLiquid(dots, x, y)
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
	spawn1X := worldWidth / 3
	spawn1Y := worldHeight / 3 * 2
	spawn1W := 3
	spawn1H := 3
	spawn2X := worldWidth / 3 * 2
	spawn2Y := worldHeight / 3
	spawn2W := 5
	spawn2H := 5
	for x := spawn1X; x < spawn1X + spawn1W; x++ {
		for y := spawn1Y; y < spawn1Y + spawn1H; y++ {
			dots[x][y] = mats.Sand
		}
	}
	for x := spawn2X; x < spawn2X + spawn2W; x++ {
		for y := spawn2Y; y < spawn2Y + spawn2H; y++ {
			dots[x][y] = mats.Water
		}
	}

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
