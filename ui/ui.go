// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024  Andy Frank Schoknecht

// hawps UI elements
package ui

import (
	"embed"
	"image"
	"image/color"
	"math"

	"github.com/hajimehoshi/ebiten/v2"
	"github.com/hajimehoshi/ebiten/v2/ebitenutil"
	"github.com/hajimehoshi/ebiten/v2/vector"
)

// The TileSetWidth controls how many tiles are next to each other,
// in respect to being vertical or horizontal.
// It is NOT defining pixels or the like.
// So if you want a 2 x 5 (W x H) TileSet,
// you need a TileWidth of 2 and a Horizontal of false.
// If you want a 5 x 2 (W x H) TileSet,
// you need a TileWidth of 2 and a Horizontal of true.
// The actual pixel size is given to "New" functions.
type TileSet struct {
	X, Y, W, H   int
	Bg           color.Color
	Cursor       int
	horizontal   bool
	Img          *ebiten.Image
	Scroll       int
	tileSetWidth int
	Tiles        []*ebiten.Image
	VisibleTiles []int
	tileW, tileH int
}

func NewTileSetFromImgs(
	horizontal bool,
	tileSetW   int,
	w, h       int,
	imgs       []*ebiten.Image,
) TileSet {
	var ret = TileSet{
		W: w,
		H: h,
		horizontal:   horizontal,
		tileSetWidth: tileSetW,
	}

	ret.tileW = imgs[0].Bounds().Dx()
	ret.tileH = imgs[0].Bounds().Dy()
	ret.Tiles = imgs

	ret.Img = ebiten.NewImage(w, h)

	return ret
}

// This panics if tiles with varying sizes are opened.
// By default no tiles will be saved in the "Visible" array.
func NewTileSetFromFS(
	horizontal bool,
	tileSetW   int,
	w, h       int,
	paths      []string,
	fs         embed.FS,
) TileSet {
	var ret = TileSet{
		W: w,
		H: h,
		horizontal:   horizontal,
		tileSetWidth: tileSetW,
	}

	imgopen := func(i int) {
		img, _, err := ebitenutil.NewImageFromFileSystem(fs,
		                                                 paths[i])
		if err != nil {
			panic(err)
		}
		ret.Tiles = append(ret.Tiles, img)
	}

	imgopen(0)
	ret.tileW = ret.Tiles[0].Bounds().Dx()
	ret.tileH = ret.Tiles[0].Bounds().Dy()

	for i := 1; i < len(paths); i++ {
		imgopen(i)
		if ret.Tiles[i].Bounds().Dx() != ret.tileW ||
		   ret.Tiles[i].Bounds().Dy() != ret.tileH {
			panic("Attempt to init TileSet with heterogeneous images")
		}
	}

	ret.Img = ebiten.NewImage(w, h)

	return ret
}

func (t TileSet) Draw(
) {
	var (
		cx, cy, x, y int
		primary, secondary *int
	)

	t.Img.Fill(t.Bg)

	if t.horizontal {
		primary = &x
		secondary = &y

		cx = t.Cursor % t.tileSetWidth
		cy = t.Cursor / t.tileSetWidth
		cy -= t.Scroll
	} else {
		primary = &y
		secondary = &x

		cx = t.Cursor / t.tileSetWidth
		cy = t.Cursor % t.tileSetWidth
		cx -= t.Scroll
	}

	for i := t.Scroll * t.tileSetWidth; i < len(t.VisibleTiles); i++ {
		opt := ebiten.DrawImageOptions{}
		opt.GeoM.Translate(float64(x * t.tileW), float64(y * t.tileH))
		t.Img.DrawImage(t.Tiles[t.VisibleTiles[i]], &opt)

		*primary++

		if *primary >= t.tileSetWidth {
			*secondary++
			*primary = 0
		}
	}

	cx *= t.tileW
	cx += t.tileW / 2
	cy *= t.tileH
	cy += t.tileH / 2
	vector.StrokeCircle(t.Img,
	                    float32(cx),
	                    float32(cy),
	                    float32(t.tileH / 2 - 1),
	                    1,
	                    image.White,
	                    false)
}

func (t *TileSet) HandleClick(
	x, y int,
) bool {
	if !pointInRect(x, y, t.X, t.Y, t.W, t.H) {
		return false
	}

	tx := (x - t.X) / t.tileW
	ty := (y - t.Y) / t.tileH

	if t.horizontal {
		t.Cursor = (ty + t.Scroll) * t.tileSetWidth + tx
	} else {
		t.Cursor = (tx + t.Scroll) * t.tileSetWidth + ty
	}

	if t.Cursor >= len(t.VisibleTiles) {
		t.Cursor = len(t.VisibleTiles) - 1
	}

	return true
}

func (t *TileSet) HandleWheel(
	x, y int,
	delta int,
) bool {
	if !pointInRect(x, y, t.X, t.Y, t.W, t.H) {
		return false
	}

	t.Scroll -= delta

	rows := int(math.Ceil(float64(len(t.VisibleTiles)) / float64(t.tileSetWidth)))

	if t.Scroll < 0 {
		t.Scroll = 0
	} else if t.Scroll >= rows {
		t.Scroll = rows - 1
	}

	return true
}

func (t TileSet) Size() image.Point {
	return t.Img.Bounds().Size()
}

func pointInRect(px, py, x, y, w, h int) bool {
	if px > x && px < x + w &&
	   py > y && py < y + h {
		return true
	}
	return false
}
