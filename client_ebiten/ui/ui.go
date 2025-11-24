// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

// hawps UI elements
package ui

import (
	"embed"
	"image"
	"image/color"

	"github.com/hajimehoshi/ebiten/v2"
	"github.com/hajimehoshi/ebiten/v2/ebitenutil"
)

const (
	FontCharMaxW = 5
	FontCharMaxH = 8
	FontCharY    = 1
	FontMaxChars = 128
)

var fontChars [FontMaxChars]*ebiten.Image

func DrawnTextLen(
	text string,
	spacing int,
) int {
	var (
		i int
		w int
		ret int
	)

	for i = 0; i < len(text); i++ {
		w, _ = fontChars[text[i]].Size()
		ret += w + spacing
	}
	ret -= spacing

	return ret
}

func DrawText(
	target *ebiten.Image,
	x, y int,
	text string,
	spacing int,
) {
	var (
		char *ebiten.Image
		i int
		opt ebiten.DrawImageOptions
		w int
	)

	for i = 0; i < len(text); i++ {
		char = fontChars[text[i]]
		opt.GeoM.Reset()
		opt.GeoM.Translate(float64(x), float64(y))
		target.DrawImage(char, &opt)
		w, _ = char.Size()
		x += w + spacing
	}
}

func Init(
	fs embed.FS,
	path string,
) {
	var (
		charW int
		i int
		rect  image.Rectangle
	)

	fontimg, fontimgraw, err := ebitenutil.NewImageFromFileSystem(fs, path)
	if err != nil {
		panic(err)
	}

	rect.Min.Y = FontCharY
	rect.Max.Y = rect.Min.Y + FontCharMaxH

	for i = 0; i < len(fontChars); i++ {
		for charW = 0; charW < FontCharMaxW; charW++ {
			if fontimgraw.At(i * FontCharMaxW + charW, 0) !=
			   fontimgraw.ColorModel().Convert(color.White) {
				break
			}
		}
		rect.Min.X = i * FontCharMaxW
		rect.Max.X = rect.Min.X + charW
		fontChars[i] = ebiten.NewImageFromImage(fontimg.SubImage(rect))
	}
}

func pointInRect(px, py, x, y, w, h int) bool {
	if px > x && px < x + w &&
	   py > y && py < y + h {
		return true
	}
	return false
}
