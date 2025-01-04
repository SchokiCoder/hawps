// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

//go:generate stringer -type=tool
package main

import (
	"embed"
	"fmt"
	"image"
	"image/color"
	_ "image/png"
	"strconv"
	"os"

	"github.com/SchokiCoder/hawps/mat"
	"github.com/SchokiCoder/hawps/ui"

	"github.com/hajimehoshi/ebiten/v2"
	"github.com/hajimehoshi/ebiten/v2/ebitenutil"
	"github.com/hajimehoshi/ebiten/v2/inpututil"
	"github.com/hajimehoshi/ebiten/v2/vector"
)

type uiLayout int
const (
	automatic uiLayout = iota
	tall
	wide
)

type tool int
const (
	brush tool = iota
	spawner
	eraser
	heater
	cooler

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
	brushRadius    = 2
	eraserRadius   = 5
	thermoRadius   = brushRadius

	maxTickrate    = stdTickrate * 2
	minTickrate    = 1
	heaterDelta    = 10

	pngSize        = 16

	uiToolBgR      = 80
	uiToolBgG      = 120
	uiToolBgB      = 120
	uiToolBgA      = 255
	uiMatBgR       = 80
	uiMatBgG       = 110
	uiMatBgB       = 130
	uiMatBgA       = 255
	uiTileSetW     = 3

	spawnerR       = 255
	spawnerG       = 0
	spawnerB       = 255

	stdTemperature = 20
	stdTickrate    = 24
	stdWinW        = 640
	stdWinH        = 480

	toolHoverR     = 0
	toolHoverG     = 255
	toolHoverB     = 0

	wBgR           = 0
	wBgG           = 0
	wBgB           = 0
	wThBgR         = 100
	wThBgG         = 0
	wThBgB         = 0
)

type physGame struct {
	FrameW      *int
	FrameH      *int
	Temperature *float64
	Tickrate    *int
	Toolbox     *ui.TileSet
	Matbox      *ui.TileSet
	World       *mat.World
	WorldImg    *ebiten.Image
	WorldX      int
	WorldY      int
}

func newPhysGame(
) physGame {
	var ret = physGame{
		FrameW:      new(int),
		FrameH:      new(int),
		Temperature: new(float64),
		Tickrate:    new(int),
		Toolbox:     new(ui.TileSet),
		Matbox:      new(ui.TileSet),
		World:       new(mat.World),
	}
	*ret.Temperature = stdTemperature
	*ret.Tickrate = stdTickrate

	return ret
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
			if true == g.World.Spawner[x][y] {
				g.WorldImg.Set(x, y,
					color.RGBA{
						spawnerR,
						spawnerG,
						spawnerB,
						255})
				continue
			}

			g.WorldImg.Set(x, y,
				color.RGBA{
					g.World.Rs[x][y],
					g.World.Gs[x][y],
					g.World.Bs[x][y],
					255})
		}
	}

	radius := 0
	switch tool(g.Toolbox.Cursor) {
	case brush:
		radius = brushRadius
	case eraser:
		radius = eraserRadius
	case heater: fallthrough
	case cooler:
		radius = thermoRadius
	}

	thX, thY := ebiten.CursorPosition()
	thX -= radius + g.WorldX
	thY -= radius + g.WorldY
	thX2 := thX + radius * 2 + 1
	thY2 := thY + radius * 2 + 1
	for x := thX; x < thX2; x++ {
		for y := thY; y < thY2; y++ {
			g.WorldImg.Set(x, y,
				color.RGBA{
					toolHoverR,
					toolHoverG,
					toolHoverB,
					255})
		}
	}

	opt.GeoM.Reset()
	opt.GeoM.Translate(float64(g.WorldX), float64(g.WorldY))
	screen.DrawImage(g.WorldImg, &opt)
}

func (g *physGame) HandleClick(
) {
	var (
		clicked bool
		mX, mY  int
	)

	mX, mY = ebiten.CursorPosition()

	clicked = g.Toolbox.HandleClick(mX, mY)
	if clicked {
		g.UpdateMatbox()
	}

	if !clicked {
		clicked = g.Matbox.HandleClick(mX, mY)
	}

	if !clicked &&
	   mX >= g.WorldX && mX < g.WorldX + g.World.W &&
	   mY >= g.WorldY && mY < g.WorldY + g.World.H {
		curTool := tool(g.Toolbox.Cursor)

		switch curTool {
		case brush:
			g.World.UseBrush(
				mat.Mat(g.Matbox.VisibleTiles[g.Matbox.Cursor]),
				*g.Temperature,
				mX - g.WorldX,
				mY - g.WorldY,
				brushRadius)

		case spawner:
			g.World.Spawner[mX - g.WorldX][mY - g.WorldY] = true
			g.World.SpwnMat[mX - g.WorldX][mY - g.WorldY] =
				mat.Mat(g.Matbox.VisibleTiles[g.Matbox.Cursor])

		case eraser:
			g.World.UseEraser(
				mX - g.WorldX,
				mY - g.WorldY,
				eraserRadius)

		case heater:
			g.World.UseThermoChanger(
				heaterDelta,
				mX - g.WorldX,
				mY - g.WorldY,
				thermoRadius)

		case cooler:
			g.World.UseThermoChanger(
				heaterDelta * -1,
				mX - g.WorldX,
				mY - g.WorldY,
				thermoRadius)

		default:
			panic("Used unknown tool " + curTool.String())
		}
	}
}

func (g *physGame) HandleWheel(
) {
	var (
		mX, mY int
		delta int
	)

	mX, mY = ebiten.CursorPosition()
	_, tmp := ebiten.Wheel()
	delta = int(tmp)

	if 0 == delta {
		return
	}

	if g.Toolbox.HandleWheel(mX, mY, delta) {
		return
	}
	if g.Matbox.HandleWheel(mX, mY, delta) {
		return
	}
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
		keys    []ebiten.Key
	)

	keys = inpututil.AppendJustPressedKeys(keys)

	for i := 0; i < len(keys); i++ {
		switch (keys[i]) {
		case ebiten.KeyEscape:
			return ebiten.Termination

		case ebiten.KeySpace:
			g.World.Paused = !g.World.Paused

		case ebiten.KeyArrowLeft:
			if g.Toolbox.Cursor > 0 {
				g.Toolbox.Cursor--
				g.UpdateMatbox()
			}

		case ebiten.KeyArrowRight:
			if g.Toolbox.Cursor < len(g.Toolbox.VisibleTiles) - 1 {
				g.Toolbox.Cursor++
				g.UpdateMatbox()
			}

		case ebiten.KeyArrowUp:
			if g.Matbox.Cursor > 0 {
				g.Matbox.Cursor--
			}

		case ebiten.KeyArrowDown:
			if g.Matbox.Cursor < len(g.Matbox.VisibleTiles) - 1 {
				g.Matbox.Cursor++
			}

		case ebiten.KeyNumpadAdd:
			if *g.Tickrate < maxTickrate {
				*g.Tickrate *= 2
				ebiten.SetTPS(*g.Tickrate)
			}

		case ebiten.KeyNumpadSubtract:
			if *g.Tickrate > minTickrate {
				*g.Tickrate /= 2
				ebiten.SetTPS(*g.Tickrate)
			}

		case ebiten.KeyT:
			g.World.ThVision = !g.World.ThVision
			if true == g.World.ThVision {
				mat.ChangeBgColor(wThBgR, wThBgG, wThBgB)
			} else {
				mat.ChangeBgColor(wBgR, wBgG, wBgB)
			}
		}
	}

	if ebiten.IsMouseButtonPressed(ebiten.MouseButtonLeft) {
		g.HandleClick()
	}

	g.HandleWheel()

	g.World.Tick(*g.Temperature)

	return nil
}

func (g physGame) UpdateMatbox(
) {
	var tiles = make([]int, 0)

	g.Matbox.Cursor = 0
	g.Matbox.Scroll = 0

	switch(tool(g.Toolbox.Cursor)) {
	case brush:
		for i := mat.FirstReal; i <= mat.Last; i++ {
			tiles = append(tiles, int(i))
		}
		g.Matbox.VisibleTiles = tiles

	case spawner:
		tiles = append(tiles, int(mat.None))
		for i := mat.FirstReal; i <= mat.Last; i++ {
			state := tempMatToState(*g.Temperature, i)
			if state != mat.MsStatic {
				tiles = append(tiles, int(i))
			}
		}
		g.Matbox.VisibleTiles = tiles

	case eraser: fallthrough
	case heater: fallthrough
	case cooler:
		g.Matbox.VisibleTiles = nil
		g.Matbox.Cursor = -1
	}
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

    -tallui
        overrides automatic layout determination, and sets tall ui

    -temperature
        sets the temperature of every new dot
        default: %v

    -tickrate NUMBER
        sets the tickrate (ticks per second), which effects visible speed
        default: %v

    -v -version
        prints version information then exits

    -wideui
        overrides automatic layout determination, and sets wide ui

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

    Arrow Left and Right
        switch tool

    Arrow Up and Down
        switch material

    Plus and Minus
        Increase and decrease the tickrate respectively
        min: %v
        max: %v

    T
        Toggle thermal vision (grayscale displaying %v to %v degree C)
`;

func genMatImages(t float64) []*ebiten.Image {
	var (
		bgImgs [mat.MsCount]*ebiten.Image
		matBgPrefix = "assets/matbg_"
		matBgPaths = [mat.MsCount]string{
			"static",
			"grain",
			"liquid",
			"gas",
		}
		matPrefix = "assets/mat_"
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

	for i := mat.None; i <= mat.Last; i++ {
		path := matPrefix + i.String() + postfix
		matImg := imgopen(path)
		opt := ebiten.DrawImageOptions{}
		img := ebiten.NewImage(pngSize, pngSize)

		state := tempMatToState(t, i)
		var r, g, b uint8
		switch state {
		case mat.MsGrain:  fallthrough
		case mat.MsStatic:
			r = mat.SolRs(i)
			g = mat.SolGs(i)
			b = mat.SolBs(i)

		case mat.MsLiquid:
			r = mat.LiqRs(i)
			g = mat.LiqGs(i)
			b = mat.LiqBs(i)

		case mat.MsGas:
			r = mat.GasRs(i)
			g = mat.GasGs(i)
			b = mat.GasBs(i)
		}

		opt.ColorM.ScaleWithColor(color.RGBA{r, g, b, 255})
		img.DrawImage(bgImgs[state], &opt)

		opt.ColorM.Reset()
		opt.ColorM.ScaleWithColor(color.RGBA{255-r, 255-g, 255-b, 255})
		opt.GeoM.Translate(float64(pngSize - matImg.Bounds().Dx()) / 2,
		                   float64(pngSize - matImg.Bounds().Dy() - 1))
		img.DrawImage(matImg, &opt)
		ret = append(ret, img)
	}

	return ret
}

func genToolImages() []*ebiten.Image {
	var (
		prefix = "assets/tool_"
		postfix = ".png"
		ret = make([]*ebiten.Image, tCount)
	)

	imgopenAndDraw := func(path string, dest *ebiten.Image) {
		img, _, err := ebitenutil.NewImageFromFileSystem(pngs, path)
		if err != nil {
			panic(err)
		}
		dest.DrawImage(img, nil)
	}

	for i := 0; i < tCount; i++ {
		ret[i] = ebiten.NewImage(pngSize, pngSize)

		vector.DrawFilledCircle(
			ret[i],
			float32(pngSize / 2),
			float32(pngSize / 2),
			float32(pngSize / 2) + 2,
			image.Black,
			false)

		path := prefix + tool(i).String() + postfix

		imgopenAndDraw(path, ret[i])
	}

	return ret
}

func handleArgs(
	layout      *uiLayout,
	temperature *float64,
	tickrate    *int,
	winW        *int,
	winH        *int,
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
			           stdTemperature,
			           stdTickrate,
			           stdWinW,
			           minTickrate,
			           maxTickrate,
			           mat.ThermalVisionMinT,
			           mat.ThermalVisionMinT+255)
			return false

		case "-noborder":
			ebiten.SetWindowDecorated(false)

		case "-tallui":
			*layout = tall

		case "-temperature":
			*temperature = float64(argToInt(i))
			i++

		case "-tickrate":
			*tickrate = argToInt(i)
			i++

		case "-v": fallthrough
		case "-version":
			fmt.Printf("%v: version %v\n", AppName, AppVersion)
			return false

		case "-wideui":
			*layout = wide

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

func tempMatToState(
	t float64,
	m mat.Mat,
) mat.State {
	var ret mat.State

	if t < mat.MeltPs(m) {
		ret = mat.SolidSs(m)
	} else if t < mat.BoilPs(m) {
		ret = mat.MsLiquid
	} else {
		ret = mat.MsGas
	}

	return ret
}

func main(
) {
	var (
		g          physGame = newPhysGame()
		layout     uiLayout
		tiles      []int
		tsWide     bool
		winW       int = stdWinW
		winH       int = stdWinH
		mbW, mbH   int
		tbW, tbH   int
		wW, wH     int
	)

	ebiten.SetFullscreen(true);

	if handleArgs(
		&layout,
		g.Temperature,
		g.Tickrate,
		&winW,
		&winH,
	) == false {
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

	switch layout {
	case automatic:
		if *g.FrameW >= *g.FrameH {
			tsWide = true
		} else {
			tsWide = false
		}

	case wide:
		tsWide = true
	}

	if true == tsWide {
		tbW = uiTileSetW * pngSize
		tbH = pngSize * 2
		mbW = tbW
		mbH = *g.FrameH - tbH
		wW = *g.FrameW - tbW
		wH = *g.FrameH
		g.WorldX = tbW
		g.WorldY = 0
	} else {
		tbW = pngSize * 2
		tbH = uiTileSetW * pngSize
		mbW = *g.FrameW - tbW
		mbH = tbH
		wW = *g.FrameW
		wH = *g.FrameH - tbH
		g.WorldX = 0
		g.WorldY = 0
	}

	*g.Toolbox = ui.NewTileSetFromImgs(
		tsWide,
		uiTileSetW,
		tbW,
		tbH,
		genToolImages())
	g.Toolbox.Bg = color.RGBA{uiToolBgR, uiToolBgG, uiToolBgB, uiToolBgA}

	for i := 0; i < len(g.Toolbox.Tiles); i++ {
		tiles = append(tiles, i)
	}
	g.Toolbox.VisibleTiles = tiles

	*g.Matbox = ui.NewTileSetFromImgs(
		tsWide,
		uiTileSetW,
		mbW,
		mbH,
		genMatImages(*g.Temperature))
	g.Matbox.Bg = color.RGBA{uiMatBgR, uiMatBgG, uiMatBgB, uiMatBgA}

	g.UpdateMatbox()

	if true == tsWide {
		g.Matbox.Y = g.Toolbox.Size().Y
	} else {
		g.Toolbox.Y = *g.FrameH - g.Toolbox.H
		g.Matbox.X = g.Toolbox.W
		g.Matbox.Y = *g.FrameH - g.Toolbox.H
	}

	*g.World = mat.NewWorld(wW, wH, *g.Temperature)
	g.WorldImg = ebiten.NewImage(wW, wH)

	ebiten.SetWindowTitle(AppName + " " + AppVersion)
	ebiten.SetWindowSize(winW, winH)
	ebiten.SetWindowResizingMode(ebiten.WindowResizingModeDisabled)
	ebiten.SetTPS(*g.Tickrate)

	ebiten.RunGame(g)
}
