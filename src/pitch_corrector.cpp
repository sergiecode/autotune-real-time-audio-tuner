#include "pitch_corrector.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace autotune {

PitchCorrector::PitchCorrector(SampleRate sample_rate, uint32_t buffer_size)
    : sample_rate_(sample_rate), buffer_size_(buffer_size), preserve_formants_(true),
      phase_accumulator_(0.0f), previous_input_pitch_(0.0f), previous_target_pitch_(0.0f),
      envelope_follower_(0.0f) {
    
    initialize_parameters();
    
    // Initialize PSOLA buffers
    grain_size_ = buffer_size * 2;  // Overlap for smooth transitions
    overlap_size_ = grain_size_ / 4;
    hop_size_ = grain_size_ - overlap_size_;
    
    overlap_buffer_.resize(overlap_size_, 0.0f);
    grain_buffer_.resize(grain_size_, 0.0f);
    window_.resize(grain_size_);
    
    // Precompute Hanning window for grain processing
    for (uint32_t i = 0; i < grain_size_; ++i) {
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (grain_size_ - 1)));
    }
}

PitchCorrector::~PitchCorrector() = default;

ProcessingResult PitchCorrector::correct_pitch(const Sample* input, Sample* output, 
                                             uint32_t sample_count,
                                             float input_pitch, float target_pitch,
                                             float correction_strength) {
    ProcessingResult result;
    
    if (!input || !output || sample_count == 0) {
        result.success = false;
        return result;
    }
    
    result.detected_pitch = input_pitch;
    result.corrected_pitch = target_pitch;
    
    // If no valid pitch detected or correction strength is zero, pass through
    if (input_pitch <= 0.0f || correction_strength <= 0.0f) {
        std::memcpy(output, input, sample_count * sizeof(Sample));
        result.success = true;
        result.confidence = 0.0f;
        return result;
    }
    
    // Calculate pitch shift ratio
    float pitch_ratio = calculate_pitch_ratio(input_pitch, target_pitch, correction_strength);
    
    // Apply pitch shifting using PSOLA
    result.success = apply_psola_shift(input, output, sample_count, pitch_ratio);
    result.confidence = 0.8f; // Placeholder confidence
    result.latency_samples = overlap_size_; // Approximate latency
    
    return result;
}

ProcessingResult PitchCorrector::correct_pitch(const AudioFrame& input, AudioFrame& output,
                                             float input_pitch, float target_pitch,
                                             float correction_strength) {
    ProcessingResult result;
    
    if (input.size() != output.size()) {
        result.success = false;
        return result;
    }
    
    // Process each channel separately
    // For simplicity, we'll process only the first channel and copy to others
    if (input.size() > 0) {
        Sample input_sample = input[0];
        Sample output_sample;
        
        result = correct_pitch(&input_sample, &output_sample, 1, 
                              input_pitch, target_pitch, correction_strength);
        
        // Copy result to all channels
        for (ChannelCount ch = 0; ch < output.size(); ++ch) {
            output[ch] = output_sample;
        }
    }
    
    return result;
}

void PitchCorrector::set_parameters(const ProcessingParams& params) {
    params_ = params;
    initialize_parameters();
}

void PitchCorrector::set_formant_preservation(bool preserve) {
    preserve_formants_ = preserve;
}

void PitchCorrector::reset() {
    phase_accumulator_ = 0.0f;
    previous_input_pitch_ = 0.0f;
    previous_target_pitch_ = 0.0f;
    envelope_follower_ = 0.0f;
    
    std::fill(overlap_buffer_.begin(), overlap_buffer_.end(), 0.0f);
    std::fill(grain_buffer_.begin(), grain_buffer_.end(), 0.0f);
}

void PitchCorrector::initialize_parameters() {
    // Calculate envelope follower coefficients
    float attack_time_samples = params_.attack_time * sample_rate_;
    float release_time_samples = params_.release_time * sample_rate_;
    
    attack_coeff_ = 1.0f - std::exp(-1.0f / attack_time_samples);
    release_coeff_ = 1.0f - std::exp(-1.0f / release_time_samples);
}

float PitchCorrector::calculate_pitch_ratio(float input_pitch, float target_pitch, float strength) {
    if (input_pitch <= 0.0f || target_pitch <= 0.0f) {
        return 1.0f;
    }
    
    float raw_ratio = target_pitch / input_pitch;
    
    // Apply correction strength (1.0 = full correction, 0.0 = no correction)
    float corrected_ratio = 1.0f + strength * (raw_ratio - 1.0f);
    
    // Limit ratio to reasonable range to avoid artifacts
    return std::clamp(corrected_ratio, 0.5f, 2.0f);
}

bool PitchCorrector::apply_psola_shift(const Sample* input, Sample* output, 
                                      uint32_t sample_count, float pitch_ratio) {
    // Simplified PSOLA implementation
    // In a real implementation, this would be much more sophisticated
    
    for (uint32_t i = 0; i < sample_count; ++i) {
        // Simple linear interpolation approach
        float read_pos = phase_accumulator_;
        uint32_t read_index = static_cast<uint32_t>(read_pos);
        float fraction = read_pos - read_index;
        
        // Bounds checking
        if (read_index < sample_count - 1) {
            // Linear interpolation
            output[i] = input[read_index] * (1.0f - fraction) + 
                       input[read_index + 1] * fraction;
        } else if (read_index < sample_count) {
            output[i] = input[read_index];
        } else {
            output[i] = 0.0f;
        }
        
        // Update phase accumulator
        phase_accumulator_ += pitch_ratio;
        if (phase_accumulator_ >= sample_count) {
            phase_accumulator_ = 0.0f;
        }
        
        // Apply envelope following for smooth transitions
        float envelope = update_envelope(std::abs(input[i]));
        output[i] *= envelope;
    }
    
    return true;
}

uint32_t PitchCorrector::find_pitch_marks(const Sample* samples, uint32_t sample_count,
                                        float pitch, std::vector<uint32_t>& marks) {
    marks.clear();
    
    if (pitch <= 0.0f || sample_count < 2) {
        return 0;
    }
    
    // Calculate expected period in samples
    float period_samples = sample_rate_ / pitch;
    
    // Simple zero-crossing based pitch mark detection
    // In a real implementation, you'd want more sophisticated pitch mark detection
    bool was_positive = samples[0] >= 0.0f;
    uint32_t last_mark = 0;
    
    for (uint32_t i = 1; i < sample_count; ++i) {
        bool is_positive = samples[i] >= 0.0f;
        
        // Detect positive-going zero crossing
        if (!was_positive && is_positive) {
            // Check if enough time has passed since last mark
            if (i - last_mark >= period_samples * 0.5f) {
                marks.push_back(i);
                last_mark = i;
            }
        }
        
        was_positive = is_positive;
    }
    
    return static_cast<uint32_t>(marks.size());
}

void PitchCorrector::apply_window(Sample* data, uint32_t size, int window_type) {
    for (uint32_t i = 0; i < size && i < window_.size(); ++i) {
        if (window_type == 0) {
            // Hanning window
            data[i] *= window_[i];
        } else {
            // Hamming window
            data[i] *= 0.54f - 0.46f * std::cos(2.0f * M_PI * i / (size - 1));
        }
    }
}

float PitchCorrector::update_envelope(float input) {
    float target = std::abs(input);
    
    if (target > envelope_follower_) {
        // Attack
        envelope_follower_ += attack_coeff_ * (target - envelope_follower_);
    } else {
        // Release
        envelope_follower_ += release_coeff_ * (target - envelope_follower_);
    }
    
    return envelope_follower_;
}

} // namespace autotune
