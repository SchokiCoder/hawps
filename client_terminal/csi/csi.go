// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2026  Andy Frank Schoknecht

package csi

import (
	"fmt"
	"os"
)

const (
	SigInt  = "\003"
	SigTstp = "\004"

	Backspace = "\x7f"

	Clear = "\033[H\033[2J"

	CursorHide  = "\033[?25l"
	CursorShow  = "\033[?25h"
	CursorUp = "\033[A"
	CursorDown = "\033[B"
	CursorRight = "\033[C"
	CursorLeft = "\033[D"

	Home = "\x1b[H"
	Insert = "\033[2~"
	Delete = "\033[3~"
	PgUp = "\033[5~"
	PgDown = "\033[6~"
	End = "\x1b[F"
	FgDefault = "\033[39m"
	BgDefault = "\033[49m"
)

func GetCursorPos(
) (int, int) {
	var x, y int

	fmt.Print("\033[6n")
	fmt.Fscanf(os.Stdout, "\033[e;%v;%vR", x, y)

	return x, y
}

func SetCursorPos(
	x int,
	y int,
) {
	fmt.Printf("\033[%v;%vH", y, x)
}
