// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

package main

import (
	"embed"
	"fmt"
	"image/color"
	_ "image/png"
	"strconv"
	"os"

	"github.com/SchokiCoder/hawps/mat"
	"github.com/SchokiCoder/hawps/ui"

	"github.com/hajimehoshi/ebiten/v2"
	"github.com/hajimehoshi/ebiten/v2/inpututil"
)

//go:embed assets/*.png
var pngs embed.FS

var (
	AppLicense    string
	AppLicenseUrl string
	AppName       string
	AppNameFormal string
	AppRepository string
	AppVersion    string
)

const (
	stdTickrate   = 24
	stdWinW       = 640
	stdWinH       = 480
	stdWorldScale = 10
)

type physGame struct {
	pause   bool
	Toolbox ui.TileSet
}

func (g physGame) Draw(
	screen *ebiten.Image,
) {
	var opt = ebiten.DrawImageOptions{}

	g.Toolbox.Draw()

	opt.GeoM.Scale(4, 4)
	screen.DrawImage(g.Toolbox.Img, &opt)
}

func (g physGame) Layout(
	outsideWidth int,
	outsideHeight int,
) (int, int) {
	return stdWinW, stdWinH
}

func (g physGame) Update(
) error {
	var keys []ebiten.Key

	keys = inpututil.AppendPressedKeys(keys)

	for i := 0; i < len(keys); i++ {
		switch (keys[i]) {
		case ebiten.KeyEscape:
			return ebiten.Termination

		case ebiten.KeySpace:
			g.pause = !g.pause
		}
	}

	if (g.pause) {
		return nil
	}

	// TODO world update goes here

	return nil
}

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

    -H -height NUMBER
        sets the window height
        default: %v

    -h -help
        prints this message then exits

    -noborder
        removes window decoration from window

    -tickrate NUMBER
        sets the tickrate (ticks per second), which effects visible speed
        default: %v

    -v -version
        prints version information then exits

    -W -width NUMBER
        sets the window width
        default: %v

    -window -windowed
        starts the app in windowed mode... not fullscreen

    -world_scale NUMBER
        sets the graphical scale of the physical world
        default: %v

Default keybinds:

    ESC
        quit the program

    Space
        pause world
`;

func handleArgs(
	tickrate *int,
	winW     *int,
	winH     *int,
	wldScale *int,
) bool {
	argToInt := func(i int) int {
		if len(os.Args) <= i + 1 {
			panic("The argument \"" + os.Args[i] + "\" needs to be followed by a value");
		}
		i++
		n, err := strconv.Atoi(os.Args[i])
		if err != nil {
			panic("\"" + os.Args[i - 1] + "\" could not be converted to a int");
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

		case "-H": fallthrough
		case "-height":
			*winH = argToInt(i)
			i++

		case "-h": fallthrough
		case "-help":
			fmt.Printf(appHelp,
			           AppName,
			           stdWinH,
			           stdTickrate,
			           stdWinW,
			           stdWorldScale)
			return false

		case "-noborder":
			ebiten.SetWindowDecorated(false)

		case "-tickrate":
			*tickrate = argToInt(i)
			i++

		case "-v": fallthrough
		case "-version":
			fmt.Printf("%v: version %v\n", AppName, AppVersion)

		case "-W": fallthrough
		case "-width":
			*winW = argToInt(i)
			i++

		case "-window": fallthrough
		case "-windowed":
			ebiten.SetFullscreen(false)

		case "-world_scale":
			*wldScale = argToInt(i)
			i++

		default:
			fmt.Printf("Argument \"%v\" is not recognized.\n",
			           os.Args[i])
			return false
		}
	}

	return true
}

func main(
) {
	var (
		game     physGame
		tickrate int = stdTickrate
		winW     int = stdWinW
		winH     int = stdWinH
		wldScale int = stdWorldScale
	)

	fmt.Printf("%v\n", mat.Hydrogen) // TODO actually use mat

	ebiten.SetFullscreen(true);

	if handleArgs(&tickrate, &winW, &winH, &wldScale) == false {
		return
	}

	ebiten.SetWindowTitle(AppName + " " + AppVersion)
	ebiten.SetWindowSize(winW, winH)
	ebiten.SetWindowResizingMode(ebiten.WindowResizingModeDisabled)
	ebiten.SetTPS(tickrate)

	paths := [...]string{
		"assets/tool_brush.png",
		"assets/tool_spawner.png",
		"assets/tool_eraser.png",
	}
	// TODO manual toolbox tomfoolery
	game.Toolbox = ui.NewTileSetFromFS(true, 3, 3 * 16, 1 * 16, paths[:], pngs)
	game.Toolbox.Bg = color.RGBA{130, 170, 170, 255}
	game.Toolbox.VisibleTiles = game.Toolbox.Tiles[:]
	game.Toolbox.Cursor = 0

	ebiten.RunGame(game)
}
