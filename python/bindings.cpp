#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "autotune_engine.h"
#include "audio_types.h"
#include "quantizer.h"

namespace py = pybind11;

PYBIND11_MODULE(autotune_engine, m) {
    m.doc() = "AutoTune Real-Time Audio Processing Engine";

    // AudioFrame class
    py::class_<AudioFrame>(m, "AudioFrame")
        .def(py::init<size_t>())
        .def("get_sample", &AudioFrame::get_sample)
        .def("set_sample", &AudioFrame::set_sample)
        .def("get_channel_count", &AudioFrame::get_channel_count)
        .def("get_sample_count", &AudioFrame::get_sample_count)
        .def("clear", &AudioFrame::clear);

    // Note class
    py::class_<Note>(m, "Note")
        .def(py::init<float, float, float>())
        .def_readwrite("frequency", &Note::frequency)
        .def_readwrite("confidence", &Note::confidence)
        .def_readwrite("timestamp", &Note::timestamp);

    // ProcessingParams struct
    py::class_<ProcessingParams>(m, "ProcessingParams")
        .def(py::init<>())
        .def_readwrite("correction_strength", &ProcessingParams::correction_strength)
        .def_readwrite("target_scale", &ProcessingParams::target_scale)
        .def_readwrite("enable_quantization", &ProcessingParams::enable_quantization)
        .def_readwrite("preserve_formants", &ProcessingParams::preserve_formants)
        .def_readwrite("smoothing_factor", &ProcessingParams::smoothing_factor);

    // ProcessingResult struct
    py::class_<ProcessingResult>(m, "ProcessingResult")
        .def(py::init<>())
        .def_readwrite("detected_pitch", &ProcessingResult::detected_pitch)
        .def_readwrite("corrected_pitch", &ProcessingResult::corrected_pitch)
        .def_readwrite("confidence", &ProcessingResult::confidence)
        .def_readwrite("processing_time_ms", &ProcessingResult::processing_time_ms);

    // Scale enum
    py::enum_<Scale>(m, "Scale")
        .value("MAJOR", Scale::MAJOR)
        .value("MINOR", Scale::MINOR)
        .value("BLUES", Scale::BLUES)
        .value("PENTATONIC", Scale::PENTATONIC)
        .value("DORIAN", Scale::DORIAN)
        .value("MIXOLYDIAN", Scale::MIXOLYDIAN)
        .value("CHROMATIC", Scale::CHROMATIC)
        .value("CUSTOM", Scale::CUSTOM);

    // AutotuneEngine class
    py::class_<AutotuneEngine>(m, "AutotuneEngine")
        .def(py::init<int, int>())
        .def("process_frame", &AutotuneEngine::process_frame)
        .def("set_target_pitch", &AutotuneEngine::set_target_pitch)
        .def("set_correction_strength", &AutotuneEngine::set_correction_strength)
        .def("enable_real_time_mode", &AutotuneEngine::enable_real_time_mode)
        .def("get_performance_stats", &AutotuneEngine::get_performance_stats)
        .def("reset", &AutotuneEngine::reset);

    // Quantizer class  
    py::class_<Quantizer>(m, "Quantizer")
        .def(py::init<>())
        .def("quantize_pitch", &Quantizer::quantize_pitch)
        .def("get_nearest_note", &Quantizer::get_nearest_note)
        .def("set_custom_scale", &Quantizer::set_custom_scale)
        .def_static("frequency_to_midi", &Quantizer::frequency_to_midi)
        .def_static("midi_to_frequency", &Quantizer::midi_to_frequency);

    // Utility functions for NumPy array processing
    m.def("process_numpy_audio", [](py::array_t<float> input, 
                                   int sample_rate, 
                                   float correction_strength) {
        py::buffer_info buf_info = input.request();
        
        if (buf_info.ndim != 1) {
            throw std::runtime_error("Input array must be 1-dimensional");
        }
        
        float* ptr = static_cast<float*>(buf_info.ptr);
        size_t size = buf_info.shape[0];
        
        // Create engine
        AutotuneEngine engine(sample_rate, 512);
        
        // Process in chunks
        auto result = py::array_t<float>(size);
        py::buffer_info result_buf = result.request();
        float* result_ptr = static_cast<float*>(result_buf.ptr);
        
        ProcessingParams params;
        params.correction_strength = correction_strength;
        
        size_t chunk_size = 512;
        for (size_t i = 0; i < size; i += chunk_size) {
            size_t current_chunk = std::min(chunk_size, size - i);
            
            AudioFrame frame(current_chunk);
            for (size_t j = 0; j < current_chunk; j++) {
                frame.set_sample(0, j, ptr[i + j]);
            }
            
            auto proc_result = engine.process_frame(frame, params);
            
            for (size_t j = 0; j < current_chunk; j++) {
                result_ptr[i + j] = frame.get_sample(0, j);
            }
        }
        
        return result;
    }, "Process NumPy audio array through AutoTune engine");

    // Version info
    m.attr("__version__") = "1.0.0";
    m.attr("__author__") = "AutoTune Engine Team";
}
