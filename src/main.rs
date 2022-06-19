use std::vec::Vec;
use sdl2::event::*;
use sdl2::pixels::*;
use sdl2::rect::*;

#[derive(Copy, Clone)]
pub enum Material {
	None,
	Oxygen,
	CarbonDioxide,
	Sand,
	Dirt,
}

impl Material {
	pub fn color(&self) -> Color {
		match self {
			Material::None => return Color::RGB(0, 0, 0),
			Material::Oxygen => return Color::RGB(155, 219, 245),
			Material::CarbonDioxide => return Color::RGB(155, 219, 245),
			Material::Sand => return Color::RGB(255, 255, 0),
			Material::Dirt => return Color::RGB(255, 255, 0),
		}
	}
}

pub struct Sandbox {
	mats: Vec<Vec<Material>>,
}

impl Sandbox {
	pub fn new(width: usize, height: usize) -> Sandbox {
		// reserve memory and initialize values
		let mut result = Sandbox {
			mats: Vec::new(),
		};

		result.mats = vec![vec![Material::None; height]; width];
		
		return result;
	}
}

fn main() {
	// init
	const WIDTH: usize = 200;
	const HEIGHT: usize = 200;
	
	let sdl = sdl2::init().unwrap();
	let mut sdlsys_event = sdl.event_pump().unwrap();
	let sdlsys_video = sdl.video().unwrap();

	let window = sdlsys_video.window("chemarium", WIDTH as u32, HEIGHT as u32)
		.position_centered()
		.build()
		.unwrap();
		
	let mut canvas = window.into_canvas().build().unwrap();

	// mainloop
	let mut sandbox = Sandbox::new(WIDTH, HEIGHT);

	'mainloop: loop {
	
		// events
		for event in sdlsys_event.poll_iter() {
		
			match event {
				Event::MouseMotion {mousestate, x, y, ..} => {
					// if lmb is held down while mousemotion
					if mousestate.left() {
					
						// spawn selected material
						sandbox.mats[x as usize][y as usize] = Material::Sand;
					}
				},

				Event::Quit {..} => break 'mainloop,
				
				_ => ()
			}
		}

		// draw bg
		canvas.set_draw_color(Color::RGB(0, 0, 0));
		canvas.fill_rect(
			Rect::new(
				0,
				0, 
				sandbox.mats.len() as u32,
				sandbox.mats[0].len() as u32)).unwrap();

		// pixels
		for x in 0..sandbox.mats.len() {
			for y in 0..x {
				canvas.set_draw_color(sandbox.mats[x][y].color());
				canvas.draw_point(Point::new(x as i32, y as i32)).unwrap();
			}
		}
		
		canvas.present();
	}
}
