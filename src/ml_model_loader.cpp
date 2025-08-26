#include "ml_model_loader.h"
#include "pitch_detector.h"
#include "pitch_corrector.h"
#include <iostream>
#include <fstream>
#include <chrono>

namespace autotune {

MLModelLoader::MLModelLoader() 
#ifdef HAVE_ONNXRUNTIME
    : memory_info_(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
#endif
{
}

MLModelLoader::~MLModelLoader() {
#ifdef HAVE_ONNXRUNTIME
    cleanup_names();
#endif
}

bool MLModelLoader::load_model(const std::string& model_path) {
    model_path_ = model_path;
    
    // Check if file exists
    std::ifstream file(model_path);
    if (!file.good()) {
        std::cerr << "ML Model file not found: " << model_path << std::endl;
        return false;
    }
    
#ifdef HAVE_ONNXRUNTIME
    return initialize_onnx_session(model_path);
#else
    std::cout << "ONNX Runtime not available - using traditional processing" << std::endl;
    return false;
#endif
}

#ifdef HAVE_ONNXRUNTIME
bool MLModelLoader::initialize_onnx_session(const std::string& model_path) {
    try {
        // Clean up previous session
        cleanup_names();
        
        env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "AutoTuneML");
        
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        
        // Create session
        session_ = std::make_unique<Ort::Session>(*env_, model_path.c_str(), session_options);
        
        // Get input/output info
        Ort::AllocatorWithDefaultOptions allocator;
        
        // Input info (expecting: audio_buffer, target_pitch, correction_strength)
        size_t num_input_nodes = session_->GetInputCount();
        input_names_.reserve(num_input_nodes);
        
        for (size_t i = 0; i < num_input_nodes; i++) {
            auto input_name = session_->GetInputNameAllocated(i, allocator);
            input_names_.push_back(input_name.release());
        }
        
        // Output info (expecting: corrected_audio, confidence)
        size_t num_output_nodes = session_->GetOutputCount();
        output_names_.reserve(num_output_nodes);
        
        for (size_t i = 0; i < num_output_nodes; i++) {
            auto output_name = session_->GetOutputNameAllocated(i, allocator);
            output_names_.push_back(output_name.release());
        }
        
        // Get input shape (should be [1, 512] for audio_buffer)
        auto input_type_info = session_->GetInputTypeInfo(0);
        auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
        input_shape_ = input_tensor_info.GetShape();
        
        // Get output shape
        auto output_type_info = session_->GetOutputTypeInfo(0);
        auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
        output_shape_ = output_tensor_info.GetShape();
        
        model_loaded_ = true;
        
        std::cout << "ML Model loaded successfully: " << model_path << std::endl;
        std::cout << "Input nodes: " << num_input_nodes << std::endl;
        std::cout << "Output nodes: " << num_output_nodes << std::endl;
        if (!input_shape_.empty() && input_shape_.size() >= 2) {
            std::cout << "Input shape: [" << input_shape_[0] << ", " << input_shape_[1] << "]" << std::endl;
        }
        
        return true;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime error: " << e.what() << std::endl;
        return false;
    }
}

void MLModelLoader::cleanup_names() {
    for (auto name : input_names_) {
        if (name) free(const_cast<char*>(name));
    }
    for (auto name : output_names_) {
        if (name) free(const_cast<char*>(name));
    }
    input_names_.clear();
    output_names_.clear();
}
#endif

ProcessingResult MLModelLoader::process_with_ml(const AudioFrame& input,
                                               const ProcessingParams& params) {
    if (!model_loaded_) {
        return process_traditional(input, params);
    }
    
#ifdef HAVE_ONNXRUNTIME
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Prepare input tensors
        std::vector<float> audio_data(512);
        for (size_t i = 0; i < 512 && i < input.get_sample_count(); ++i) {
            audio_data[i] = input.get_sample(0, i);
        }
        
        // Detect pitch for target_pitch input
        PitchDetector detector;
        Note detected_note = detector.detect_pitch(input);
        
        std::vector<float> target_pitch_data = {detected_note.frequency};
        std::vector<float> correction_strength_data = {params.correction_strength};
        
        // Create input tensors
        std::vector<Ort::Value> input_tensors;
        
        // Audio buffer tensor [1, 512]
        std::vector<int64_t> audio_shape = {1, 512};
        input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
            memory_info_, audio_data.data(), audio_data.size(),
            audio_shape.data(), audio_shape.size()));
        
        // Target pitch tensor [1, 1] (if model expects it)
        if (input_names_.size() > 1) {
            std::vector<int64_t> pitch_shape = {1, 1};
            input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
                memory_info_, target_pitch_data.data(), target_pitch_data.size(),
                pitch_shape.data(), pitch_shape.size()));
        }
        
        // Correction strength tensor [1, 1] (if model expects it)
        if (input_names_.size() > 2) {
            std::vector<int64_t> strength_shape = {1, 1};
            input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
                memory_info_, correction_strength_data.data(), correction_strength_data.size(),
                strength_shape.data(), strength_shape.size()));
        }
        
        // Run inference
        auto output_tensors = session_->Run(Ort::RunOptions{nullptr},
                                          input_names_.data(), input_tensors.data(), input_tensors.size(),
                                          output_names_.data(), output_names_.size());
        
        // Extract results
        float* corrected_audio = output_tensors[0].GetTensorMutableData<float>();
        
        // Copy corrected audio back to input frame (in-place processing)
        AudioFrame& mutable_input = const_cast<AudioFrame&>(input);
        for (size_t i = 0; i < 512 && i < input.get_sample_count(); ++i) {
            mutable_input.set_sample(0, i, corrected_audio[i]);
        }
        
        // Calculate inference time
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        last_inference_time_ms_ = duration.count() / 1000.0f;
        
        ProcessingResult result;
        result.detected_pitch = detected_note.frequency;
        result.corrected_pitch = detected_note.frequency; // ML model handles correction
        result.confidence = (output_names_.size() > 1) ? 
                           *output_tensors[1].GetTensorMutableData<float>() : 0.9f;
        result.processing_time_ms = last_inference_time_ms_;
        
        return result;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "ML inference error: " << e.what() << std::endl;
        return process_traditional(input, params);
    }
#else
    return process_traditional(input, params);
#endif
}

ProcessingResult MLModelLoader::process_traditional(const AudioFrame& input,
                                                  const ProcessingParams& params) {
    // Use existing AutoTune algorithms as fallback
    PitchDetector detector;
    PitchCorrector corrector;
    
    Note detected_note = detector.detect_pitch(input);
    
    ProcessingResult result;
    result.detected_pitch = detected_note.frequency;
    result.confidence = detected_note.confidence;
    result.processing_time_ms = 1.0f; // Estimated traditional processing time
    
    // Apply traditional correction
    AudioFrame& mutable_input = const_cast<AudioFrame&>(input);
    corrector.apply_correction(mutable_input, params);
    result.corrected_pitch = detected_note.frequency;
    
    return result;
}

std::string MLModelLoader::get_model_info() const {
    if (!model_loaded_) {
        return "No ML model loaded - using traditional processing";
    }
    
    std::string info = "ML model loaded: " + model_path_;
    if (last_inference_time_ms_ > 0.0f) {
        info += " (last inference: " + std::to_string(last_inference_time_ms_) + "ms)";
    }
    return info;
}

} // namespace autotune
