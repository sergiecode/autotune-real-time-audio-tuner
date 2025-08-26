# Build Instructions for AutoTune Real-Time Audio Engine

This document provides detailed instructions for building the project on different platforms.

## Prerequisites

### Windows
1. **Visual Studio 2019 or later** with C++ development tools
2. **CMake 3.16+** - Download from https://cmake.org/download/
3. **Git** (optional) - For cloning dependencies

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

### macOS
```bash
# Install Xcode command line tools
xcode-select --install

# Install CMake (using Homebrew)
brew install cmake
```

## Building the Project

### Method 1: Using CMake (Recommended)

#### Windows
```powershell
# Open Command Prompt or PowerShell
cd path\to\autotune-real-time-audio-tuner
mkdir build
cd build

# Configure (Visual Studio)
cmake .. -G "Visual Studio 16 2019" -A x64

# Build
cmake --build . --config Release

# Run example
.\Release\autotune_example.exe
```

#### Linux/macOS
```bash
cd autotune-real-time-audio-tuner
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Run example
./autotune_example
```

### Method 2: Manual Compilation (Fallback)

If CMake is not available, you can compile manually:

#### Windows (MSVC)
```cmd
cl /EHsc /std:c++17 /I"include" src/*.cpp examples/main.cpp /Fe:autotune_example.exe
```

#### Linux/macOS (GCC/Clang)
```bash
g++ -std=c++17 -Iinclude -O3 src/*.cpp examples/main.cpp -o autotune_example
```

## Build Options

### Standard Options
- `CMAKE_BUILD_TYPE`: Debug, Release, RelWithDebInfo, MinSizeRel
- `BUILD_TESTS`: ON/OFF (default: ON)
- `BUILD_PYTHON_BINDINGS`: ON/OFF (default: OFF)

### Example with Options
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_PYTHON_BINDINGS=ON
```

## Python Bindings

### Prerequisites
```bash
pip install pybind11 numpy
```

### Building
```bash
cmake .. -DBUILD_PYTHON_BINDINGS=ON
make
```

### Testing Python Bindings
```python
import sys
sys.path.append('path/to/build/directory')
import pyautotune
print("Python bindings work!")
```

## Troubleshooting

### Common Issues

1. **CMake not found**
   - Install CMake from https://cmake.org/download/
   - Add CMake to your system PATH

2. **Compiler not found**
   - Windows: Install Visual Studio with C++ tools
   - Linux: `sudo apt-get install build-essential`
   - macOS: `xcode-select --install`

3. **PyBind11 not found**
   - Install: `pip install pybind11`
   - Or clone to extern/pybind11/

4. **Missing headers**
   - Ensure all source files are present
   - Check include paths in CMakeLists.txt

### Performance Optimization

For maximum performance:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"
```

### Debug Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## IDE Integration

### Visual Studio Code
1. Install C++ extension
2. Open project folder
3. Configure IntelliSense with CMake

### Visual Studio
```powershell
cmake .. -G "Visual Studio 16 2019"
# Open generated .sln file
```

### CLion
1. Open project folder
2. CLion will automatically detect CMake configuration

## Testing

```bash
# Run all tests
./autotune_tests

# Run specific tests
./autotune_tests audio_buffer
./autotune_tests pitch_detector
```

## Installation

```bash
# Install to system (optional)
sudo make install

# Or specify custom prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make install
```

---

## Next Steps

1. Build the project successfully
2. Run the example application
3. Integrate with your audio application
4. Explore Python bindings for ML workflows

For support, please check the main README.md or contact the development team.
