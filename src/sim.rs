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

use sdl2::event::*;
use sdl2::rect::*;
use materials::*;

pub fn main_sdl(win_w: u32, win_h: u32, cin: std::sync::mpsc::Receiver<Command>) {
	// init sdl
	let sdl = sdl2::init().unwrap();
	let mut sdlsys_event = sdl.event_pump().unwrap();
	let sdlsys_video = sdl.video().unwrap();

	let window = sdlsys_video.window(env!("CARGO_PKG_NAME"), win_w, win_h)
		.position_centered()
		.build()
		.unwrap();

	let mut canvas = window.into_canvas().build().unwrap();

	// mainloop
	let mut sandbox = Sandbox::new(win_w as usize, win_w as usize);

	'mainloop: loop {
		// wait for a command
		'passwait: loop {
		let cmd = cin.recv();
		
			if cmd.is_ok() {
			
				match cmd.unwrap() {
					// pass time
					Command::Pass(_) => {
						break 'passwait;
					},
				}
			}
		}
	
		// events
		for event in sdlsys_event.poll_iter() {
		
			match event {
				Event::Quit {..} => break 'mainloop,
				
				_ => ()
			}
		}

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

		// reset moved flags
		for x in 0..sandbox.w() as usize {
			for y in 0..sandbox.h() as usize {
				sandbox.moved[x][y] = false;
			}
		}

		// draw pixels		
		for x in 0..sandbox.w() {
			for y in 0..sandbox.h() {
				canvas.set_draw_color(MATERIALS[sandbox.material[x][y]].color);
				canvas.draw_point(Point::new(x as i32, y as i32)).unwrap();
			}
		}
		
		canvas.present();
	}	
}
