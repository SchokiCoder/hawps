// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

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
	celsiusToKelvin = 273.15

	stdBrushRadius  = 2
	stdEraserRadius = 5
	stdThermoRadius = stdBrushRadius
	maxRadius = 32
	thermalVisionMinT  = -75 + celsiusToKelvin

	stdTickrate     = 120
	stdWldTRateFrac = 0.25
	heaterDelta     = 1

	firstRealMat   = mat.Sand
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
	spawnerA       = 10

	stdTemperature = 20 + celsiusToKelvin
	stdWinW        = 640
	stdWinH        = 480

	toolHoverR     = 0
	toolHoverG     = 255
	toolHoverB     = 0
	toolHoverA     = spawnerA

	wBgR           = 0
	wBgG           = 0
	wBgB           = 0
	wThBgR         = 100
	wThBgG         = 0
	wThBgB         = 0

	// Kelvin / 100
	glowRange    = 77
	noGlowRange  = 7  // draper point
	dimGlowRange = 40
)

type physGame struct {
	BgColor      color.RGBA
	BrushRadius  int
	EraserRadius int
	ThermoRadius int
	FrameW       int
	FrameH       int
	GlowColors   [glowRange]color.RGBA
	GlowImg      *ebiten.Image
	Paused       bool
	Temperature  float64
	ThVision     bool
	Tickrate     int
	Toolbox      ui.TileSet
	// ticks since world tick
	TsSinceWldT  int
	Matbox       ui.TileSet
	ToolImg      *ebiten.Image
	WldTRateFrac float64
	World        mat.World
	WorldImg     *ebiten.Image
	WorldX       int
	WorldY       int
}

func newPhysGame(
) physGame {
	var ret = physGame{
		BgColor:      color.RGBA{R: wBgR, G: wBgG, B: wBgB, A: 255},
		BrushRadius:  stdBrushRadius,
		EraserRadius: stdEraserRadius,
		ThermoRadius: stdThermoRadius,
		Temperature:  stdTemperature,
		Tickrate:     stdTickrate,
		TsSinceWldT:  9999,
		WldTRateFrac: stdWldTRateFrac,
	}

	/* Instead of doing expensive HSV stuff for temperature based glowing,
	 * we prepare an array of glow colors,
	 * which we then iterate through based on dot temperature.
	 */
	for i := 0; i < noGlowRange; i++ {
		ret.GlowColors[i] = color.RGBA{0, 0, 0, 0}
	}
	for i := noGlowRange; i < dimGlowRange; i++ {
		ret.GlowColors[i] = color.RGBA{255, 0, 0,
			uint8(255.0 /
				100.0 *
				(100.0 /
					float64(dimGlowRange - noGlowRange) *
					float64(i - noGlowRange)))}
	}
	chromaGlow := [...]color.RGBA{
		color.RGBA{255, 0, 0, 255},
		color.RGBA{255, 0, 0, 255},
		color.RGBA{255, 0, 0, 255},
		color.RGBA{255, 0, 0, 255},
		color.RGBA{255, 0, 0, 255},

		color.RGBA{255, 63, 0, 255},
		color.RGBA{255, 127, 0, 255},
		color.RGBA{255, 127, 0, 255},
		color.RGBA{255, 190, 0, 255},

		color.RGBA{255, 127, 0, 255},
		color.RGBA{255, 255, 0, 255},
		color.RGBA{127, 255, 0, 255},

		color.RGBA{0, 255, 0, 255},
		color.RGBA{0, 255, 0, 255},
		color.RGBA{0, 255, 0, 255},
		color.RGBA{0, 255, 85, 255},
		color.RGBA{0, 255, 170, 255},

		color.RGBA{0, 255, 255, 255},
		color.RGBA{0, 255, 255, 255},
		color.RGBA{0, 170, 255, 255},

		color.RGBA{0, 85, 255, 255},
		color.RGBA{0, 0, 255, 255},
		color.RGBA{0, 0, 255, 255},
		color.RGBA{0, 0, 255, 255},
		color.RGBA{0, 0, 255, 255},
		color.RGBA{0, 0, 255, 255},
		color.RGBA{19, 0, 255, 255},
		color.RGBA{37, 0, 255, 255},
		color.RGBA{55, 0, 255, 255},

		color.RGBA{73, 0, 255, 255},
		color.RGBA{91, 0, 255, 255},
		color.RGBA{109, 0, 255, 255},
		color.RGBA{127, 0, 255, 255},
		color.RGBA{127, 0, 255, 255},
		color.RGBA{127, 0, 255, 255},
		color.RGBA{127, 0, 255, 255},
		color.RGBA{127, 0, 255, 170},
		color.RGBA{127, 0, 255, 85},
	}
	for i := dimGlowRange; i < glowRange; i++ {
		ret.GlowColors[i] = chromaGlow[i - dimGlowRange]
	}

	return ret
}

func (g physGame) Draw(
	screen *ebiten.Image,
) {
	getNormalDotColor := func(
		x, y int,
	) color.Color {
		return color.RGBA{
			mat.Rs[g.World.Dots[x][y]],
			mat.Gs[g.World.Dots[x][y]],
			mat.Bs[g.World.Dots[x][y]],
			mat.As[g.World.Dots[x][y]]}
	}

	getThermalDotColor := func(
		x, y int,
	) color.Color {
		if mat.None == g.World.Dots[x][y] {
			return getNormalDotColor(x, y)
		}

		visT := g.World.Thermo[x][y]
		if visT > thermalVisionMinT + 255 {
			visT = thermalVisionMinT + 255
		} else if visT < thermalVisionMinT {
			visT = thermalVisionMinT
		}

		return color.Gray{uint8(visT - thermalVisionMinT)}
	}

	getGlowColor := func(
		x, y int,
	) color.Color {
		var glowIndex int

		if mat.None == g.World.Dots[x][y] {
			return color.RGBA{0, 0, 0, 0}
		}

		glowIndex = int(g.World.Thermo[x][y] / 100.0)
		if glowIndex >= glowRange {
			return color.RGBA{0, 0, 0, 0}
		}

		return g.GlowColors[glowIndex]
	}

	var (
		getDotColor func(int, int) color.Color
		drawDotGlow func(x, y int)
		drawGlowImg func()
		opt = ebiten.DrawImageOptions{}
	)

	screen.Fill(g.BgColor)
	g.GlowImg.Clear()
	g.ToolImg.Clear()
	g.WorldImg.Clear()

	g.Toolbox.Draw()
	g.Matbox.Draw()

	opt.GeoM.Translate(float64(g.Toolbox.X), float64(g.Toolbox.Y))
	screen.DrawImage(g.Toolbox.Img, &opt)

	opt.GeoM.Reset()
	opt.GeoM.Translate(float64(g.Matbox.X), float64(g.Matbox.Y))
	screen.DrawImage(g.Matbox.Img, &opt)

	if g.ThVision {
		getDotColor = getThermalDotColor
		drawDotGlow = func(x, y int) {}
		drawGlowImg = func() {}
	} else {
		getDotColor = getNormalDotColor
		drawDotGlow = func(x, y int) {g.GlowImg.Set(x, y, getGlowColor(x, y))}
		drawGlowImg = func() {
			screen.DrawImage(g.GlowImg, &opt)
		}
	}

	// So I was initially trying to use ebiten.Image.WritePixels(),
	// but it literally didn't do anything.
	// Couldn't find an update-like function either.
	// So away it went for Set().
	for x := 0; x < g.World.W; x++ {
		for y := 0; y < g.World.H; y++ {
			if true == g.World.Spawner[x][y] {
				g.ToolImg.Set(x, y,
					color.RGBA{
						spawnerR,
						spawnerG,
						spawnerB,
						spawnerA})
				continue
			}

			g.WorldImg.Set(x, y, getDotColor(x, y))
			drawDotGlow(x, y)
		}
	}

	radius := 0
	switch tool(g.Toolbox.Cursor) {
	case brush:
		radius = g.BrushRadius
	case eraser:
		radius = g.EraserRadius
	case heater: fallthrough
	case cooler:
		radius = g.ThermoRadius
	}

	thX, thY := ebiten.CursorPosition()
	thX -= radius + g.WorldX
	thY -= radius + g.WorldY
	thX2 := thX + radius * 2 + 1
	thY2 := thY + radius * 2 + 1
	for x := thX; x < thX2; x++ {
		for y := thY; y < thY2; y++ {
			g.ToolImg.Set(x, y,
				color.RGBA{
					toolHoverR,
					toolHoverG,
					toolHoverB,
					toolHoverA})
		}
	}

	opt.GeoM.Reset()
	opt.GeoM.Translate(float64(g.WorldX), float64(g.WorldY))
	screen.DrawImage(g.WorldImg, &opt)

	drawGlowImg()

	screen.DrawImage(g.ToolImg, &opt)
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
				g.Temperature,
				mX - g.WorldX,
				mY - g.WorldY,
				g.BrushRadius)

		case spawner:
			g.World.Spawner[mX - g.WorldX][mY - g.WorldY] = true
			g.World.SpwnMat[mX - g.WorldX][mY - g.WorldY] =
				mat.Mat(g.Matbox.VisibleTiles[g.Matbox.Cursor])

		case eraser:
			g.World.UseEraser(
				mX - g.WorldX,
				mY - g.WorldY,
				g.EraserRadius)

		case heater:
			g.World.UseHeater(
				heaterDelta,
				mX - g.WorldX,
				mY - g.WorldY,
				g.ThermoRadius)

		case cooler:
			g.World.UseCooler(
				heaterDelta,
				mX - g.WorldX,
				mY - g.WorldY,
				g.ThermoRadius)

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
		target *int
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
	if mX >= g.WorldX && mX < g.WorldX + g.World.W &&
	   mY >= g.WorldY && mY < g.WorldY + g.World.H {
		switch tool(g.Toolbox.Cursor) {
		case brush:
			target = &g.BrushRadius
		case eraser:
			target = &g.EraserRadius
		case heater: fallthrough
		case cooler:
			target = &g.ThermoRadius
		default:
			return
		}
		*target += delta
		if *target > maxRadius {
			*target = maxRadius
		} else if *target < 0 {
			*target = 0
		}
		return
	}
}

func (g physGame) Layout(
	outsideWidth int,
	outsideHeight int,
) (int, int) {
	return g.FrameW, g.FrameH
}

func (g *physGame) Update(
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
			g.Paused = !g.Paused

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
			if g.WldTRateFrac < 1.0 {
				g.WldTRateFrac *= 2
			}

		case ebiten.KeyNumpadSubtract:
			if g.WldTRateFrac * float64(g.Tickrate) > 1.0 {
				g.WldTRateFrac /= 2
			}

		case ebiten.KeyT:
			g.ThVision = !g.ThVision
			if true == g.ThVision {
				g.BgColor.R = wThBgR
				g.BgColor.G = wThBgG
				g.BgColor.B = wThBgB
			} else {
				g.BgColor.R = wBgR
				g.BgColor.G = wBgG
				g.BgColor.B = wBgB
			}
		}
	}

	if ebiten.IsMouseButtonPressed(ebiten.MouseButtonLeft) {
		g.HandleClick()
	}

	g.HandleWheel()

	g.World.Update(g.Temperature)

	if !g.Paused {
		if g.TsSinceWldT >= int(1.0 / g.WldTRateFrac) {
			g.World.Simulate()
			g.TsSinceWldT = 0
		} else {
			g.TsSinceWldT++
		}
	}

	return nil
}

func (g *physGame) UpdateMatbox(
) {
	var tiles = make([]int, 0)

	g.Matbox.Cursor = 0
	g.Matbox.Scroll = 0

	switch(tool(g.Toolbox.Cursor)) {
	case brush:
		for i := firstRealMat; i < mat.Mat(mat.Count); i++ {
			tiles = append(tiles, int(i))
		}
		g.Matbox.VisibleTiles = tiles

	case spawner:
		tiles = append(tiles, int(mat.None))
		for i := firstRealMat; i < mat.Mat(mat.Count); i++ {
			state := tempMatToState(g.Temperature, i)
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
        Increase and decrease the simulation speed respectively
        default: %v updates per second

    T
        Toggle thermal vision (grayscale displaying %v to %v degree Celsius)

    Wheel Up and Down
        Scrolls a TileSet or increases/decreases the tool radius,
        depending on where the mouse is at the time
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

	for i := mat.None; i < mat.Mat(mat.Count); i++ {
		path := matPrefix + i.String() + postfix
		matImg := imgopen(path)
		opt := ebiten.DrawImageOptions{}
		img := ebiten.NewImage(pngSize, pngSize)

		state := tempMatToState(t, i)
		c := color.RGBA{
			R: mat.Rs[i],
			G: mat.Gs[i],
			B: mat.Bs[i],
			A: mat.As[i],
		}

		opt.ColorM.ScaleWithColor(c)
		img.DrawImage(bgImgs[state], &opt)

		opt.ColorM.Reset()
		opt.ColorM.ScaleWithColor(color.RGBA{
			R: 255 - c.R,
			G: 255 - c.G,
			B: 255 - c.B,
			A: 255})
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

		case "-H": fallthrough
		case "-height":
			*winH = argToInt(i)
			i++

		case "-h": fallthrough
		case "-help":
			fmt.Printf(appHelp,
			           AppName,
			           stdWinH,
			           celsiusToKelvin,
			           stdTemperature,
			           stdTickrate,
			           stdWinW,
			           stdTickrate * stdWldTRateFrac,
			           thermalVisionMinT - celsiusToKelvin,
			           thermalVisionMinT - celsiusToKelvin + 255)
			return false

		case "-noborder":
			ebiten.SetWindowDecorated(false)

		case "-tallui":
			*layout = tall

		case "-temperature":
			*temperature = float64(argToInt(i))
			if *temperature < 0 {
				panic("The value for \"" +
					os.Args[i] +
					"\" must not be negative")
			}
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
		&g.Temperature,
		&g.Tickrate,
		&winW,
		&winH,
	) == false {
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

	switch layout {
	case automatic:
		if g.FrameW >= g.FrameH {
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
		mbH = g.FrameH - tbH
		wW = g.FrameW - tbW
		wH = g.FrameH
		g.WorldX = tbW
		g.WorldY = 0
	} else {
		tbW = pngSize * 2
		tbH = uiTileSetW * pngSize
		mbW = g.FrameW - tbW
		mbH = tbH
		wW = g.FrameW
		wH = g.FrameH - tbH
		g.WorldX = 0
		g.WorldY = 0
	}

	g.Toolbox = ui.NewTileSetFromImgs(
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

	g.Matbox = ui.NewTileSetFromImgs(
		tsWide,
		uiTileSetW,
		mbW,
		mbH,
		genMatImages(g.Temperature))
	g.Matbox.Bg = color.RGBA{uiMatBgR, uiMatBgG, uiMatBgB, uiMatBgA}

	g.UpdateMatbox()

	if true == tsWide {
		g.Matbox.Y = g.Toolbox.Size().Y
	} else {
		g.Toolbox.Y = g.FrameH - g.Toolbox.H
		g.Matbox.X = g.Toolbox.W
		g.Matbox.Y = g.FrameH - g.Toolbox.H
	}

	g.World = mat.NewWorld(wW, wH, g.Temperature)
	g.ToolImg = ebiten.NewImage(wW, wH)
	g.WorldImg = ebiten.NewImage(wW, wH)
	g.GlowImg = ebiten.NewImage(wW, wH)

	ebiten.SetWindowTitle(AppName + " " + AppVersion)
	ebiten.SetWindowSize(winW, winH)
	ebiten.SetWindowResizingMode(ebiten.WindowResizingModeDisabled)
	ebiten.SetTPS(g.Tickrate)

	ebiten.RunGame(&g)
}
