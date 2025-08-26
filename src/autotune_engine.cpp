#include "autotune_engine.h"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace autotune {

AutotuneEngine::AutotuneEngine(SampleRate sample_rate, uint32_t buffer_size, ChannelCount channels)
    : sample_rate_(sample_rate), buffer_size_(buffer_size), channels_(channels),
      mode_(Mode::FULL_AUTOTUNE), initialized_(false), current_pitch_(0.0f),
      target_pitch_(0.0f), confidence_(0.0f), performance_counter_(0),
      current_scale_(Quantizer::Scale::MAJOR), key_center_(60), tempo_(120.0f) {
    
    // Initialize default parameters
    params_.sample_rate = sample_rate;
    params_.buffer_size = buffer_size;
    
    initialized_ = initialize_components();
}

AutotuneEngine::~AutotuneEngine() = default;

ProcessingResult AutotuneEngine::process(const AudioFrame* input, AudioFrame* output, uint32_t frame_count) {
    ProcessingResult result;
    
    if (!initialized_ || !input || !output || frame_count == 0) {
        result.success = false;
        return result;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Prepare processing buffer
    if (processing_buffer_.size() != frame_count) {
        processing_buffer_.resize(frame_count);
        for (auto& frame : processing_buffer_) {
            frame = AudioFrame(channels_);
        }
    }
    
    // Copy input to processing buffer
    for (uint32_t i = 0; i < frame_count; ++i) {
        processing_buffer_[i] = input[i];
    }
    
    // Process based on current mode
    switch (mode_) {
        case Mode::PITCH_CORRECTION:
            result = process_pitch_correction(input, output, frame_count);
            break;
        case Mode::QUANTIZATION:
            result = process_quantization(input, output, frame_count);
            break;
        case Mode::FULL_AUTOTUNE:
            // First apply pitch correction, then quantization
            result = process_pitch_correction(input, processing_buffer_.data(), frame_count);
            if (result.success) {
                ProcessingResult quant_result = process_quantization(processing_buffer_.data(), output, frame_count);
                result.success = result.success && quant_result.success;
            }
            break;
        case Mode::BYPASS:
            // Pass through
            for (uint32_t i = 0; i < frame_count; ++i) {
                output[i] = input[i];
            }
            result.success = true;
            break;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    float processing_time = duration.count() / 1000.0f; // Convert to milliseconds
    
    update_performance_metrics(processing_time);
    
    return result;
}

ProcessingResult AutotuneEngine::process_frame(const AudioFrame& input, AudioFrame& output) {
    return process(&input, &output, 1);
}

void AutotuneEngine::set_parameters(const ProcessingParams& params) {
    params_ = params;
    
    if (pitch_corrector_) {
        pitch_corrector_->set_parameters(params);
    }
}

const ProcessingParams& AutotuneEngine::get_parameters() const {
    return params_;
}

void AutotuneEngine::set_mode(Mode mode) {
    mode_ = mode;
}

void AutotuneEngine::set_scale(Quantizer::Scale scale, int key_center) {
    current_scale_ = scale;
    key_center_ = key_center;
}

void AutotuneEngine::set_tempo(float tempo) {
    tempo_ = tempo;
    if (quantizer_) {
        quantizer_->set_tempo(tempo);
    }
}

void AutotuneEngine::configure_features(bool enable_pitch_correction,
                                       bool enable_quantization,
                                       bool enable_formant_preservation) {
    if (pitch_corrector_) {
        pitch_corrector_->set_formant_preservation(enable_formant_preservation);
    }
    
    // Update mode based on enabled features
    if (enable_pitch_correction && enable_quantization) {
        mode_ = Mode::FULL_AUTOTUNE;
    } else if (enable_pitch_correction) {
        mode_ = Mode::PITCH_CORRECTION;
    } else if (enable_quantization) {
        mode_ = Mode::QUANTIZATION;
    } else {
        mode_ = Mode::BYPASS;
    }
}

AutotuneEngine::PerformanceMetrics AutotuneEngine::get_performance_metrics() const {
    return metrics_;
}

void AutotuneEngine::reset() {
    if (pitch_detector_) {
        pitch_detector_->reset();
    }
    if (pitch_corrector_) {
        pitch_corrector_->reset();
    }
    if (quantizer_) {
        quantizer_->reset();
    }
    if (input_buffer_) {
        input_buffer_->clear();
    }
    if (output_buffer_) {
        output_buffer_->clear();
    }
    
    current_pitch_ = 0.0f;
    target_pitch_ = 0.0f;
    confidence_ = 0.0f;
    metrics_ = PerformanceMetrics();
    performance_counter_ = 0;
    latency_history_.clear();
}

uint32_t AutotuneEngine::get_recommended_buffer_size(SampleRate sample_rate) {
    // Recommended buffer sizes for low latency based on sample rate
    if (sample_rate <= 22050) return 128;
    if (sample_rate <= 44100) return 256;
    if (sample_rate <= 48000) return 512;
    if (sample_rate <= 96000) return 1024;
    return 2048;
}

bool AutotuneEngine::initialize_components() {
    try {
        // Create audio buffers
        input_buffer_ = std::make_unique<AudioBuffer>(buffer_size_ * 4, channels_);
        output_buffer_ = std::make_unique<AudioBuffer>(buffer_size_ * 4, channels_);
        
        // Create processing components
        pitch_detector_ = std::make_unique<PitchDetector>(sample_rate_, buffer_size_);
        pitch_corrector_ = std::make_unique<PitchCorrector>(sample_rate_, buffer_size_);
        quantizer_ = std::make_unique<Quantizer>(sample_rate_, tempo_);
        
        // Initialize buffers
        mono_buffer_.resize(buffer_size_, 0.0f);
        latency_history_.reserve(100); // Keep last 100 measurements
        
        return true;
    } catch (...) {
        return false;
    }
}

ProcessingResult AutotuneEngine::process_pitch_correction(const AudioFrame* input, 
                                                        AudioFrame* output, 
                                                        uint32_t frame_count) {
    ProcessingResult result;
    
    if (frame_count == 0) {
        result.success = false;
        return result;
    }
    
    // Convert to mono for pitch detection
    convert_to_mono(input, frame_count);
    
    // Detect pitch
    current_pitch_ = pitch_detector_->detect_pitch(mono_buffer_.data(), 
                                                  std::min(frame_count, static_cast<uint32_t>(mono_buffer_.size())), 
                                                  confidence_);
    
    // Calculate target pitch
    target_pitch_ = calculate_target_pitch(current_pitch_);
    
    // Apply pitch correction to each frame
    for (uint32_t i = 0; i < frame_count; ++i) {
        result = pitch_corrector_->correct_pitch(input[i], output[i], 
                                               current_pitch_, target_pitch_,
                                               params_.correction_strength);
        if (!result.success) {
            break;
        }
    }
    
    result.detected_pitch = current_pitch_;
    result.corrected_pitch = target_pitch_;
    result.confidence = confidence_;
    
    return result;
}

ProcessingResult AutotuneEngine::process_quantization(const AudioFrame* input, 
                                                    AudioFrame* output, 
                                                    uint32_t frame_count) {
    ProcessingResult result;
    result.success = true;
    
    // For now, quantization is applied through pitch correction
    // In a more sophisticated implementation, this would handle timing quantization
    
    // Simple passthrough with potential timing adjustments
    for (uint32_t i = 0; i < frame_count; ++i) {
        output[i] = input[i];
    }
    
    return result;
}

void AutotuneEngine::convert_to_mono(const AudioFrame* input, uint32_t frame_count) {
    uint32_t samples_to_process = std::min(frame_count, static_cast<uint32_t>(mono_buffer_.size()));
    
    for (uint32_t i = 0; i < samples_to_process; ++i) {
        if (input[i].size() == 1) {
            // Already mono
            mono_buffer_[i] = input[i][0];
        } else if (input[i].size() >= 2) {
            // Mix stereo to mono
            mono_buffer_[i] = (input[i][0] + input[i][1]) * 0.5f;
        } else {
            mono_buffer_[i] = 0.0f;
        }
    }
}

void AutotuneEngine::update_performance_metrics(float processing_time) {
    latency_history_.push_back(processing_time);
    if (latency_history_.size() > 100) {
        latency_history_.erase(latency_history_.begin());
    }
    
    // Calculate average latency
    if (!latency_history_.empty()) {
        float sum = 0.0f;
        for (float latency : latency_history_) {
            sum += latency;
        }
        metrics_.average_latency_ms = sum / latency_history_.size();
    }
    
    // Estimate CPU usage (simplified)
    float max_processing_time = (static_cast<float>(buffer_size_) / sample_rate_) * 1000.0f;
    metrics_.cpu_usage_percent = (processing_time / max_processing_time) * 100.0f;
    
    metrics_.frames_processed += buffer_size_;
    performance_counter_++;
}

float AutotuneEngine::calculate_target_pitch(float detected_pitch) {
    if (detected_pitch <= 0.0f || !quantizer_) {
        return detected_pitch;
    }
    
    // Use quantizer to find target pitch
    return quantizer_->quantize_pitch(detected_pitch, current_scale_, key_center_, 
                                     params_.quantize_strength);
}

} // namespace autotune
