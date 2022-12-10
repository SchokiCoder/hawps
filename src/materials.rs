/*
 * Chemarium
 * Copyright (C) 2022  Andy Frank Schoknecht
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not see
 * <https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
 */

use sdl2::pixels::Color;

pub struct Material {
	pub name: &'static str,
	pub color: Color,
	pub weight: u32,
	pub melt_point: i16,
	pub boil_point: i16,
}

pub const MATERIALS: [Material; 14] = [
	Material {
		name: "None",
		color: Color::RGB(0, 0, 0),
		weight: 0,
		melt_point: 0,
		boil_point: 0,
	},

	Material {
		name: "Oxygen",
		color: Color::RGB(155, 219, 245),
		weight: 32,
		melt_point: -219,
		boil_point: -183,
	},

	Material {
		name: "CarbonDioxide",
		color: Color::RGB(155, 219, 245),
		weight: 44,
		melt_point: -57,
		boil_point: -78,
	},

	Material {
		name: "Hydrogen",
		color: Color::RGB(155, 219, 245),
		weight: 2,
		melt_point: -259,
		boil_point: -256,
	},

	Material {
		name: "Nitrogen",
		color: Color::RGB(155, 219, 245),
		weight: 28,
		melt_point: -210,
		boil_point: -196,
	},

	Material {
		name: "Carbon",
		color: Color::RGB(10, 10, 10),
		weight: 12,
		melt_point: 3550,
		boil_point: 4200,
	},

	Material {
		name: "SiliconDioxide",
		color: Color::RGB(255, 255, 0),
		weight: 60,
		melt_point: 1710,
		boil_point: 2230,
	},

	Material {
		name: "Silicon",
		color: Color::RGB(50, 50, 50),
		weight: 28,
		melt_point: 1410,
		boil_point: 2355,
	},

	Material {
		name: "Aluminum",
		color: Color::RGB(200, 200, 220),
		weight: 27,
		melt_point: 660,
		boil_point: 2327,
	},

	Material {
		name: "Iron",
		color: Color::RGB(170, 170, 170),
		weight: 56,
		melt_point: 1538,
		boil_point: 2861,
	},

	Material {
		name: "Calcium",
		color: Color::RGB(230, 230, 230),
		weight: 40,
		melt_point: 840,
		boil_point: 1484,
	},

	Material {
		name: "Sodium",
		color: Color::RGB(230, 230, 230),
		weight: 23,
		melt_point: 98,
		boil_point: 880,
	},

	Material {
		name: "Magnesium",
		color: Color::RGB(190, 190, 190),
		weight: 24,
		melt_point: 650,
		boil_point: 1100,
	},

	Material {
		name: "Potassium",
		color: Color::RGB(220, 220, 220),
		weight: 39,
		melt_point: 63,
		boil_point: 762,
	},
];
