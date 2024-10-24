// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

package mats

type Mat int
const (
	None = iota
	Sand
	Water
)

type MatStructure int
const (
	MsStatic = iota
	MsGrain
	MsLiquid
)

type matDefinition struct {
	Weight int
	Structure MatStructure
	R uint8
	G uint8
	B uint8
}

var matDefs = [...]matDefinition {
	/* None */  {Weight: 0, Structure: MsStatic, R: 0,   G: 0,   B: 0},
	/* Sand */  {Weight: 2, Structure: MsGrain,  R: 238, G: 217, B: 86},
	/* Water */ {Weight: 1, Structure: MsLiquid, R: 0,   G: 253, B: 255},
}

func Weight(mat Mat) int {
	return matDefs[mat].Weight
}

func Structure(mat Mat) MatStructure {
	return matDefs[mat].Structure
}

func R(mat Mat) uint8 {
	return matDefs[mat].R
}

func G(mat Mat) uint8 {
	return matDefs[mat].G
}

func B(mat Mat) uint8 {
	return matDefs[mat].B
}
