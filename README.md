# Building project

## setup

Source: https://mesonbuild.com

After cloning this project, please execute these commands at the root of the project (directory `dynamic_program_analyser`).

To setup the build directory:
```
meson setup build
```

## compiling & running

To compile the program:
```
meson compile -C build
```
To run the program:
```
./build/tools/dpa
```
