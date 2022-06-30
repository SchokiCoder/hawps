use sdl2::keyboard::*;
use sdl2::event::*;
use sdl2::pixels::*;
use sdl2::surface::*;
use sdl2::render::*;
use sdl2::video::*;
use sdl2::rect::*;
use sdl2::ttf::*;

pub struct Material {
	name: &'static str,
	color: Color,
	weight: u32,
	melt_point: i16,
	boil_point: i16,
}

// game stuff
const STD_CURSOR_SIZE: i32 = 5;
const STD_TICK_PS: u32 = 30;
const TICK_CHANGE_STEP: u32 = 10;

// window stuff
const FRAME_DELTA: u32 = 1_000_000_000 / 30;
const WINDOW_W: u32 = 400;
const WINDOW_H: u32 = 400;

// world size
const SANDBOX_W: i32 = 200;
const SANDBOX_H: i32 = 200;

// hud
const FONT_SIZE: u16 = 24;

const HUD_BG_COLOR: Color = Color::RGB(50, 50, 50);

const HUD_CURSOR_COLOR: Color = Color::RGB(255, 0, 255);

const HUD_MATERIAL_X: i32 = SANDBOX_W + 10;
const HUD_MATERIAL_Y: i32 = 10;

const HUD_TICK_PS_X: i32 = HUD_MATERIAL_X;
const HUD_TICK_PS_Y: i32 = HUD_MATERIAL_Y + FONT_SIZE as i32 + 5;

// material properties
const MATERIALS: [Material; 14] = [
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

type MatIndex = usize;

struct Sprite<'a> {
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

#[derive(Copy, Clone)]
enum AggregateState {
	Solid,
	Liquid,
	Gas,
}

struct Sandbox {
	width: usize,
	height: usize,
	
	pub material: Vec<Vec<MatIndex>>,
	pub moved: Vec<Vec<bool>>,
	pub temperature: Vec<Vec<i16>>,	// celsius
	pub state: Vec<Vec<AggregateState>>,
}

impl Sandbox {
	pub fn new(width: usize, height: usize) -> Sandbox {
		return Sandbox {
			width: width,
			height: height,
			material: vec![vec![0; height]; width],
			moved: vec![vec![false; height]; width],
			temperature: vec![vec![20; height]; width],
			state: vec![vec![AggregateState::Solid; height]; width],
		};
	}

	pub fn w(&self) -> usize {
		return self.width;
	}

	pub fn h(&self) -> usize {
		return self.height;
	}
}

fn main() {	
	let window_rect = Rect::new(0, 0, WINDOW_W, WINDOW_H);
	
	let mut rect_draw = Rect::new(0, 0, 0, 0);

	let mut tick_ps = STD_TICK_PS;
	let mut tick_delta = 1_000_000_000 / tick_ps;

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

	let font = ttf.load_font(
		"/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
		FONT_SIZE).unwrap();

	// text textures
	let mut spr_tick_ps = Sprite::new(&font, &texture_creator);
	spr_tick_ps.text(tick_ps.to_string().as_str());
	
	let mut spr_mat_names: [Sprite; MATERIALS.len()] = [
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
		Sprite::new(&font, &texture_creator),
	];

	for i in 0..MATERIALS.len() {
		spr_mat_names[i].text(MATERIALS[i].name);
	}

	// mainloop
	let mut ts_now: std::time::Instant;
	let mut ts_tick = std::time::Instant::now();
	let mut ts_draw = std::time::Instant::now();

	let mut selected_mat: MatIndex = 0;
	let mut cursor = Point::new(0, 0);
	let mut cursor_speed: i32 = 3;
	let mut cursor_size: i32 = STD_CURSOR_SIZE;

	let mut sandbox = Sandbox::new(SANDBOX_W as usize, SANDBOX_H as usize);

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
						Keycode::N => {
							if selected_mat > 0 {
								selected_mat -= 1;
							}
						},

						Keycode::M => {
							if selected_mat < (MATERIALS.len() - 1) {
								selected_mat += 1;
							}
						},

						Keycode::K => {
							if tick_ps > TICK_CHANGE_STEP {
								tick_ps -= TICK_CHANGE_STEP;
								tick_delta = 1_000_000_000 / tick_ps;
								spr_tick_ps.text(tick_ps.to_string().as_str());
							}
						},

						Keycode::L => {
							tick_ps += TICK_CHANGE_STEP;
							tick_delta = 1_000_000_000 / tick_ps;
							spr_tick_ps.text(tick_ps.to_string().as_str());
						},

						Keycode::Q => {
							if cursor_speed > 1 {
								cursor_speed -= 1;
							}
						},
						
						Keycode::E => {
							if cursor_speed < i32::MAX {
								cursor_speed += 1;
							}
						},

						Keycode::V => {
							if cursor_size > 1 {
								cursor_size -= 1;
							}
						},
						
						Keycode::B => {
							if cursor_size < i32::MAX {
								cursor_size += 1;
							}
						},

						_ => ()
					}
				}

				Event::KeyDown {keycode, ..} => {
					if keycode.is_some() == false {
						break;
					}

					match keycode.unwrap() {
						Keycode::W => {
							cursor.y -= cursor_speed;

							if cursor.y < 0 {
								cursor.y = 0;
							}
						},
						
						Keycode::A => {
							cursor.x -= cursor_speed;

							if cursor.x < 0 {
								cursor.x = 0;
							}
						},
						
						Keycode::S => {
							cursor.y += cursor_speed;

							if cursor.y > sandbox.h() as i32 - 1 {
								cursor.y = sandbox.h() as i32 - 1;
							}
						},
						
						Keycode::D => {
							cursor.x += cursor_speed;

							if cursor.x > sandbox.w() as i32 - 1 {
								cursor.x = sandbox.w() as i32 - 1;
							}
						},

						Keycode::Space => {
							// calculate spawn area from cursor and cursor size
							let mut x1 = cursor.x - cursor_size / 2;

							if x1 < 0 {
								x1 = 0;
							}
							
							let x1 = x1 as usize;
							
							let mut x2 = cursor.x + cursor_size / 2;

							if x2 > sandbox.w() as i32 - 1 {
								x2 = sandbox.w() as i32 - 1;
							}
							
							let x2 = x2 as usize;

							let mut y1 = cursor.y - cursor_size / 2;

							if y1 < 0 {
								y1 = 0;
							}
							
							let y1 = y1 as usize;
							
							let mut y2 = cursor.y + cursor_size / 2;

							if y2 > sandbox.h() as i32 - 1 {
								y2 = sandbox.h() as i32 - 1;
							}
							
							let y2 = y2 as usize;

							// spawn selected material
							for x in x1..x2 {
								for y in y1..y2 {
									sandbox.material[x][y] = selected_mat;
								}
							}
						},

						_ => ()
					}
				},

				/*Event::MouseMotion {mousestate, x, y, ..} => {
					// if lmb is held down while mousemotion
					if mousestate.left() {
					
						// spawn selected material
						sandbox_mat[x as usize][y as usize] = selected_mat;
					}
				},*/

				Event::Quit {..} => break 'mainloop,
				
				_ => ()
			}
		}

		// physics tick
		if ts_now > (ts_tick + std::time::Duration::new(0, tick_delta)) {

			// temperature
			for x in 0..sandbox.w() {
				
				for y in 0..sandbox.h() {

					// skip, material none
					if sandbox.material[x][y] == 0 {
						continue;
					}

				}
			}

			// aggregate state flag
			for x in 0..sandbox.w() {
				for y in 0..sandbox.h() {
					if sandbox.temperature[x][y] > MATERIALS[sandbox.material[x][y]].boil_point {
						sandbox.state[x][y] = AggregateState::Gas;
					}

					else if sandbox.temperature[x][y] > MATERIALS[sandbox.material[x][y]].melt_point {
						sandbox.state[x][y] = AggregateState::Liquid;
					}

					else {
						sandbox.state[x][y] = AggregateState::Solid;
					}
				}
			}

			// gravity
			for x in 0..sandbox.w() {
				for y in 0..sandbox.h() - 1 {
				
					if sandbox.moved[x][y] == false {
						if MATERIALS[sandbox.material[x][y]].weight >
							MATERIALS[sandbox.material[x][y + 1]].weight {
							
							let temp = sandbox.material[x][y];
							sandbox.material[x][y] = sandbox.material[x][y + 1];
							sandbox.material[x][y + 1] = temp;

							sandbox.moved[x][y] = true;
							sandbox.moved[x][y + 1] = true;
						}
					}
				}
			}

			// update ts
			ts_tick = ts_now;

			// reset moved flags
			for x in 0..SANDBOX_W as usize {
				for y in 0..SANDBOX_H as usize {
					sandbox.moved[x][y] = false;
				}
			}
		}

		// draw
		if ts_now > (ts_draw + std::time::Duration::new(0, FRAME_DELTA)) {
			// bg
			canvas.set_draw_color(HUD_BG_COLOR);
			canvas.fill_rect(window_rect).unwrap();

			// hud
			rect_draw.x = HUD_MATERIAL_X as i32;
			rect_draw.y = HUD_MATERIAL_Y as i32;
			rect_draw.w = spr_mat_names[selected_mat].rect().w;
			rect_draw.h = spr_mat_names[selected_mat].rect().h;
			
			spr_mat_names[selected_mat].draw(&mut canvas, rect_draw);

			rect_draw.x = HUD_TICK_PS_X as i32;
			rect_draw.y = HUD_TICK_PS_Y as i32;
			rect_draw.w = spr_tick_ps.rect().w;
			rect_draw.h = spr_tick_ps.rect().h;
			
			spr_tick_ps.draw(&mut canvas, rect_draw);

			// pixels		
			for x in 0..SANDBOX_W as usize {
				for y in 0..SANDBOX_H as usize {
					canvas.set_draw_color(MATERIALS[sandbox.material[x][y]].color);
					canvas.draw_point(Point::new(x as i32, y as i32)).unwrap();
				}
			}

			// cursor
			let x1 = Point::new(cursor.x - cursor_size as i32 / 2, cursor.y);
			let x2 = Point::new(cursor.x + cursor_size as i32 / 2, cursor.y);
			let y1 = Point::new(cursor.x, cursor.y - cursor_size as i32 / 2);
			let y2 = Point::new(cursor.x, cursor.y + cursor_size as i32 / 2);

			canvas.set_draw_color(HUD_CURSOR_COLOR);
			canvas.draw_line(x1, x2).unwrap();
			canvas.draw_line(y1, y2).unwrap();
			
			canvas.present();

			// update ts
			ts_draw = ts_now;
		}
	}
}
