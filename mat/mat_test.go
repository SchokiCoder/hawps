// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

package mat

import "testing"

const (
	W = 1000
	H = 1000
)

func TestBench(
	_ *testing.T,
) {
	var (
		w = NewWorld(W, H, 20)
	)

	w.UseBrush(Oxygen,   50, W / 3, H / 2, W / 3)
	w.UseBrush(Hydrogen, 70, W / 3 * 2, H / 2, W / 3)

	for i := 0; i < 200; i++ {
		w.Update(20)
		w.Simulate()
	}
}
