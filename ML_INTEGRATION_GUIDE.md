# AutoTune Engine ML Integration Guide

## Overview for AI Agent: autotune-audio-ml-trainer Repository Creation

This document provides technical specifications and integration requirements for creating the **autotune-audio-ml-trainer** Python framework. This companion repository will train neural networks for intelligent pitch correction and natural timing adjustments, with models designed to be deployed in the real-time C++ AutoTune engine.

---

## 🎯 ML Trainer Repository Objectives

### Primary Goals:
1. **Dataset Creation Pipeline** - Tools for preparing training data from audio files
2. **Neural Network Training** - Models for pitch correction and timing adjustment
3. **Model Export System** - Convert trained models for C++ engine deployment
4. **Evaluation Framework** - Metrics and validation tools for model performance
5. **Integration Bridge** - Seamless connection with the real-time C++ engine

### Technical Stack Required:
- **Python 3.8+** with PyTorch/TensorFlow for deep learning
- **Audio Processing** using Librosa, torchaudio, or similar
- **Pitch Detection** with CREPE, PYIN, or custom implementations
- **Data Management** with NumPy, Pandas for dataset handling
- **Model Export** to ONNX, TensorRT, or custom formats for C++ integration

---

## 🔗 Integration Architecture

### Data Flow:
```
Raw Audio Files → Dataset Creation → Model Training → Model Export → C++ Engine Deployment
      ↓                ↓               ↓             ↓                    ↓
   [Librosa]      [PyTorch/TF]    [Training Loop]  [ONNX/Custom]    [Real-time Processing]
```

### Key Integration Points:

#### 1. **Audio Format Compatibility**
```python
# The C++ engine expects this audio format:
SAMPLE_RATE = 44100  # Hz
BUFFER_SIZE = 512    # samples
CHANNELS = 1         # mono (can extend to stereo)
BIT_DEPTH = 32       # float32
```

#### 2. **Pitch Detection Standards**
```python
# Pitch detection range (compatible with C++ engine)
MIN_FREQUENCY = 80.0   # Hz (lowest guitar string)
MAX_FREQUENCY = 2000.0 # Hz (highest vocals)
CONFIDENCE_THRESHOLD = 0.7  # minimum confidence for valid detection
```

#### 3. **Musical Scale Integration**
```python
# Built-in scales in C++ engine (use same definitions)
SCALES = {
    'MAJOR': [0, 2, 4, 5, 7, 9, 11],
    'MINOR': [0, 2, 3, 5, 7, 8, 10],
    'BLUES': [0, 3, 5, 6, 7, 10],
    'PENTATONIC': [0, 2, 4, 7, 9],
    'DORIAN': [0, 2, 3, 5, 7, 9, 10],
    'MIXOLYDIAN': [0, 2, 4, 5, 7, 9, 10],
    'CHROMATIC': [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
}
```

---

## 📊 Dataset Creation Requirements

### Training Data Structure:
```python
# Expected dataset format for training
TrainingExample = {
    'input_audio': np.ndarray,      # Raw audio samples
    'target_audio': np.ndarray,     # Corrected audio samples
    'input_pitch': float,           # Detected pitch (Hz)
    'target_pitch': float,          # Desired pitch (Hz)
    'confidence': float,            # Pitch detection confidence
    'scale': str,                   # Musical scale context
    'correction_strength': float,   # How much correction to apply
    'metadata': {
        'instrument': str,          # 'vocal', 'guitar', 'violin', etc.
        'key': str,                 # Musical key (C, D, E, etc.)
        'tempo': float,             # BPM for timing context
        'genre': str               # Musical genre context
    }
}
```

### Data Augmentation Pipeline:
```python
# Required augmentation techniques
AUGMENTATIONS = [
    'pitch_shift',        # ±50 cents pitch variations
    'time_stretch',       # ±10% tempo changes  
    'noise_addition',     # Add realistic background noise
    'reverb_simulation',  # Room acoustics simulation
    'formant_shifting',   # Voice characteristic changes
    'dynamics_variation'  # Volume/compression changes
]
```

### Dataset Sources:
- **Vocal Recordings** - Multiple singers, various styles
- **Instrumental Audio** - Guitar, violin, brass, woodwinds
- **Synthetic Data** - Generated using audio synthesis
- **Multi-track Sessions** - Isolated instrument tracks
- **Live Performance** - Real-world noisy conditions

---

## 🧠 Neural Network Architecture Specifications

### Model Types Required:

#### 1. **Pitch Correction Model**
```python
class PitchCorrectionNet:
    """
    Real-time pitch correction neural network
    """
    def __init__(self):
        self.input_size = 512      # Audio buffer size
        self.output_size = 512     # Corrected audio buffer
        self.latency_target = 5    # milliseconds max
        
    def forward(self, audio_buffer, target_pitch, correction_strength):
        """
        Args:
            audio_buffer: [batch, 512] - Input audio samples
            target_pitch: [batch, 1] - Target frequency in Hz
            correction_strength: [batch, 1] - Correction amount (0.0-1.0)
        Returns:
            corrected_audio: [batch, 512] - Pitch-corrected samples
            confidence: [batch, 1] - Correction confidence score
        """
        pass
```

#### 2. **Intelligent Timing Model**
```python
class TimingAdjustmentNet:
    """
    Natural timing and rhythm correction
    """
    def __init__(self):
        self.context_window = 2048  # Larger context for timing
        self.tempo_range = (60, 200)  # BPM range
        
    def forward(self, audio_sequence, target_tempo, musical_context):
        """
        Args:
            audio_sequence: [batch, 2048] - Audio with timing issues
            target_tempo: [batch, 1] - Target BPM
            musical_context: [batch, features] - Scale, key, genre info
        Returns:
            adjusted_audio: [batch, 2048] - Timing-corrected audio
            timing_confidence: [batch, 1] - Adjustment confidence
        """
        pass
```

#### 3. **Pitch Detection Enhancement**
```python
class EnhancedPitchDetector:
    """
    ML-enhanced pitch detection (better than autocorrelation)
    """
    def __init__(self):
        self.accuracy_target = 0.95  # 95% accuracy on test set
        self.frequency_resolution = 1.0  # Hz precision
        
    def forward(self, audio_buffer):
        """
        Args:
            audio_buffer: [batch, 512] - Input audio samples
        Returns:
            pitch: [batch, 1] - Detected fundamental frequency
            confidence: [batch, 1] - Detection confidence
            harmonics: [batch, N] - Harmonic content analysis
        """
        pass
```

---

## 🔄 Model Export and Integration

### Export Format Requirements:

#### 1. **ONNX Export** (Recommended)
```python
def export_to_onnx(model, output_path):
    """
    Export trained model to ONNX for C++ integration
    """
    dummy_input = torch.randn(1, 512)  # Example input
    torch.onnx.export(
        model,
        dummy_input,
        output_path,
        export_params=True,
        opset_version=11,
        input_names=['audio_input'],
        output_names=['corrected_audio'],
        dynamic_axes={
            'audio_input': {0: 'batch_size'},
            'corrected_audio': {0: 'batch_size'}
        }
    )
```

#### 2. **Custom Binary Format**
```python
def export_to_custom_format(model, output_path):
    """
    Export to custom binary format optimized for real-time C++
    """
    # Extract weights and architecture
    weights = extract_model_weights(model)
    architecture = extract_model_architecture(model)
    
    # Save in optimized binary format
    save_binary_model(weights, architecture, output_path)
```

### C++ Integration Points:

#### 1. **Model Loading in C++**
```cpp
// C++ engine integration (for reference)
class MLPitchCorrector {
private:
    std::unique_ptr<ONNXRuntime> model_;
    
public:
    bool load_model(const std::string& model_path);
    ProcessingResult process_with_ml(const AudioFrame& input, 
                                   const ProcessingParams& params);
};
```

#### 2. **Performance Requirements**
```cpp
// Performance constraints for C++ deployment
constexpr float MAX_LATENCY_MS = 5.0f;      // Maximum processing delay
constexpr float MAX_CPU_USAGE = 25.0f;      // Percentage of single core
constexpr size_t MAX_MEMORY_MB = 100;       // Memory footprint limit
```

---

## 📈 Training Pipeline Specifications

### Training Configuration:
```python
TRAINING_CONFIG = {
    'batch_size': 32,
    'learning_rate': 0.001,
    'num_epochs': 100,
    'early_stopping_patience': 10,
    'validation_split': 0.2,
    'test_split': 0.1,
    'optimizer': 'Adam',
    'loss_function': 'combined_mse_perceptual',
    'metrics': ['mse', 'perceptual_quality', 'pitch_accuracy'],
    'checkpoint_frequency': 5  # epochs
}
```

### Loss Functions:
```python
def combined_loss(predicted_audio, target_audio, predicted_pitch, target_pitch):
    """
    Multi-component loss function
    """
    # Audio reconstruction loss
    audio_loss = F.mse_loss(predicted_audio, target_audio)
    
    # Pitch accuracy loss
    pitch_loss = F.mse_loss(predicted_pitch, target_pitch)
    
    # Perceptual quality loss (STFT-based)
    perceptual_loss = stft_loss(predicted_audio, target_audio)
    
    # Phase coherence loss
    phase_loss = phase_coherence_loss(predicted_audio, target_audio)
    
    return 0.4 * audio_loss + 0.3 * pitch_loss + 0.2 * perceptual_loss + 0.1 * phase_loss
```

### Evaluation Metrics:
```python
EVALUATION_METRICS = {
    'pitch_accuracy': 'percentage of correctly detected pitches',
    'timing_precision': 'RMS error in timing adjustments',
    'audio_quality': 'perceptual quality scores (PESQ, STOI)',
    'latency_compliance': 'percentage of inferences under 5ms',
    'artifact_detection': 'frequency of audible artifacts',
    'naturalness_score': 'human evaluation of correction naturalness'
}
```

---

## 🛠️ Development Tools Required

### Core Libraries:
```python
# Essential dependencies for the ML trainer
REQUIRED_PACKAGES = [
    'torch>=1.12.0',           # Deep learning framework
    'torchaudio>=0.12.0',      # Audio processing
    'librosa>=0.9.0',          # Audio analysis
    'numpy>=1.21.0',           # Numerical computing
    'scipy>=1.7.0',            # Scientific computing
    'pandas>=1.3.0',           # Data manipulation
    'matplotlib>=3.5.0',       # Visualization
    'tensorboard>=2.8.0',      # Training monitoring
    'onnx>=1.12.0',            # Model export
    'onnxruntime>=1.12.0',     # Model inference
    'crepe>=0.0.12',           # Pitch detection
    'resampy>=0.3.0',          # Audio resampling
    'soundfile>=0.10.0',       # Audio I/O
    'pydub>=0.25.0'            # Audio format conversion
]
```

### Project Structure:
```
autotune-audio-ml-trainer/
├── README.md
├── requirements.txt
├── setup.py
├── config/
│   ├── training_config.yaml
│   ├── model_architectures.yaml
│   └── export_settings.yaml
├── src/
│   ├── __init__.py
│   ├── data/
│   │   ├── dataset_creator.py
│   │   ├── audio_preprocessor.py
│   │   ├── augmentation.py
│   │   └── pitch_extractor.py
│   ├── models/
│   │   ├── pitch_correction_net.py
│   │   ├── timing_adjustment_net.py
│   │   ├── enhanced_pitch_detector.py
│   │   └── model_utils.py
│   ├── training/
│   │   ├── trainer.py
│   │   ├── loss_functions.py
│   │   ├── metrics.py
│   │   └── callbacks.py
│   ├── export/
│   │   ├── onnx_exporter.py
│   │   ├── cpp_bridge.py
│   │   └── model_optimizer.py
│   └── evaluation/
│       ├── model_evaluator.py
│       ├── audio_quality_metrics.py
│       └── real_time_tester.py
├── tests/
│   ├── test_data_pipeline.py
│   ├── test_models.py
│   ├── test_training.py
│   └── test_export.py
├── examples/
│   ├── train_pitch_correction.py
│   ├── evaluate_model.py
│   ├── export_for_cpp.py
│   └── dataset_creation_example.py
├── data/
│   ├── raw/
│   ├── processed/
│   └── datasets/
├── models/
│   ├── checkpoints/
│   ├── exported/
│   └── pretrained/
└── docs/
    ├── API_REFERENCE.md
    ├── TRAINING_GUIDE.md
    └── INTEGRATION_GUIDE.md
```

---

## 🔬 Integration Testing Framework

### Testing Requirements:
```python
class IntegrationTester:
    """
    Test ML models with the C++ AutoTune engine
    """
    
    def test_model_export(self, model_path):
        """Test ONNX export compatibility"""
        pass
        
    def test_latency_compliance(self, model_path):
        """Ensure < 5ms inference time"""
        pass
        
    def test_audio_quality(self, model_path, test_dataset):
        """Evaluate audio output quality"""
        pass
        
    def test_cpp_integration(self, exported_model):
        """Test with actual C++ engine"""
        pass
```

### Performance Benchmarks:
```python
PERFORMANCE_TARGETS = {
    'pitch_detection_accuracy': 0.95,      # 95% accuracy minimum
    'correction_naturalness': 0.90,        # Human evaluation score
    'latency_ms': 5.0,                     # Maximum inference time
    'cpu_usage_percent': 25.0,             # Single core usage
    'memory_footprint_mb': 100,            # RAM usage
    'model_size_mb': 50                    # Disk storage
}
```

---

## 📋 Implementation Checklist for AI Agent

### Phase 1: Foundation
- [ ] Set up Python project structure
- [ ] Install and configure dependencies
- [ ] Create basic audio I/O pipeline
- [ ] Implement pitch detection with CREPE/Librosa
- [ ] Set up training/validation data loaders

### Phase 2: Core Models
- [ ] Implement PitchCorrectionNet architecture
- [ ] Implement TimingAdjustmentNet architecture  
- [ ] Implement EnhancedPitchDetector
- [ ] Create training loops and optimization
- [ ] Add loss functions and metrics

### Phase 3: Data Pipeline
- [ ] Build dataset creation tools
- [ ] Implement audio augmentation pipeline
- [ ] Create data preprocessing utilities
- [ ] Add support for multiple audio formats
- [ ] Build train/validation/test splits

### Phase 4: Training System
- [ ] Implement distributed training support
- [ ] Add hyperparameter optimization
- [ ] Create model checkpointing system
- [ ] Build evaluation and monitoring tools
- [ ] Add early stopping and scheduling

### Phase 5: Export & Integration
- [ ] Implement ONNX export functionality
- [ ] Create C++ integration bridge
- [ ] Build model optimization tools
- [ ] Add real-time performance testing
- [ ] Create deployment documentation

### Phase 6: Testing & Validation
- [ ] Write comprehensive unit tests
- [ ] Create integration test suite
- [ ] Build audio quality evaluation tools
- [ ] Add performance benchmarking
- [ ] Validate C++ engine compatibility

---

## 🎵 Success Criteria

The autotune-audio-ml-trainer repository will be considered successful when:

1. **Training Pipeline** - Can train models from audio datasets with 95%+ accuracy
2. **Model Export** - Successfully exports models compatible with C++ AutoTune engine
3. **Real-time Performance** - Exported models meet <5ms latency requirements
4. **Audio Quality** - Corrected audio maintains naturalness and musical quality
5. **Integration** - Seamless workflow from training to C++ deployment
6. **Documentation** - Complete guides for training, evaluation, and deployment

---

## 📞 Support and Reference

### Technical References:
- **AutoTune C++ Engine**: This repository (autotune-real-time-audio-tuner)
- **Audio Processing**: Librosa documentation and tutorials
- **Deep Learning**: PyTorch audio tutorials and examples
- **Model Export**: ONNX Python tutorials and C++ integration guides
- **Real-time Audio**: Jack Audio and PortAudio documentation

### Key Files to Reference:
- `include/autotune_engine.h` - Main engine interface
- `src/pitch_detector.cpp` - Existing pitch detection implementation
- `src/quantizer.cpp` - Musical scale quantization logic
- `python/bindings.cpp` - Python integration examples
- `examples/main.cpp` - Usage patterns and API examples

---

**This guide provides the complete technical specification for creating the autotune-audio-ml-trainer repository that will seamlessly integrate with your real-time C++ AutoTune engine.**
