# HAWPS

Half assed wannabe physics simulator.  
The name is the game.  
I just want to have fun writing something dumb.  

![heating a metal bowl and releasing an oxygen-methane mix into it](demo.gif)

# Build dependencies

- `go install golang.org/x/tools/cmd/stringer`  

## desktop

- `sudo apt install libgtk-3-dev`  

## cross_platform

- `sudo apt install {libx11-dev,libxrandr-dev,libglx-dev,libgl-dev,libxcursor-dev,libxinerama-dev,libxi-dev,libxxf86vm-dev}`  
or  
`sudo dnf install {libX11-devel,libXrandr-devel,mesa-libGL-devel,libglvnd-glx,libXcursor-devel,libXinerama-devel,libXi-devel,libXxf86vm-devel}`  

# Build

`make`  
