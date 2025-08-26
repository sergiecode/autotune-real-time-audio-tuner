#pragma once

#ifdef HAVE_ONNXRUNTIME
#include <onnxruntime_cxx_api.h>
#endif

#include "audio_types.h"
#include <memory>
#include <string>
#include <vector>

namespace autotune {

/**
 * ML Model Loader for trained AutoTune models
 * Loads ONNX models exported from autotune-audio-ml-trainer
 */
class MLModelLoader {
public:
    MLModelLoader();
    ~MLModelLoader();
    
    /**
     * Load an ONNX model from the ML trainer project
     * @param model_path Path to .onnx file
     * @return true if loaded successfully
     */
    bool load_model(const std::string& model_path);
    
    /**
     * Check if a model is currently loaded
     */
    bool is_model_loaded() const { return model_loaded_; }
    
    /**
     * Process audio using the loaded ML model
     * @param input Audio frame (512 samples)
     * @param params Processing parameters
     * @return Processing result with corrected audio
     */
    ProcessingResult process_with_ml(const AudioFrame& input,
                                   const ProcessingParams& params);
    
    /**
     * Get model information
     */
    std::string get_model_info() const;
    
    /**
     * Get last inference time in milliseconds
     */
    float get_last_inference_time_ms() const { return last_inference_time_ms_; }

private:
    bool model_loaded_ = false;
    std::string model_path_;
    float last_inference_time_ms_ = 0.0f;
    
#ifdef HAVE_ONNXRUNTIME
    std::unique_ptr<Ort::Session> session_;
    std::unique_ptr<Ort::Env> env_;
    Ort::MemoryInfo memory_info_;
    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;
    std::vector<int64_t> input_shape_;
    std::vector<int64_t> output_shape_;
    
    bool initialize_onnx_session(const std::string& model_path);
    void cleanup_names();
#endif
    
    // Fallback processing without ML (existing algorithm)
    ProcessingResult process_traditional(const AudioFrame& input,
                                       const ProcessingParams& params);
};

} // namespace autotune
