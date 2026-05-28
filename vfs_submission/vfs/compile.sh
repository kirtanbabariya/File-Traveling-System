#!/bin/bash
# compile.sh — compiles the Time-Travelling File System
set -e
echo "Compiling Time-Travelling File System..."
g++ -std=c++17 -O2 -Wall -Wextra \
    main.cpp FileSystem.cpp \
    -o vfs
echo "Done. Run with: ./vfs"
