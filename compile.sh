#!/bin/bash

# Delete build and bin directories if exists
rm -rf ./build
rm -rf ./bin

# Create a directory named 'build' in your project root
mkdir build

# Navigate to the 'build' directory
cd build

# Generate the Makefile using CMake
cmake ..

# Build the executable using make
cmake --build .

# Change directory
cd ..

# Create a directory names 'bin' for executabled files
mkdir bin

# Copy files
cp ./build/boat-rudder ./bin/boat-rudder
# cp ./build/boat-rudder-ssl ./bin/boat-rudder-ssl
# Copy folders
cp -r ./ssl ./bin/ssl
cp -r ./configs ./bin/configs
cp -r ./src/html ./bin/html

# Delete build directory if exist
rm -rf ./build