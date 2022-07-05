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

mod materials;
use materials::*;
mod sim;
use sim::*;

use std::io::prelude::*;

/*
-- commands --

quits:
exit
quit

passes time:
pass microseconds

sets spawnsize:
editsize size

spawns materials:
spawn mat x y             [size]
          (of upper left) (if not given use editsize)

sets temperature:
temp celsius x y [size]
                 (if not given use editsize)
*/

type MatIndex = usize;

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

enum Command {
	Pass(u32),
}

// because disabling the buffer is not a choice
// who is laughing now?
fn pnow(text: &str) {
	print!("{}", text);
	std::io::stdout().flush().expect("Stdout flush failed.");
}

fn main() {
	// parse args
	let args: Vec<String> = std::env::args().collect();

	if args.len() < 3 {
		println!("Not enough arguments given.");
		return;
	}

	let win_w = args[1]
		.parse::<u32>()
		.expect("Could not parse width.");
		
	let win_h = args[2]
		.parse::<u32>()
		.expect("Could not parse height.");

	// start sdl thread
	let (cout, cin) = std::sync::mpsc::channel();
	let _thread = std::thread::spawn(move || {main_sdl(win_w, win_h, cin)});

	// start control loop
	'mainloop: loop {
		let stdin = std::io::stdin();
		let mut input_raw = String::new();

		// get input
		pnow(" > ");
		stdin.read_line(&mut input_raw).expect("Could not read console input.");

		let input: Vec<&str> = input_raw.trim().split(' ').collect();

		// parse
		match input[0] {
			"exit" | "quit" => {
				pnow("Exiting.");
				break 'mainloop;
			},

			"pass" => {
				if input.len() < 2 {
					println!("Not enough arguments given.");
					continue;
				}

				let time = input[1].parse::<u32>();

				if time.is_ok() == false {
					println!("Given arguments do not fit the criteria.");
					continue;
				}
				
				cout
					.send(Command::Pass(time.unwrap()))
					.expect("Command could not be send to thread.");
			},

			_ => {
				println!("Command not recognized.");
			}
		}
	}

	println!("");
}
