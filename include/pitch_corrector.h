#pragma once

#include "audio_types.h"
#include <vector>

namespace autotune {

/**
 * @brief Real-time pitch correction with smooth transitions
 * 
 * This class implements pitch shifting and correction algorithms
 * optimized for musical applications with minimal artifacts.
 */
class PitchCorrector {
public:
    /**
     * @brief Construct PitchCorrector
     * @param sample_rate Audio sample rate
     * @param buffer_size Processing buffer size
     */
    PitchCorrector(SampleRate sample_rate, uint32_t buffer_size = 512);
    
    /**
     * @brief Destructor
     */
    ~PitchCorrector();
    
    /**
     * @brief Correct pitch of audio samples
     * @param input Input audio samples
     * @param output Output corrected samples
     * @param sample_count Number of samples to process
     * @param input_pitch Detected input pitch (Hz)
     * @param target_pitch Target pitch (Hz)
     * @param correction_strength Correction amount (0.0 - 1.0)
     * @return Processing result
     */
    ProcessingResult correct_pitch(const Sample* input, Sample* output, 
                                 uint32_t sample_count,
                                 float input_pitch, float target_pitch,
                                 float correction_strength = 1.0f);
    
    /**
     * @brief Correct pitch of audio frame
     * @param input Input audio frame
     * @param output Output corrected frame
     * @param input_pitch Detected input pitch (Hz)
     * @param target_pitch Target pitch (Hz)
     * @param correction_strength Correction amount (0.0 - 1.0)
     * @return Processing result
     */
    ProcessingResult correct_pitch(const AudioFrame& input, AudioFrame& output,
                                 float input_pitch, float target_pitch,
                                 float correction_strength = 1.0f);
    
    /**
     * @brief Set processing parameters
     * @param params Processing parameters
     */
    void set_parameters(const ProcessingParams& params);
    
    /**
     * @brief Get current parameters
     * @return Current processing parameters
     */
    const ProcessingParams& get_parameters() const { return params_; }
    
    /**
     * @brief Set formant preservation (affects voice quality)
     * @param preserve True to preserve formants
     */
    void set_formant_preservation(bool preserve);
    
    /**
     * @brief Reset internal state
     */
    void reset();

private:
    SampleRate sample_rate_;
    uint32_t buffer_size_;
    ProcessingParams params_;
    bool preserve_formants_;
    
    // PSOLA (Pitch Synchronous Overlap and Add) state
    std::vector<Sample> overlap_buffer_;
    std::vector<Sample> grain_buffer_;
    std::vector<float> window_;
    
    // Pitch shifting state
    float phase_accumulator_;
    float previous_input_pitch_;
    float previous_target_pitch_;
    uint32_t grain_size_;
    uint32_t hop_size_;
    uint32_t overlap_size_;
    
    // Envelope following for smooth transitions
    float envelope_follower_;
    float attack_coeff_;
    float release_coeff_;
    
    /**
     * @brief Initialize processing parameters
     */
    void initialize_parameters();
    
    /**
     * @brief Calculate pitch shift ratio
     * @param input_pitch Input frequency
     * @param target_pitch Target frequency
     * @param strength Correction strength
     * @return Pitch shift ratio
     */
    float calculate_pitch_ratio(float input_pitch, float target_pitch, float strength);
    
    /**
     * @brief Apply time-domain pitch shifting using PSOLA
     * @param input Input samples
     * @param output Output samples
     * @param sample_count Number of samples
     * @param pitch_ratio Pitch shift ratio
     * @return Processing success
     */
    bool apply_psola_shift(const Sample* input, Sample* output, 
                          uint32_t sample_count, float pitch_ratio);
    
    /**
     * @brief Find pitch marks for PSOLA
     * @param samples Input samples
     * @param sample_count Number of samples
     * @param pitch Current pitch estimate
     * @param marks Output pitch marks
     * @return Number of marks found
     */
    uint32_t find_pitch_marks(const Sample* samples, uint32_t sample_count,
                             float pitch, std::vector<uint32_t>& marks);
    
    /**
     * @brief Apply windowing function
     * @param data Input/output data
     * @param size Window size
     * @param window_type Window type (0=Hanning, 1=Hamming)
     */
    void apply_window(Sample* data, uint32_t size, int window_type = 0);
    
    /**
     * @brief Update envelope follower
     * @param input Current input level
     * @return Envelope value
     */
    float update_envelope(float input);
    
    // Non-copyable
    PitchCorrector(const PitchCorrector&) = delete;
    PitchCorrector& operator=(const PitchCorrector&) = delete;
};

} // namespace autotune
