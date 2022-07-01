/*
	Chemarium
	Copyright (C) 2022	Andy Frank Schoknecht

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

pub struct Sprite<'a> {
	font: &'a Font<'a, 'a>,
	texture_creator: &'a TextureCreator<WindowContext>,
	surface: Option<Surface<'a>>,
	texture: Option<Texture<'a>>,
}

impl Sprite<'_> {
	pub fn new<'a>(font: &'a Font, texture_creator: &'a TextureCreator<WindowContext>) -> Sprite<'a> {
		return Sprite {
			font: font,
			texture_creator: texture_creator,
			surface: None,
			texture: None,
		};
	}

	pub fn text(&mut self, text: &str) {
		self.surface = Some(self.font
			.render(text)
			.solid(Color::RGB(255, 255, 255))
			.unwrap());
		
		self.texture = Some(self.surface
			.as_ref()
			.unwrap()
			.as_texture(self.texture_creator)
			.unwrap());
	}

	pub fn rect(&self) -> Rect {
		return self.surface.as_ref().unwrap().rect();
	}

	pub fn draw(&self, canvas: &mut Canvas<Window>, rect: Rect) {
		canvas
			.copy(&self.texture.as_ref().unwrap(), None, Some(rect))
			.unwrap();
	}
}
