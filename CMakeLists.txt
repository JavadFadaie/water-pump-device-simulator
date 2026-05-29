cmake_minimum_required(VERSION 3.14)

# Project name and C++ standard
project(PumpSimulation LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch cpp-httplib
include(FetchContent)

FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Specify the source files
set(SOURCES
    main.cpp
    simulation_kernel/grundfos_pump_register.cpp
    simulation_kernel/wilo_pump_register.cpp
)

# Specify the header files (for IDEs to recognize them)
set(HEADERS
    include/simulation_variable.hpp
    include/driver_base.hpp
    include/driver_registry.hpp
    include/pump_factory.hpp
    include/web_server.hpp
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
target_link_libraries(pump_simulation PRIVATE
    Threads::Threads
    httplib::httplib
    nlohmann_json::nlohmann_json
)

# Copy static web files to build directory
add_custom_command(TARGET pump_simulation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/static
        ${CMAKE_BINARY_DIR}/static
)

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
