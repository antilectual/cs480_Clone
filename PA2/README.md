# PA2: Spinning Cube w/ Keyboard Interaction

# Dependencies, Building, and Running

## Dependency Instructions
For both of the operating systems to run this project installation of these three programs are required [GLEW](http://glew.sourceforge.net/), [GLM](http://glm.g-truc.net/0.9.7/index.html), and [SDL2](https://wiki.libsdl.org/Tutorials).

This project uses OpenGL 3.3. Some computers, such as virtual machines in the ECC, can not run this version. In in order to run OpenGL 2.7 follow the instructions at [Using OpenGL 2.7](https://github.com/HPC-Vis/computer-graphics/wiki/Using-OpenGL-2.7)

### Ubuntu/Linux
```bash
sudo apt-get install libglew-dev libglm-dev libsdl2-dev
```

### Mac OSX
Installation of brew is suggested to easily install the libs. Ensure that the latest version of the Developer Tools is installed.
```bash
brew install glew glm sdl2
```

## Building and Running

### CMake Instructions
The building of the project is done using CMake, installation with apt-get or brew may be necessary. Later use with CMake and Shader files will be require the copy of a directory where those files are stored (ex. shaders). To do this in the ```add_custom_target``` function place

```bash
mkdir build
cd build
cmake ..
make
./Tutorial
```

# Controls

`up arrow` - Decrease orbit radius  
`down arrow` - Increase orbit radius  
`left arrow` - Slow down  
`right arrow` - Speed up  
`r` - Reverse  
`s` - Stop
`esc` - Quit

`left click` - Reverse
`right click` - Stop

Hold `shift` to only affect the circular movement  
Hold `ctrl` to only affect the spinning movement

# Command Line Parameters

## Command Usage

`Tutorial --help` - Pull up the help menu / command usage  
`Tutorial [options] <-f filename> <-v filename>` - Run the program with the given vertex and fragment shaders

## Options

```
    --help                       Show help menu and command usage
    -v, --vertex <filename>      Specify where to load vertex shader from
    -f, --fragment <filename>    Specify where to load fragment shader from
    -h, --height <number>        Choose the window height
    -w, --width <number>         Choose the window width
    -n, --name <name>            Choose the window name
```