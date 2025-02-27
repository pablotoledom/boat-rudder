#!/bin/bash
source ./show/welcome

# Get user in session and say hello
user="$(whoami)" 
echo "Hi ${user}!!"

source ./show/divbar
echo '1- Cleaning directories...'
sleep .5

# Delete build and bin directories if exists
rm -rf ./build
rm -rf ./bin

source ./show/divbar
echo '2- Compiling...'
sleep .5

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

source ./show/divbar
echo '3- Copying binaries, HTML content and configs...'
sleep .5

# Copy files
cp ./build/boat-rudder ./bin/boat-rudder
# cp ./build/boat-rudder-ssl ./bin/boat-rudder-ssl
# Copy folders
cp -r ./ssl ./bin/ssl
cp -r ./configs ./bin/configs
cp -r ./src/html ./bin/html

source ./show/divbar
echo '4- Cleaning directories...'
sleep .5

# Delete build directory if exist
rm -rf ./build

source ./show/divbar
echo 'Done!!'
sleep .5