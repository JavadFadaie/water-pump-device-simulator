#!/bin/bash

# Exit on any error
set -e

# Define the build directory
BUILD_DIR="build"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
    echo "Created build directory: $BUILD_DIR"
fi

# Navigate to the build directory
cd "$BUILD_DIR"

# Run CMake to generate build files
echo "Running cmake .."
cmake ..

# Build the project
echo "Running cmake --build ."
cmake --build .

# Notify user of the executable location
echo "Build completed. Executable is located at: $BUILD_DIR/bin/pump_simulation"

# Run the executable
echo "Running the executable..."
./pump_simulation