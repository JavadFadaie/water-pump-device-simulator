cmake_minimum_required(VERSION 3.10)

# Project name and C++ standard
project(PumpSimulation LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify the source files
set(SOURCES
    main.cpp
)

# Specify the header files (optional, for IDEs to recognize them)
set(HEADERS
    simulation_variable.hpp
    driver_base.hpp
    grundfos_pump.hpp
)

# Create the executable
add_executable(pump_simulation ${SOURCES} ${HEADERS})

# Find and link the threading library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(pump_simulation PRIVATE Threads::Threads)

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)