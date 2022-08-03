#! /bin/bash

set -xeu

warning_flags="-Wall -Wextra -pedantic"
other_flags="-g -std=c++11"
libs="-lX11 -lGL -lGLEW"
files="src/main.cpp src/X11Window.cpp"

g++ ${warning_flags} ${other_flags} ${files} ${libs}
