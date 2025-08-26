#include "pitch_detector.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace autotune {

PitchDetector::PitchDetector(SampleRate sample_rate, uint32_t buffer_size)
    : sample_rate_(sample_rate), buffer_size_(buffer_size),
      min_frequency_(80.0f), max_frequency_(2000.0f), confidence_threshold_(0.3f),
      previous_pitch_(0.0f), pitch_smoothing_factor_(0.8f) {
    
    // Initialize processing buffers
    windowed_buffer_.resize(buffer_size);
    autocorr_buffer_.resize(buffer_size);
    hanning_window_.resize(buffer_size);
    
    // Precompute Hanning window
    for (uint32_t i = 0; i < buffer_size; ++i) {
        hanning_window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (buffer_size - 1)));
    }
}

PitchDetector::~PitchDetector() = default;

float PitchDetector::detect_pitch(const Sample* samples, uint32_t sample_count, float& confidence) {
    if (!samples || sample_count == 0 || sample_count > buffer_size_) {
        confidence = 0.0f;
        return 0.0f;
    }
    
    // Apply windowing to reduce spectral leakage
    apply_window(samples, windowed_buffer_.data(), sample_count);
    
    // Compute autocorrelation
    compute_autocorrelation(windowed_buffer_.data(), autocorr_buffer_.data(), sample_count);
    
    // Find peak in autocorrelation function
    uint32_t peak_lag = find_autocorr_peak(autocorr_buffer_.data(), sample_count, confidence);
    
    if (confidence < confidence_threshold_ || peak_lag == 0) {
        confidence = 0.0f;
        return 0.0f;
    }
    
    // Convert lag to frequency
    float detected_pitch = lag_to_frequency(peak_lag);
    
    // Validate frequency range
    if (detected_pitch < min_frequency_ || detected_pitch > max_frequency_) {
        confidence = 0.0f;
        return 0.0f;
    }
    
    // Apply smoothing
    detected_pitch = smooth_pitch(detected_pitch);
    
    return detected_pitch;
}

float PitchDetector::detect_pitch(const AudioFrame& frame, float& confidence) {
    // Use first channel for mono processing
    if (frame.size() == 0) {
        confidence = 0.0f;
        return 0.0f;
    }
    
    // For now, just use the first channel
    // In a real implementation, you might want to mix channels or process them separately
    std::vector<Sample> mono_samples(1);
    mono_samples[0] = frame[0];
    
    return detect_pitch(mono_samples.data(), 1, confidence);
}

void PitchDetector::set_min_frequency(float min_freq) {
    min_frequency_ = std::max(1.0f, min_freq);
}

void PitchDetector::set_max_frequency(float max_freq) {
    max_frequency_ = std::min(static_cast<float>(sample_rate_) / 2.0f, max_freq);
}

void PitchDetector::set_confidence_threshold(float threshold) {
    confidence_threshold_ = std::clamp(threshold, 0.0f, 1.0f);
}

void PitchDetector::reset() {
    previous_pitch_ = 0.0f;
    std::fill(windowed_buffer_.begin(), windowed_buffer_.end(), 0.0f);
    std::fill(autocorr_buffer_.begin(), autocorr_buffer_.end(), 0.0f);
}

void PitchDetector::apply_window(const Sample* input, Sample* output, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        output[i] = input[i] * hanning_window_[i];
    }
}

void PitchDetector::compute_autocorrelation(const Sample* input, Sample* output, uint32_t size) {
    // Simple autocorrelation implementation
    // In a production system, you'd want to use FFT for better performance
    for (uint32_t lag = 0; lag < size; ++lag) {
        float sum = 0.0f;
        for (uint32_t i = 0; i < size - lag; ++i) {
            sum += input[i] * input[i + lag];
        }
        output[lag] = sum;
    }
}

uint32_t PitchDetector::find_autocorr_peak(const Sample* autocorr, uint32_t size, float& confidence) {
    if (size < 2) {
        confidence = 0.0f;
        return 0;
    }
    
    // Skip the first sample (lag = 0) which is always the maximum
    uint32_t min_lag = static_cast<uint32_t>(sample_rate_ / max_frequency_);
    uint32_t max_lag = static_cast<uint32_t>(sample_rate_ / min_frequency_);
    
    min_lag = std::max(1u, std::min(min_lag, size - 1));
    max_lag = std::min(max_lag, size - 1);
    
    if (min_lag >= max_lag) {
        confidence = 0.0f;
        return 0;
    }
    
    // Find maximum in valid range
    uint32_t peak_lag = min_lag;
    float peak_value = autocorr[min_lag];
    
    for (uint32_t lag = min_lag + 1; lag <= max_lag; ++lag) {
        if (autocorr[lag] > peak_value) {
            peak_value = autocorr[lag];
            peak_lag = lag;
        }
    }
    
    // Calculate confidence as ratio of peak to first value
    confidence = (autocorr[0] > 0.0f) ? (peak_value / autocorr[0]) : 0.0f;
    confidence = std::clamp(confidence, 0.0f, 1.0f);
    
    return peak_lag;
}

float PitchDetector::lag_to_frequency(uint32_t lag) const {
    if (lag == 0) return 0.0f;
    return static_cast<float>(sample_rate_) / static_cast<float>(lag);
}

float PitchDetector::smooth_pitch(float current_pitch) {
    if (previous_pitch_ == 0.0f) {
        previous_pitch_ = current_pitch;
        return current_pitch;
    }
    
    // Simple exponential smoothing
    float smoothed = pitch_smoothing_factor_ * previous_pitch_ + 
                    (1.0f - pitch_smoothing_factor_) * current_pitch;
    
    previous_pitch_ = smoothed;
    return smoothed;
}

} // namespace autotune
