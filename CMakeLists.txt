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
    include/simulation_variable.hpp
    include/driver_base.hpp
    include/pump_factory.hpp
    #include/menu_helper.hpp
    include/driver_registery.hpp
    include/driverId.hpp
    simulation_kernel/grundfos_pump.hpp
    simulation_kernel/wilo_pump.hpp
)


# Create the executable
add_executable(pump_simulation ${SOURCES} ${HEADERS})


# Add include directories
target_include_directories(pump_simulation PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/simulation_kernel
)

# Find and link the threading library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(pump_simulation PRIVATE Threads::Threads)

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)