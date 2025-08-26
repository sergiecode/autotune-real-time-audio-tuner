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

# Find ONNX Runtime (optional for ML model support)
option(ENABLE_ML_MODELS "Enable ML model integration" ON)

if(ENABLE_ML_MODELS)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(ONNXRUNTIME onnxruntime)
        if(ONNXRUNTIME_FOUND)
            add_definitions(-DHAVE_ONNXRUNTIME)
            include_directories(${ONNXRUNTIME_INCLUDE_DIRS})
            link_directories(${ONNXRUNTIME_LIBRARY_DIRS})
            message(STATUS "ONNX Runtime found - ML model support enabled")
            set(HAVE_ONNX_RUNTIME TRUE)
        else()
            message(STATUS "ONNX Runtime not found via pkg-config")
        endif()
    endif()
    
    # Windows fallback - look for ONNX Runtime manually
    if(NOT HAVE_ONNX_RUNTIME)
        find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h
            PATHS "C:/Program Files/onnxruntime/include"
                  "C:/onnxruntime/include"
                  "$ENV{ONNXRUNTIME_ROOT}/include"
                  "${CMAKE_SOURCE_DIR}/extern/onnxruntime/include")
        
        find_library(ONNXRUNTIME_LIB onnxruntime
            PATHS "C:/Program Files/onnxruntime/lib"
                  "C:/onnxruntime/lib" 
                  "$ENV{ONNXRUNTIME_ROOT}/lib"
                  "${CMAKE_SOURCE_DIR}/extern/onnxruntime/lib")
        
        if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIB)
            add_definitions(-DHAVE_ONNXRUNTIME)
            include_directories(${ONNXRUNTIME_INCLUDE_DIR})
            set(ONNXRUNTIME_LIBRARIES ${ONNXRUNTIME_LIB})
            message(STATUS "ONNX Runtime found manually - ML model support enabled")
            set(HAVE_ONNX_RUNTIME TRUE)
        else()
            message(STATUS "ONNX Runtime not found - using traditional processing only")
        endif()
    endif()
    
    if(HAVE_ONNX_RUNTIME)
        add_definitions(-DENABLE_ML_MODELS)
        message(STATUS "ML model integration enabled")
    endif()
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
    src/ml_model_loader.cpp
)

# Header files
set(AUTOTUNE_HEADERS
    include/pitch_detector.h
    include/pitch_corrector.h
    include/audio_buffer.h
    include/quantizer.h
    include/autotune_engine.h
    include/audio_types.h
    include/ml_model_loader.h
)

# Create static library
add_library(autotune_engine STATIC ${AUTOTUNE_SOURCES} ${AUTOTUNE_HEADERS})

# Link ONNX Runtime if available
if(HAVE_ONNX_RUNTIME)
    target_link_libraries(autotune_engine ${ONNXRUNTIME_LIBRARIES})
endif()

# Create shared library (for Python bindings)
add_library(autotune_engine_shared SHARED ${AUTOTUNE_SOURCES} ${AUTOTUNE_HEADERS})
set_target_properties(autotune_engine_shared PROPERTIES OUTPUT_NAME autotune_engine)

# Link ONNX Runtime to shared library too
if(HAVE_ONNX_RUNTIME)
    target_link_libraries(autotune_engine_shared ${ONNXRUNTIME_LIBRARIES})
endif()

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
if(ENABLE_ML_MODELS)
    message(STATUS "  ML model support: ${ENABLE_ML_MODELS}")
    if(HAVE_ONNX_RUNTIME)
        message(STATUS "  ONNX Runtime: Found - ML models enabled")
    else()
        message(STATUS "  ONNX Runtime: Not found - traditional processing only")
    endif()
else()
    message(STATUS "  ML model support: Disabled")
endif()
message(STATUS "")
