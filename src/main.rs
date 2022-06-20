use std::vec::Vec;
use sdl2::keyboard::*;
use sdl2::event::*;
use sdl2::pixels::*;
use sdl2::rect::*;

pub struct Material {
	name: &'static str,
	color: Color,
}

const MAT_COUNT: usize = 5;

const MATERIALS: [Material; MAT_COUNT] = [
	Material {
		name: "None",
		color: Color::RGB(0, 0, 0),
	},

	Material {
		name: "Oxygen",
		color: Color::RGB(155, 219, 245),
	},

	Material {
		name: "CarbonDioxide",
		color: Color::RGB(155, 219, 245),
	},

	Material {
		name: "Sand",
		color: Color::RGB(255, 255, 0),
	},

	Material {
		name: "Dirt",
		color: Color::RGB(255, 255, 0),
	},
];

type MatIndex = usize;

pub struct Sandbox {
	mats: Vec<Vec<MatIndex>>,
}

impl Sandbox {
	pub fn new(width: usize, height: usize) -> Sandbox {
		// reserve memory and initialize values
		let mut result = Sandbox {
			mats: Vec::new(),
		};

		result.mats = vec![vec![0; height]; width];
		
		return result;
	}
}

fn main() {
	// init
	const FRAME_DELTA: u32 = 1_000_000_000 / 30;
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
	let mut ts_now: std::time::Instant;
	let mut ts_draw = std::time::Instant::now();
	let mut sandbox = Sandbox::new(WIDTH, HEIGHT);

	let mut selected_mat: MatIndex = 0;

	'mainloop: loop {
		// time
		ts_now = std::time::Instant::now();
	
		// events
		for event in sdlsys_event.poll_iter() {
		
			match event {

				Event::KeyUp {keycode, ..} => {
					if keycode.is_some() == false {
						break;
					}
					
					match keycode.unwrap() {
						Keycode::Up => {
							if selected_mat > 0 {
								selected_mat -= 1;
							}
						},

						Keycode::Down => {
							if selected_mat < (MAT_COUNT - 1) {
								selected_mat += 1;
							}
						},

						_ => ()
					}
				}

				Event::MouseMotion {mousestate, x, y, ..} => {
					// if lmb is held down while mousemotion
					if mousestate.left() {
					
						// spawn selected material
						sandbox.mats[x as usize][y as usize] = selected_mat;
					}
				},

				Event::Quit {..} => break 'mainloop,
				
				_ => ()
			}
		}

		// draw
		if ts_now > (ts_draw + std::time::Duration::new(0, FRAME_DELTA)) {
			// pixels
			for x in 0..sandbox.mats.len() {
				for y in 0..sandbox.mats[0].len() {
					canvas.set_draw_color(MATERIALS[sandbox.mats[x][y]].color);
					canvas.draw_point(Point::new(x as i32, y as i32)).unwrap();
				}
			}
			
			canvas.present();

			// update ts
			ts_draw = ts_now;
		}
	}
}
