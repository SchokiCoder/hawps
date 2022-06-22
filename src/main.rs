use std::vec::Vec;
use sdl2::keyboard::*;
use sdl2::event::*;
use sdl2::pixels::*;
use sdl2::surface::*;
use sdl2::render::*;
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
		color: Color::RGB(37, 25, 13),
	},
];

type MatIndex = usize;

pub struct Sandbox {
	mats: Vec<Vec<MatIndex>>,
}

impl Sandbox {
	pub fn new(width: u32, height: u32) -> Sandbox {
		// reserve memory and initialize values
		let mut result = Sandbox {
			mats: Vec::new(),
		};

		result.mats = vec![vec![0; height as usize]; width as usize];
		
		return result;
	}
}

fn main() {
	const FRAME_DELTA: u32 = 1_000_000_000 / 30;
	const WINDOW_W: u32 = 400;
	const WINDOW_H: u32 = 400;
	let window_rect = Rect::new(0, 0, WINDOW_W, WINDOW_H);

	const SANDBOX_X: u32 = 200;
	const SANDBOX_Y: u32 = 200;
	const SANDBOX_W: u32 = 200;
	const SANDBOX_H: u32 = 200;

	const HUD_MATERIAL_X: u32 = 10;
	const HUD_MATERIAL_Y: u32 = 10;
	let mut rect_selected_mat = Rect::new(
		HUD_MATERIAL_X as i32,
		HUD_MATERIAL_Y as i32,
		100,
		100);

	// init sdl
	let sdl = sdl2::init().unwrap();
	let mut sdlsys_event = sdl.event_pump().unwrap();
	let sdlsys_video = sdl.video().unwrap();

	let window = sdlsys_video.window(env!("CARGO_PKG_NAME"), WINDOW_W, WINDOW_H)
		.position_centered()
		.build()
		.unwrap();

	let mut canvas = window.into_canvas().build().unwrap();
	let texture_creator = canvas.texture_creator();

	// init ttf
	let ttf = sdl2::ttf::init().unwrap();

	let font = ttf.load_font("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 24).unwrap();

	// text textures	
	let mat_surfaces: [Surface; MAT_COUNT];
	let mat_textures: [Texture; MAT_COUNT];

	for i in 0..MAT_COUNT {
		mat_surfaces[i] = font
			.render(MATERIALS[i].name)
			.solid(Color::RGB(255, 255, 255))
			.unwrap();
				
		mat_textures[i] = mat_surfaces[i]
			.as_texture(&texture_creator)
			.unwrap();
	}

	// mainloop
	let mut ts_now: std::time::Instant;
	let mut ts_draw = std::time::Instant::now();
	let mut sandbox = Sandbox::new(SANDBOX_W, SANDBOX_H);

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
			// bg
			canvas.set_draw_color(Color::RGB(0, 0, 0));
			canvas.fill_rect(window_rect).unwrap();

			// hud
			rect_selected_mat.w = mat_surfaces[selected_mat].rect().w;
			rect_selected_mat.h = mat_surfaces[selected_mat].rect().h;
			
			canvas
				.copy(&mat_textures[selected_mat], None, Some(rect_selected_mat))
				.unwrap();

			// pixels
			let mut dx: i32 = SANDBOX_X as i32;
			let mut dy: i32 = SANDBOX_Y as i32;
			
			for x in 0..SANDBOX_W as usize {
				for y in 0..SANDBOX_H as usize {
					canvas.set_draw_color(MATERIALS[sandbox.mats[x][y]].color);
					canvas.draw_point(Point::new(dx, dy)).unwrap();

					dy += 1;
				}
				
				dx += 1;
				dy = SANDBOX_Y as i32;
			}
			
			canvas.present();

			// update ts
			ts_draw = ts_now;
		}
	}
}
