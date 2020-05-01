#!/bin/bash
clear
BUILD_DIR=./build
COMPILATION_UNITS=./main.c
EX_NAME=screenshot
C_VERSION=-std=c99
#getimage
#screenshot

if [ ! -d ./build ]; then
    echo "Creating $BUILD_DIR"
    mkdir $BUILD_DIR
fi

echo "Building..."
#
gcc -m64 -g $COMPILATION_UNITS $C_VERSION -Wall -Wextra -Wno-missing-braces -Wno-unused-function -lm -lXinerama -lXrandr -lXext -lX11 -o "$BUILD_DIR"/"$EX_NAME"
EC=$?

[ $EC -eq 0 ] && echo "Build succesfull" || echo "Build failed"
