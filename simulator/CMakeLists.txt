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

# Find and link libmodbus
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)
include_directories(${LIBMODBUS_INCLUDE_DIRS})
link_directories(${LIBMODBUS_LIBRARY_DIRS})

# Specify the source files
set(SOURCES
    main.cpp
    simulation_kernel/grundfos_pump.cpp
    simulation_kernel/wilo_pump.cpp
    modbus/modbus_server.cpp
)

# Specify the header files (for IDEs to recognize them)
set(HEADERS
    include/simulation_variable.hpp
    include/driver_base.hpp
    include/driver_registry.hpp
    include/web_server.hpp
    include/model_info.hpp
    modbus/modbus_types.hpp
    modbus/protocol_writer.hpp
    modbus/modbus_core.hpp
    modbus/channel_device.hpp
    modbus/modbus_server.hpp
    simulation_kernel/grundfos_pump.hpp
    simulation_kernel/wilo_pump.hpp
)

# Create the executable
add_executable(pump_simulation ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(pump_simulation PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/simulation_kernel
    ${CMAKE_SOURCE_DIR}/modbus
)

# Find and link the threading library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(pump_simulation PRIVATE
    Threads::Threads
    httplib::httplib
    nlohmann_json::nlohmann_json
    ${LIBMODBUS_LIBRARIES}
)

# Copy static web files to build directory
add_custom_command(TARGET pump_simulation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/static
        ${CMAKE_BINARY_DIR}/static
)

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Unit Tests ---
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(pump_tests
        tests/test_modbus_core.cpp
        tests/test_channel_device.cpp
        tests/test_driver_registry.cpp
        tests/test_pump_drivers.cpp
        simulation_kernel/grundfos_pump.cpp
        simulation_kernel/wilo_pump.cpp
        modbus/modbus_server.cpp
    )
    target_include_directories(pump_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/simulation_kernel
        ${CMAKE_SOURCE_DIR}/modbus
    )
    target_link_libraries(pump_tests PRIVATE
        GTest::gtest_main
        Threads::Threads
        ${LIBMODBUS_LIBRARIES}
    )
    include(GoogleTest)
    gtest_discover_tests(pump_tests)
endif()
