# PyBind11 Placeholder

This directory is a placeholder for the PyBind11 library, which is used to create Python bindings for the C++ AutoTune engine.

## Installation Instructions

To use Python bindings, you need to install PyBind11. Here are several methods:

### Method 1: Git Submodule (Recommended)
```bash
cd extern
git clone https://github.com/pybind/pybind11.git
```

### Method 2: Package Manager
```bash
# Using pip
pip install pybind11

# Using conda
conda install -c conda-forge pybind11

# Using vcpkg (Windows)
vcpkg install pybind11
```

### Method 3: System Package Manager
```bash
# Ubuntu/Debian
sudo apt-get install pybind11-dev

# macOS with Homebrew
brew install pybind11
```

## Building Python Bindings

Once PyBind11 is installed, enable Python bindings in CMake:

```bash
mkdir build
cd build
cmake .. -DBUILD_PYTHON_BINDINGS=ON
make
```

## Usage Example

```python
import pyautotune
import numpy as np

# Create engine
engine = pyautotune.AutotuneEngine(44100, 512, 2)

# Configure for C Major scale
engine.set_scale(pyautotune.Scale.MAJOR, 60)
engine.set_mode(pyautotune.Mode.FULL_AUTOTUNE)

# Process audio
audio_data = np.random.randn(1024, 2).astype(np.float32)
output, result = engine.process_numpy(audio_data)

print(f"Detected pitch: {result.detected_pitch} Hz")
print(f"Corrected pitch: {result.corrected_pitch} Hz")
```

## ML Integration

The Python bindings are designed to facilitate machine learning workflows:

1. **Data Collection**: Use the engine to collect training data
2. **Feature Extraction**: Extract pitch and spectral features
3. **Model Training**: Train ML models for enhanced pitch detection
4. **Real-time Inference**: Deploy trained models back to the engine

## Next Steps

1. Install PyBind11 using one of the methods above
2. Build the project with Python bindings enabled
3. Test the bindings with the provided examples
4. Integrate with your ML workflow
