# Trained ML Models Directory

This directory contains ONNX models exported from the `autotune-audio-ml-trainer` project.

## Model Files:

- `pitch_corrector.onnx` - Main pitch correction neural network
- `timing_adjuster.onnx` - Natural timing adjustment model (optional)  
- `enhanced_detector.onnx` - ML-enhanced pitch detector (optional)

## Usage:

Models are automatically loaded by the AutoTune engine if present. You can also manually load models:

```cpp
AutotuneEngine engine(44100, 512);
bool success = engine.load_ml_model("models/pitch_corrector.onnx");
engine.set_ml_processing_enabled(true);
```

## Model Requirements:

- **Input Format**: ONNX models must accept:
  - `audio_buffer`: [1, 512] float32 tensor
  - `target_pitch`: [1, 1] float32 tensor (optional)
  - `correction_strength`: [1, 1] float32 tensor (optional)

- **Output Format**: ONNX models must provide:
  - `corrected_audio`: [1, 512] float32 tensor
  - `confidence`: [1, 1] float32 tensor (optional)

- **Performance**: Models should process in <5ms for real-time use

## Integration:

Models from `autotune-audio-ml-trainer` are automatically compatible.
Use the deployment bridge to copy trained models here:

```python
# In autotune-audio-ml-trainer project
from src.export.cpp_deployment import CppDeploymentBridge

bridge = CppDeploymentBridge("../autotune-real-time-audio-tuner")
bridge.deploy_model("models/exported/my_model.onnx", "my_model", config)
```
