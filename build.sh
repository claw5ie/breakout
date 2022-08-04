#! /bin/bash

set -eu

warning_flags="-Wall -Wextra -pedantic"
other_flags="-g -std=c++11"
libs="-lX11 -lGL -lGLEW"
files="src/main.cpp src/X11Window.cpp src/Vectors.cpp src/Utils.cpp"

if [ $# -ne 1 ]; then
    (set -x; g++ ${warning_flags} ${other_flags} ${files} ${libs})
elif [ $1 = "release" ]; then
    (set -x; g++ -O2 ${files} ${libs})
fi
