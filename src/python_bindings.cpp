#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "autotune_engine.h"

namespace py = pybind11;
using namespace autotune;

/**
 * @brief Python bindings for AutoTune Real-Time Engine
 * 
 * This module provides Python access to the C++ AutoTune engine,
 * enabling integration with machine learning workflows and 
 * Python-based audio processing tools.
 */

PYBIND11_MODULE(pyautotune, m) {
    m.doc() = "AutoTune Real-Time Audio Engine - Python Bindings";
    m.attr("__version__") = "1.0.0";
    
    // Audio types
    py::class_<AudioFrame>(m, "AudioFrame")
        .def(py::init<ChannelCount>(), "Create AudioFrame with specified channel count")
        .def("__getitem__", [](const AudioFrame& frame, ChannelCount channel) {
            return frame[channel];
        })
        .def("__setitem__", [](AudioFrame& frame, ChannelCount channel, Sample value) {
            frame[channel] = value;
        })
        .def("size", &AudioFrame::size, "Get number of channels")
        .def("__len__", &AudioFrame::size);
    
    py::class_<Note>(m, "Note")
        .def(py::init<float, int, float>(), 
             "Create Note with frequency, MIDI note, and cents deviation")
        .def_readwrite("frequency", &Note::frequency, "Frequency in Hz")
        .def_readwrite("midi_note", &Note::midi_note, "MIDI note number")
        .def_readwrite("cents", &Note::cents, "Cents deviation from equal temperament");
    
    py::class_<ProcessingParams>(m, "ProcessingParams")
        .def(py::init<>())
        .def_readwrite("sample_rate", &ProcessingParams::sample_rate, "Sample rate in Hz")
        .def_readwrite("buffer_size", &ProcessingParams::buffer_size, "Buffer size in samples")
        .def_readwrite("correction_strength", &ProcessingParams::correction_strength, 
                      "Pitch correction strength (0.0-1.0)")
        .def_readwrite("attack_time", &ProcessingParams::attack_time, "Attack time in seconds")
        .def_readwrite("release_time", &ProcessingParams::release_time, "Release time in seconds")
        .def_readwrite("enable_quantization", &ProcessingParams::enable_quantization, 
                      "Enable quantization")
        .def_readwrite("quantize_strength", &ProcessingParams::quantize_strength, 
                      "Quantization strength (0.0-1.0)");
    
    py::class_<ProcessingResult>(m, "ProcessingResult")
        .def(py::init<>())
        .def_readwrite("success", &ProcessingResult::success, "Processing success flag")
        .def_readwrite("detected_pitch", &ProcessingResult::detected_pitch, 
                      "Detected pitch in Hz")
        .def_readwrite("corrected_pitch", &ProcessingResult::corrected_pitch, 
                      "Corrected pitch in Hz")
        .def_readwrite("confidence", &ProcessingResult::confidence, 
                      "Detection confidence (0.0-1.0)")
        .def_readwrite("latency_samples", &ProcessingResult::latency_samples, 
                      "Processing latency in samples");
    
    // Quantizer enums and class
    py::enum_<Quantizer::Scale>(m, "Scale")
        .value("CHROMATIC", Quantizer::Scale::CHROMATIC)
        .value("MAJOR", Quantizer::Scale::MAJOR)
        .value("MINOR", Quantizer::Scale::MINOR)
        .value("PENTATONIC", Quantizer::Scale::PENTATONIC)
        .value("BLUES", Quantizer::Scale::BLUES)
        .value("DORIAN", Quantizer::Scale::DORIAN)
        .value("MIXOLYDIAN", Quantizer::Scale::MIXOLYDIAN)
        .value("CUSTOM", Quantizer::Scale::CUSTOM);
    
    py::class_<Quantizer>(m, "Quantizer")
        .def(py::init<SampleRate, float>(), "Create Quantizer with sample rate and tempo")
        .def("quantize_pitch", &Quantizer::quantize_pitch,
             "Quantize pitch to musical scale",
             py::arg("input_pitch"), py::arg("scale"), 
             py::arg("key_center") = 60, py::arg("strength") = 1.0f)
        .def("set_tempo", &Quantizer::set_tempo, "Set tempo in BPM")
        .def("set_custom_scale", &Quantizer::set_custom_scale, 
             "Set custom scale intervals")
        .def("get_nearest_note", &Quantizer::get_nearest_note,
             "Get nearest note in scale")
        .def_static("frequency_to_midi", &Quantizer::frequency_to_midi,
                   "Convert frequency to MIDI note number")
        .def_static("midi_to_frequency", &Quantizer::midi_to_frequency,
                   "Convert MIDI note number to frequency")
        .def("get_tempo", &Quantizer::get_tempo, "Get current tempo")
        .def("reset", &Quantizer::reset, "Reset quantizer state");
    
    // PitchDetector class
    py::class_<PitchDetector>(m, "PitchDetector")
        .def(py::init<SampleRate, uint32_t>(), 
             "Create PitchDetector with sample rate and buffer size")
        .def("detect_pitch", 
             [](PitchDetector& detector, py::array_t<float> samples) {
                 py::buffer_info buf = samples.request();
                 float confidence;
                 float pitch = detector.detect_pitch(
                     static_cast<const Sample*>(buf.ptr), 
                     static_cast<uint32_t>(buf.size), 
                     confidence);
                 return py::make_tuple(pitch, confidence);
             },
             "Detect pitch from audio samples, returns (pitch, confidence)")
        .def("set_min_frequency", &PitchDetector::set_min_frequency,
             "Set minimum detectable frequency")
        .def("set_max_frequency", &PitchDetector::set_max_frequency,
             "Set maximum detectable frequency")
        .def("set_confidence_threshold", &PitchDetector::set_confidence_threshold,
             "Set confidence threshold")
        .def("reset", &PitchDetector::reset, "Reset detector state");
    
    // AutotuneEngine class
    py::enum_<AutotuneEngine::Mode>(m, "Mode")
        .value("PITCH_CORRECTION", AutotuneEngine::Mode::PITCH_CORRECTION)
        .value("QUANTIZATION", AutotuneEngine::Mode::QUANTIZATION)
        .value("FULL_AUTOTUNE", AutotuneEngine::Mode::FULL_AUTOTUNE)
        .value("BYPASS", AutotuneEngine::Mode::BYPASS);
    
    py::class_<AutotuneEngine::PerformanceMetrics>(m, "PerformanceMetrics")
        .def(py::init<>())
        .def_readwrite("average_latency_ms", 
                      &AutotuneEngine::PerformanceMetrics::average_latency_ms)
        .def_readwrite("cpu_usage_percent", 
                      &AutotuneEngine::PerformanceMetrics::cpu_usage_percent)
        .def_readwrite("buffer_underruns", 
                      &AutotuneEngine::PerformanceMetrics::buffer_underruns)
        .def_readwrite("frames_processed", 
                      &AutotuneEngine::PerformanceMetrics::frames_processed);
    
    py::class_<AutotuneEngine>(m, "AutotuneEngine")
        .def(py::init<SampleRate, uint32_t, ChannelCount>(),
             "Create AutotuneEngine with sample rate, buffer size, and channel count",
             py::arg("sample_rate"), py::arg("buffer_size") = 512, py::arg("channels") = 2)
        .def("process_numpy", 
             [](AutotuneEngine& engine, py::array_t<float> input_array) {
                 py::buffer_info buf = input_array.request();
                 
                 if (buf.ndim != 2) {
                     throw std::runtime_error("Input array must be 2D (frames, channels)");
                 }
                 
                 uint32_t frame_count = static_cast<uint32_t>(buf.shape[0]);
                 ChannelCount channel_count = static_cast<ChannelCount>(buf.shape[1]);
                 
                 // Convert numpy array to AudioFrames
                 std::vector<AudioFrame> input_frames(frame_count);
                 std::vector<AudioFrame> output_frames(frame_count);
                 
                 float* input_ptr = static_cast<float*>(buf.ptr);
                 
                 for (uint32_t i = 0; i < frame_count; ++i) {
                     input_frames[i] = AudioFrame(channel_count);
                     output_frames[i] = AudioFrame(channel_count);
                     for (ChannelCount ch = 0; ch < channel_count; ++ch) {
                         input_frames[i][ch] = input_ptr[i * channel_count + ch];
                     }
                 }
                 
                 // Process audio
                 ProcessingResult result = engine.process(input_frames.data(), 
                                                        output_frames.data(), 
                                                        frame_count);
                 
                 // Convert back to numpy array
                 auto output_array = py::array_t<float>(
                     {frame_count, channel_count},
                     {sizeof(float) * channel_count, sizeof(float)}
                 );
                 
                 py::buffer_info output_buf = output_array.request();
                 float* output_ptr = static_cast<float*>(output_buf.ptr);
                 
                 for (uint32_t i = 0; i < frame_count; ++i) {
                     for (ChannelCount ch = 0; ch < channel_count; ++ch) {
                         output_ptr[i * channel_count + ch] = output_frames[i][ch];
                     }
                 }
                 
                 return py::make_tuple(output_array, result);
             },
             "Process numpy audio array, returns (output_array, result)")
        .def("set_parameters", &AutotuneEngine::set_parameters, "Set processing parameters")
        .def("get_parameters", &AutotuneEngine::get_parameters, "Get current parameters")
        .def("set_mode", &AutotuneEngine::set_mode, "Set processing mode")
        .def("get_mode", &AutotuneEngine::get_mode, "Get current mode")
        .def("set_scale", &AutotuneEngine::set_scale, "Set musical scale and key center")
        .def("set_tempo", &AutotuneEngine::set_tempo, "Set tempo for quantization")
        .def("configure_features", &AutotuneEngine::configure_features,
             "Configure processing features")
        .def("get_performance_metrics", &AutotuneEngine::get_performance_metrics,
             "Get performance metrics")
        .def("reset", &AutotuneEngine::reset, "Reset engine state")
        .def("is_initialized", &AutotuneEngine::is_initialized, "Check if initialized")
        .def_static("get_recommended_buffer_size", 
                   &AutotuneEngine::get_recommended_buffer_size,
                   "Get recommended buffer size for sample rate");
    
    // Utility functions
    m.def("generate_sine_wave", 
          [](float frequency, SampleRate sample_rate, float duration, float amplitude) {
              uint32_t sample_count = static_cast<uint32_t>(duration * sample_rate);
              auto output = py::array_t<float>(sample_count);
              py::buffer_info buf = output.request();
              float* ptr = static_cast<float*>(buf.ptr);
              
              float angular_freq = 2.0f * M_PI * frequency / sample_rate;
              for (uint32_t i = 0; i < sample_count; ++i) {
                  ptr[i] = amplitude * std::sin(angular_freq * i);
              }
              
              return output;
          },
          "Generate sine wave for testing",
          py::arg("frequency"), py::arg("sample_rate"), 
          py::arg("duration"), py::arg("amplitude") = 0.5f);
    
    // Module-level constants
    m.attr("DEFAULT_SAMPLE_RATE") = 44100;
    m.attr("DEFAULT_BUFFER_SIZE") = 512;
    m.attr("DEFAULT_CHANNELS") = 2;
}

// Example Python usage in docstring
static const char* python_example = R"(
import pyautotune
import numpy as np

# Create engine
engine = pyautotune.AutotuneEngine(44100, 512, 2)

# Configure for C Major scale
engine.set_scale(pyautotune.Scale.MAJOR, 60)  # C4 as root
engine.set_mode(pyautotune.Mode.FULL_AUTOTUNE)

# Set parameters
params = pyautotune.ProcessingParams()
params.correction_strength = 0.8
params.quantize_strength = 0.9
engine.set_parameters(params)

# Generate test audio (slightly off-pitch)
test_audio = pyautotune.generate_sine_wave(265.0, 44100, 1.0)  # Flat C4
stereo_audio = np.column_stack([test_audio, test_audio])  # Make stereo

# Process audio
output_audio, result = engine.process_numpy(stereo_audio)

print(f"Detected pitch: {result.detected_pitch:.2f} Hz")
print(f"Corrected pitch: {result.corrected_pitch:.2f} Hz")
print(f"Confidence: {result.confidence:.2f}")

# Get performance metrics
metrics = engine.get_performance_metrics()
print(f"Average latency: {metrics.average_latency_ms:.2f} ms")
print(f"CPU usage: {metrics.cpu_usage_percent:.1f}%")
)";
