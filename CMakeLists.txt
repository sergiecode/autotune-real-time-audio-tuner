cmake_minimum_required(VERSION 3.16)
project(AutotuneRealTimeEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release for performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific optimizations for low latency
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(AUTOTUNE_SOURCES
    src/pitch_detector.cpp
    src/pitch_corrector.cpp
    src/audio_buffer.cpp
    src/quantizer.cpp
    src/autotune_engine.cpp
)

# Header files
set(AUTOTUNE_HEADERS
    include/pitch_detector.h
    include/pitch_corrector.h
    include/audio_buffer.h
    include/quantizer.h
    include/autotune_engine.h
    include/audio_types.h
)

# Create static library
add_library(autotune_engine STATIC ${AUTOTUNE_SOURCES} ${AUTOTUNE_HEADERS})

# Create shared library (for Python bindings)
add_library(autotune_engine_shared SHARED ${AUTOTUNE_SOURCES} ${AUTOTUNE_HEADERS})
set_target_properties(autotune_engine_shared PROPERTIES OUTPUT_NAME autotune_engine)

# Example executable
add_executable(autotune_example examples/main.cpp)
target_link_libraries(autotune_example autotune_engine)

# Optional: Pybind11 Python bindings
option(BUILD_PYTHON_BINDINGS "Build Python bindings using pybind11" OFF)

if(BUILD_PYTHON_BINDINGS)
    # Check if pybind11 is available
    find_package(pybind11 QUIET)
    
    if(pybind11_FOUND)
        pybind11_add_module(pyautotune src/python_bindings.cpp)
        target_link_libraries(pyautotune PRIVATE autotune_engine)
        
        # Compiler-specific flags for pybind11
        target_compile_definitions(pyautotune PRIVATE VERSION_INFO="${PROJECT_VERSION}")
    else()
        message(WARNING "pybind11 not found. Python bindings will not be built.")
        message(STATUS "To build Python bindings, install pybind11 or add it to extern/pybind11/")
    endif()
endif()

# Testing
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS autotune_engine autotune_engine_shared autotune_example
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(FILES ${AUTOTUNE_HEADERS} DESTINATION include/autotune)

# Print configuration summary
message(STATUS "")
message(STATUS "AutoTune Real-Time Engine Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Python bindings: ${BUILD_PYTHON_BINDINGS}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "")
