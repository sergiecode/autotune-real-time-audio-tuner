#pragma once

#include "audio_types.h"
#include <vector>
#include <memory>

namespace autotune {

/**
 * @brief Real-time pitch detection using autocorrelation and harmonic analysis
 * 
 * This class implements efficient pitch detection algorithms optimized for
 * real-time performance with low latency.
 */
class PitchDetector {
public:
    /**
     * @brief Construct PitchDetector
     * @param sample_rate Audio sample rate
     * @param buffer_size Processing buffer size
     */
    PitchDetector(SampleRate sample_rate, uint32_t buffer_size = 512);
    
    /**
     * @brief Destructor
     */
    ~PitchDetector();
    
    /**
     * @brief Detect pitch from audio samples
     * @param samples Audio samples (mono)
     * @param sample_count Number of samples
     * @param confidence Output confidence level (0.0 - 1.0)
     * @return Detected frequency in Hz (0.0 if no pitch detected)
     */
    float detect_pitch(const Sample* samples, uint32_t sample_count, float& confidence);
    
    /**
     * @brief Detect pitch from audio frame
     * @param frame Audio frame (uses first channel for mono processing)
     * @param confidence Output confidence level
     * @return Detected frequency in Hz
     */
    float detect_pitch(const AudioFrame& frame, float& confidence);
    
    /**
     * @brief Set minimum detectable frequency
     * @param min_freq Minimum frequency in Hz
     */
    void set_min_frequency(float min_freq);
    
    /**
     * @brief Set maximum detectable frequency
     * @param max_freq Maximum frequency in Hz
     */
    void set_max_frequency(float max_freq);
    
    /**
     * @brief Set confidence threshold for pitch detection
     * @param threshold Minimum confidence (0.0 - 1.0)
     */
    void set_confidence_threshold(float threshold);
    
    /**
     * @brief Get current settings
     */
    float get_min_frequency() const { return min_frequency_; }
    float get_max_frequency() const { return max_frequency_; }
    float get_confidence_threshold() const { return confidence_threshold_; }
    
    /**
     * @brief Reset internal state
     */
    void reset();

private:
    SampleRate sample_rate_;
    uint32_t buffer_size_;
    float min_frequency_;
    float max_frequency_;
    float confidence_threshold_;
    
    // Internal buffers for processing
    std::vector<Sample> windowed_buffer_;
    std::vector<Sample> autocorr_buffer_;
    std::vector<float> hanning_window_;
    
    // Previous frame for smoothing
    float previous_pitch_;
    float pitch_smoothing_factor_;
    
    /**
     * @brief Apply Hanning window to input samples
     * @param input Input samples
     * @param output Windowed output
     * @param size Number of samples
     */
    void apply_window(const Sample* input, Sample* output, uint32_t size);
    
    /**
     * @brief Compute autocorrelation
     * @param input Input samples
     * @param output Autocorrelation result
     * @param size Number of samples
     */
    void compute_autocorrelation(const Sample* input, Sample* output, uint32_t size);
    
    /**
     * @brief Find peak in autocorrelation function
     * @param autocorr Autocorrelation data
     * @param size Data size
     * @param confidence Output confidence
     * @return Peak position (samples)
     */
    uint32_t find_autocorr_peak(const Sample* autocorr, uint32_t size, float& confidence);
    
    /**
     * @brief Convert lag to frequency
     * @param lag Lag in samples
     * @return Frequency in Hz
     */
    float lag_to_frequency(uint32_t lag) const;
    
    /**
     * @brief Smooth pitch estimate
     * @param current_pitch Current detected pitch
     * @return Smoothed pitch
     */
    float smooth_pitch(float current_pitch);
    
    // Non-copyable
    PitchDetector(const PitchDetector&) = delete;
    PitchDetector& operator=(const PitchDetector&) = delete;
};

} // namespace autotune
