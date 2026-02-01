#!/bin/sh
# Compile the minimal triangle test with Metal + SDL3
clang -fobjc-arc -O2 test/triangle_minimal.m \
    -I subprojects/sokol \
    -I/opt/homebrew/include \
    -framework Metal \
    -framework QuartzCore \
    -framework Foundation \
    -framework Cocoa \
    -L/opt/homebrew/lib \
    -Wl,-rpath,/opt/homebrew/lib \
    -lSDL3 \
    -o test/triangle_minimal
