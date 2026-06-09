// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2026  Andy Frank Schoknecht

//go:generate stringer -type=tool -linecomment
package main

import (
	"fmt"
	"os"
	"strconv"
	"time"

	"golang.org/x/term"

	"github.com/SchokiCoder/hawps/core/mat"
	"github.com/SchokiCoder/hawps/core"
	//"github.com/SchokiCoder/hawps/extra"
	"github.com/SchokiCoder/hawps/client_shell/csi"
)

var (
	AppLicense    string
	AppLicenseUrl string
	AppName       string
	AppNameFormal string
	AppRepository string
	AppVersion    string
)

const (
	celsiusToKelvin = 273.15

	stdBrushRadius  = 2
	stdEraserRadius = 5
	stdThermoRadius = stdBrushRadius
	maxRadius = 16
	thermalVisionMinT  = -75 + celsiusToKelvin

	stdTickrate     = 120
	stdSimSubsample = 4
	heaterDelta     = 1

	firstRealMat   = mat.Sand

	spawnerR       = 255
	spawnerG       = 0
	spawnerB       = 255

	stdTemperature = 20 + celsiusToKelvin

	toolHoverR     = 175
	toolHoverG     = 255
	toolHoverB     = 175
)

type dotStateLook string
const (
	Solid dotStateLook = "X"
	Liquid             = "+"
	Gas                = "-"
)

type tool int
const (
	brush tool = iota // BRUSH
	spawner           // SPAWNER
	eraser            // ERASER
	heater            // HEATER
	cooler            // COOLER
)

const appAbout = `The source code of "%v" aka %v %v is available,
licensed under the %v at:
%v

If you did not receive a copy of the license, see below:
%v
`

const appHelp = `Usage: %v [OPTIONS]

Silly program to simulate physics in *very* convincing ways.
It'll be great. Trust me.

Options:

    -a -about
        prints program name, version, license and repository information then exits

    -h -help
        prints this message then exits

    -temperature
        sets the temperature of every new dot in Kelvin
        0 °C == %v K
        default: %v

    -tickrate NUMBER
        sets the tickrate (ticks per second),
        which also effects simulation speed
        only use when otherwise performance problems occur
        default: %v

    -v -version
        prints version information then exits

Default keybinds:

    /
        enter the command line

    Escape
        quit the program

    Space
        pause world

    H J K L
        move cursor left, down, up, and right, respectively

    Plus and Minus
        increase and decrease the simulation speed respectively
        default: %.2f updates per second

    T
        toggle thermal vision (grayscale displaying %v to %v degree Celsius)
`

func draw(
	brushMat mat.Mat,
	cmdmode bool,
	cursorX int,
	cursorY int,
	ipAddress string,
	selTool tool,
	spawnerMat mat.Mat,
	thVision bool,
	winW *int,
	winH *int,
	world core.World,
	worldName string,
	worldW int,
	worldH int,
) {
	var (
		dot dotStateLook
		err error
		leftStBar string
		rightStBar string
		stBar string
		vision string
		worldDrawW int
		worldDrawH int
		x, y int
	)

	fmt.Print(csi.Clear)

	*winW, *winH, err = term.GetSize(int(os.Stdin.Fd()))
	if err != nil {
		panic(err)
	}

	if worldW > *winW {
		worldDrawW = *winW 
	} else {
		worldDrawW = worldW
	}
	if worldH > *winH - 2 {
		worldDrawH = *winH - 2
	} else {
		worldDrawH = worldH
	}

	for y = 0; y < worldDrawH; y++ {
		for x = 0; x < worldDrawW; x++ {
			if world.Dot[x][y] == mat.None {
				fmt.Print(" ")
				continue
			}

			switch world.State[x][y] {
			case mat.Static: fallthrough
			case mat.Grain:
				dot = Solid

			case mat.Liquid:
				dot = Liquid

			case mat.Gas:
				dot = Gas
			}
			fmt.Print(dot)
		}
		fmt.Printf("\n")
	}

	if thVision {
		vision = "Thermal"
	} else {
		vision = "Normal"
	}

	leftStBar = fmt.Sprintf(
		"%v (%v,%v) | View:%v | %v",
		worldName, cursorX, cursorY, vision, ipAddress)

	rightStBar = fmt.Sprintf(
		"%v: bindings, %v: help",
		"bind1", "bind2")

	stBar = fmt.Sprintf(
		"%%v%%%vv",
		*winW - len(leftStBar))

	fmt.Printf(stBar,
		leftStBar,
		rightStBar)

	if cmdmode {
		fmt.Print(":cmd input currently not implemented")
	} else {
		fmt.Print(selTool.String())
		fmt.Print(" ")
		switch selTool {
		case brush:
			fmt.Print(brushMat.String())
		case spawner:
			fmt.Print(spawnerMat.String())
		default:
		}
	}
}

func handleArgs(
	temperature *float64,
	tickrate    *int64,
) bool {
	argToInt := func(i int) int {
		if len(os.Args) <= i + 1 {
			panic("The argument \"" +
				os.Args[i] +
				"\" needs to be followed by a value");
		}
		i++
		n, err := strconv.Atoi(os.Args[i])
		if err != nil {
			panic("\"" +
				os.Args[i - 1] +
				"\" could not be converted to a int");
		}
		return n
	}

	for i := 1; i < len(os.Args); i++ {
		switch os.Args[i] {
		case "-a": fallthrough
		case "-about":
			fmt.Printf(appAbout,
			           AppNameFormal, AppName, AppVersion,
			           AppLicense,
			           AppRepository,
			           AppLicenseUrl)
			return false

		case "-h": fallthrough
		case "-help":
			fmt.Printf(appHelp,
			           AppName,
			           celsiusToKelvin,
			           stdTemperature,
			           stdTickrate,
			           float64(stdTickrate) / float64(stdSimSubsample),
			           thermalVisionMinT - celsiusToKelvin,
			           thermalVisionMinT - celsiusToKelvin + 255)
			return false

		case "-temperature":
			*temperature = float64(argToInt(i))
			if *temperature < 0 {
				panic("The value for \"" +
					os.Args[i] +
					"\" must not be negative")
			}
			i++

		case "-tickrate":
			*tickrate = int64(argToInt(i))
			i++

		case "-v": fallthrough
		case "-version":
			fmt.Printf("%v: version %v\n", AppName, AppVersion)
			return false

		default:
			panic(`Argument "` + os.Args[i] + `" is not recognized`)
		}
	}

	return true
}

func main(
) {
	var (
		active       bool = true
		brushMat     mat.Mat = firstRealMat
		//brushRadius  int = stdBrushRadius
		cmdmode      bool = false
		cursorX      int
		cursorY      int
		//eraserRadius int = stdEraserRadius
		err error
		//paused       bool = false
		selTool      tool
		//simSubsample int = stdSimSubsample
		spawnerMat   mat.Mat = firstRealMat
		temperature  float64 = stdTemperature
		tick         time.Time
		tickrate     int64 = stdTickrate
		//thermoRadius int = stdThermoRadius
		thVision     bool = false
		//tsSinceSim   int = 9001 // ticks since last simulation
		winW         int
		winH         int
		world        core.World
		worldW       int
		worldH       int
	)

	if handleArgs(
		&temperature,
		&tickrate,
	) == false {
		return
	}

	winW, winH, err = term.GetSize(int(os.Stdin.Fd()))
	if err != nil {
		panic(err)
	}

	worldW, worldH = winW, winH - 2
	world = core.NewWorld(worldW, worldH, temperature)

	for active {
		if time.Since(tick).Nanoseconds() > (1_000_000 / tickrate) {
			tick = time.Now()

			draw(
				brushMat,
				cmdmode,
				cursorX, cursorY,
				"localhost",
				selTool,
				spawnerMat,
				thVision,
				&winW, &winH,
				world, "worldname", worldW, worldH)

			active = false
		}
	}
}
