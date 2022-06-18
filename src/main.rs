use std::vec::Vec;

pub enum Material {
	None,
	Oxygen,
	CarbonDioxide,
	Sand,
	Dirt,
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

		result.mats.reserve_exact(width);

		for matvec in &mut result.mats {
			matvec.reserve_exact(height);

			for mat in matvec {
				*mat = Material::None;
			}
		}
		
		return result;
	}
}

fn main() {
	use sdl2::event::*;
	/*use sdl2::pixels::*;
	use sdl2::rect::*;*/

	// init
	let sdl = sdl2::init().unwrap();
	let mut sdlsys_event = sdl.event_pump().unwrap();
	let sdlsys_video = sdl.video().unwrap();

	let window = sdlsys_video.window("chemarium", 200, 200)
		.position_centered()
		.build()
		.unwrap();
		
	let mut canvas = window.into_canvas().build().unwrap();

	'mainloop: loop {
	
		// events
		for event in sdlsys_event.poll_iter() {
			match event {
				Event::MouseMotion {mousestate/*, x, y*/, ..} => {
					// if lmb is held down while mousemotion
					if mousestate.left() {

						// spawn selected material
					}
				},

				Event::Quit {..} => break 'mainloop,
				
				_ => ()
			}
		}

		// draw
		

		
		canvas.present();
	}
}
