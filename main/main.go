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
	pngSize       = 16
	uiBgR         = 130
	uiBgG         = 170
	uiBgB         = 170
	uiBgA         = 255
	uiTileSetW    = 3
	stdTickrate   = 24
	stdWinW       = 640
	stdWinH       = 480
)

type physGame struct {
	FrameW     int
	FrameH     int
	LayoutWide bool
	pause      bool
	Toolbox    ui.TileSet
	Matbox     ui.TileSet
}

func (g physGame) Draw(
	screen *ebiten.Image,
) {
	var (
		opt = ebiten.DrawImageOptions{}
		tbW, tbH int
	)

	tbW, tbH = g.Toolbox.Size()

	g.Toolbox.Draw()
	g.Matbox.Draw()

	if !g.LayoutWide {
		opt.GeoM.Translate(0, float64(g.FrameH - tbH))
	}

	screen.DrawImage(g.Toolbox.Img, &opt)

	if g.LayoutWide {
		opt.GeoM.Translate(0, float64(tbH))
	} else {
		opt.GeoM.Translate(float64(tbW), 0)
	}

	screen.DrawImage(g.Matbox.Img, &opt)
}

func (g physGame) Layout(
	outsideWidth int,
	outsideHeight int,
) (int, int) {
	return g.FrameW, g.FrameH
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
			           stdWinW)
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
		g        physGame
		tickrate int = stdTickrate
		winW     int = stdWinW
		winH     int = stdWinH
		uiW      int
		uiH      int
	)

	fmt.Printf("%v\n", mat.Hydrogen) // TODO actually use mat

	ebiten.SetFullscreen(true);

	if handleArgs(&tickrate, &winW, &winH) == false {
		return
	}

	if ebiten.IsFullscreen() {
		screenW, screenH := ebiten.Monitor().Size()
		g.FrameW = screenW / 10
		g.FrameH = screenH / 10
	} else {
		g.FrameW = winW / 4
		g.FrameH = winH / 4
	}

	paths := [...]string{
		"assets/tool_brush.png",
		"assets/tool_spawner.png",
		"assets/tool_eraser.png",
	}

	if g.FrameW >= g.FrameH {
		g.LayoutWide = true
		uiW = uiTileSetW * pngSize
		uiH = pngSize
	} else {
		g.LayoutWide = false
		uiW = pngSize
		uiH = uiTileSetW * pngSize
	}

	g.Toolbox = ui.NewTileSetFromFS(g.LayoutWide,
	                                uiTileSetW,
	                                uiW,
	                                uiH,
	                                paths[:],
	                                pngs)
	g.Toolbox.Bg = color.RGBA{uiBgR, uiBgG, uiBgB, uiBgA}
	g.Toolbox.VisibleTiles = g.Toolbox.Tiles[:]

	g.Matbox = ui.NewTileSetFromFS(g.LayoutWide,
	                               uiTileSetW,
	                               uiW,
	                               uiH,
	                               paths[:],
	                               pngs) // TODO make the mat pngs
	g.Matbox.Bg = color.RGBA{uiBgR, uiBgG, uiBgB, uiBgA}
	g.Matbox.VisibleTiles = g.Toolbox.Tiles[:]

	ebiten.SetWindowTitle(AppName + " " + AppVersion)
	ebiten.SetWindowSize(winW, winH)
	ebiten.SetWindowResizingMode(ebiten.WindowResizingModeDisabled)
	ebiten.SetTPS(tickrate)

	ebiten.RunGame(g)
}
