// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

//go:generate stringer -type=tool
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
	"github.com/hajimehoshi/ebiten/v2/ebitenutil"
	"github.com/hajimehoshi/ebiten/v2/inpututil"
)

type tool int
const (
	brush tool = iota
	spawner
	eraser

	tCount int = iota
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
	uiToolBgR     = 80
	uiToolBgG     = 120
	uiToolBgB     = 120
	uiToolBgA     = 255
	uiMatBgR      = 80
	uiMatBgG      = 110
	uiMatBgB      = 130
	uiMatBgA      = 255
	uiTileSetW    = 3
	stdTickrate   = 24
	stdWinW       = 640
	stdWinH       = 480
)

type physGame struct {
	FrameW     *int
	FrameH     *int
	pause      *bool
	Toolbox    *ui.TileSet
	Matbox     *ui.TileSet
	World      *mat.World
	WorldImg   *ebiten.Image
	WorldX     int
	WorldY     int
}

func newPhysGame(
) physGame {
	return physGame{
		FrameW:  new(int),
		FrameH:  new(int),
		pause:   new(bool),
		Toolbox: new(ui.TileSet),
		Matbox:  new(ui.TileSet),
		World:   new(mat.World),
	}
}

func (g physGame) Draw(
	screen *ebiten.Image,
) {
	var (
		opt = ebiten.DrawImageOptions{}
	)

	g.Toolbox.Draw()
	g.Matbox.Draw()

	opt.GeoM.Translate(float64(g.Toolbox.X), float64(g.Toolbox.Y))
	screen.DrawImage(g.Toolbox.Img, &opt)

	opt.GeoM.Reset()
	opt.GeoM.Translate(float64(g.Matbox.X), float64(g.Matbox.Y))
	screen.DrawImage(g.Matbox.Img, &opt)

	// So I was initially trying to use ebiten.Image.WritePixels(),
	// but it literally didn't do anything.
	// Couldn't find an update-like function either.
	// So away it went for Set().
	for x := 0; x < g.World.W; x++ {
		for y := 0; y < g.World.H; y++ {
			g.WorldImg.Set(x, y,
				color.RGBA{
					mat.Rs(g.World.Dots[x][y]),
					mat.Gs(g.World.Dots[x][y]),
					mat.Bs(g.World.Dots[x][y]),
					255})
		}
	}
	opt.GeoM.Reset()
	opt.GeoM.Translate(float64(g.WorldX), float64(g.WorldY))
	screen.DrawImage(g.WorldImg, &opt)
}

func (g physGame) Layout(
	outsideWidth int,
	outsideHeight int,
) (int, int) {
	return *g.FrameW, *g.FrameH
}

func (g physGame) Update(
) error {
	var (
		clicked bool
		keys    []ebiten.Key
	)

	keys = inpututil.AppendPressedKeys(keys)

	for i := 0; i < len(keys); i++ {
		switch (keys[i]) {
		case ebiten.KeyEscape:
			return ebiten.Termination

		case ebiten.KeySpace:
			*g.pause = !*g.pause
		}
	}

	if inpututil.IsMouseButtonJustReleased(ebiten.MouseButtonLeft) {
		mX, mY := ebiten.CursorPosition()

		boxen := [...]*ui.TileSet{ // hah!
			g.Toolbox,
			g.Matbox,
		}
		for i := 0; i < len(boxen); i++ {
			clicked = boxen[i].HandleClick(mX, mY)

			if clicked {
				break
			}
		}

		if !clicked &&
		   mX > g.WorldX && mX < g.WorldX + g.World.W &&
		   mY > g.WorldY && mY < g.WorldY + g.World.H {
			curTool := tool(g.Toolbox.Cursor)

			switch curTool {
			case brush:
				g.World.UseBrush(
					mat.Mat(g.Matbox.Cursor) + 1,
					mX - g.WorldX,
					mY - g.WorldY)

			case eraser:
				g.World.UseBrush(
					mat.None,
					mX - g.WorldX,
					mY - g.WorldY)

			default:
				panic("Used unknown tool " + curTool.String())
			}
		}
	}

	if (*g.pause) {
		return nil
	}

	g.World.Tick()

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

func genMatImages() []*ebiten.Image {
	var (
		bgImgs [mat.MsCount - 1]*ebiten.Image
		matBgPrefix = "assets/matbg_"
		matBgPaths = [mat.MsCount - 1]string{
			"static",
			"grain",
			"liquid",
			"gas",
		}
		matPrefix = "assets/mat_"
		matsBegin = mat.Sand
		matsEnd = mat.Hydrogen
		postfix = ".png"
		ret []*ebiten.Image
	)

	imgopen := func(path string) *ebiten.Image {
		img, _, err := ebitenutil.NewImageFromFileSystem(pngs, path)
		if err != nil {
			panic(err)
		}
		return img
	}

	for i := 0; i < len(bgImgs); i++ {
		path := matBgPrefix + matBgPaths[i] + postfix
		bgImgs[i] = imgopen(path)
	}

	for i := matsBegin; i <= matsEnd; i++ {
		path := matPrefix + i.String() + postfix
		matImg := imgopen(path)
		opt := ebiten.DrawImageOptions{}
		img := ebiten.NewImage(pngSize, pngSize)

		opt.ColorM.ScaleWithColor(color.RGBA{mat.Rs(i), mat.Gs(i), mat.Bs(i), 255})
		img.DrawImage(bgImgs[mat.States(i) - 1], &opt)

		opt.ColorM.Reset()
		opt.ColorM.ScaleWithColor(color.RGBA{255-mat.Rs(i), 255-mat.Gs(i), 255-mat.Bs(i), 255})
		opt.GeoM.Translate(float64(pngSize - matImg.Bounds().Dx()) / 2,
		                   float64(pngSize - matImg.Bounds().Dy() - 1))
		img.DrawImage(matImg, &opt)
		ret = append(ret, img)
	}

	return ret
}

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
		g          physGame = newPhysGame()
		layoutWide bool
		tickrate   int = stdTickrate
		winW       int = stdWinW
		winH       int = stdWinH
		mbW, mbH   int
		tbW, tbH   int
		wW, wH     int
	)

	ebiten.SetFullscreen(true);

	if handleArgs(&tickrate, &winW, &winH) == false {
		return
	}

	if ebiten.IsFullscreen() {
		screenW, screenH := ebiten.Monitor().Size()
		*g.FrameW = screenW / 10
		*g.FrameH = screenH / 10
	} else {
		*g.FrameW = winW / 4
		*g.FrameH = winH / 4
	}

	tPaths := make([]string, tCount)
	for i := 0; i < tCount; i++ {
		tPaths[i] = "assets/tool_" + tool(i).String() + ".png"
	}

	if *g.FrameW >= *g.FrameH {
		layoutWide = true
		tbW = uiTileSetW * pngSize
		tbH = pngSize
		mbW = tbW
		mbH = *g.FrameH - tbH
		wW = *g.FrameW - tbW
		wH = *g.FrameH
		g.WorldX = tbW
		g.WorldY = 0
	} else {
		layoutWide = false
		tbW = pngSize
		tbH = uiTileSetW * pngSize
		mbW = *g.FrameW - tbW
		mbH = tbH
		wW = *g.FrameW
		wH = *g.FrameH - tbH
		g.WorldX = 0
		g.WorldY = 0
	}

	*g.Toolbox = ui.NewTileSetFromFS(layoutWide,
	                                uiTileSetW,
	                                tbW,
	                                tbH,
	                                tPaths[:],
	                                pngs)
	g.Toolbox.Bg = color.RGBA{uiToolBgR, uiToolBgG, uiToolBgB, uiToolBgA}
	g.Toolbox.VisibleTiles = g.Toolbox.Tiles[:]

	*g.Matbox = ui.NewTileSetFromImgs(layoutWide,
	                                 uiTileSetW,
	                                 mbW,
	                                 mbH,
	                                 genMatImages())
	g.Matbox.Bg = color.RGBA{uiMatBgR, uiMatBgG, uiMatBgB, uiMatBgA}
	g.Matbox.VisibleTiles = g.Matbox.Tiles[:]

	if layoutWide {
		g.Matbox.Y = g.Toolbox.Size().Y
	} else {
		g.Toolbox.Y = *g.FrameH - g.Toolbox.H
		g.Matbox.X = g.Toolbox.W
		g.Matbox.Y = *g.FrameH - g.Toolbox.H
	}

	*g.World = mat.NewWorld(wW, wH)
	g.WorldImg = ebiten.NewImage(wW, wH)

	// TODO remove one last manual erahplresd
	g.World.Dots[8][10] = mat.Sand
	g.World.Dots[10][10] = mat.Oxygen
	g.World.Dots[10][5] = mat.Hydrogen
	g.World.Dots[12][10] = mat.Sand

	ebiten.SetWindowTitle(AppName + " " + AppVersion)
	ebiten.SetWindowSize(winW, winH)
	ebiten.SetWindowResizingMode(ebiten.WindowResizingModeDisabled)
	ebiten.SetTPS(tickrate)

	ebiten.RunGame(g)
}
