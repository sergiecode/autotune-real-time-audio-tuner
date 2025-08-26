#pragma once

#include "audio_types.h"
#include <vector>
#include <array>

namespace autotune {

/**
 * @brief Musical quantizer for rhythm and pitch quantization
 * 
 * This class provides quantization capabilities for both rhythmic
 * and pitch elements to help musicians stay in time and in tune.
 */
class Quantizer {
public:
    /**
     * @brief Musical scales for pitch quantization
     */
    enum class Scale {
        CHROMATIC,      // All 12 semitones
        MAJOR,          // Major scale (Ionian)
        MINOR,          // Natural minor scale (Aeolian)
        PENTATONIC,     // Pentatonic scale
        BLUES,          // Blues scale
        DORIAN,         // Dorian mode
        MIXOLYDIAN,     // Mixolydian mode
        CUSTOM          // User-defined scale
    };
    
    /**
     * @brief Time signatures for rhythmic quantization
     */
    enum class TimeSignature {
        FOUR_FOUR,      // 4/4 time
        THREE_FOUR,     // 3/4 time
        TWO_FOUR,       // 2/4 time
        SIX_EIGHT,      // 6/8 time
        TWELVE_EIGHT    // 12/8 time
    };
    
    /**
     * @brief Quantization grid resolution
     */
    enum class GridResolution {
        QUARTER_NOTE,   // 1/4 notes
        EIGHTH_NOTE,    // 1/8 notes
        SIXTEENTH_NOTE, // 1/16 notes
        TRIPLET,        // Triplet subdivision
        DOTTED          // Dotted rhythm
    };
    
    /**
     * @brief Construct Quantizer
     * @param sample_rate Audio sample rate
     * @param tempo Initial tempo in BPM
     */
    Quantizer(SampleRate sample_rate, float tempo = 120.0f);
    
    /**
     * @brief Destructor
     */
    ~Quantizer();
    
    /**
     * @brief Quantize pitch to musical scale
     * @param input_pitch Input frequency (Hz)
     * @param scale Musical scale to quantize to
     * @param key_center Root note of the scale (MIDI note number)
     * @param strength Quantization strength (0.0 - 1.0)
     * @return Quantized frequency (Hz)
     */
    float quantize_pitch(float input_pitch, Scale scale, int key_center = 60, 
                        float strength = 1.0f);
    
    /**
     * @brief Quantize timing to rhythmic grid
     * @param input_time Input time position (samples)
     * @param grid_resolution Rhythmic grid resolution
     * @param strength Quantization strength (0.0 - 1.0)
     * @return Quantized time position (samples)
     */
    uint32_t quantize_timing(uint32_t input_time, GridResolution grid_resolution,
                           float strength = 1.0f);
    
    /**
     * @brief Set tempo for rhythmic quantization
     * @param tempo Tempo in beats per minute
     */
    void set_tempo(float tempo);
    
    /**
     * @brief Set time signature
     * @param time_sig Time signature
     */
    void set_time_signature(TimeSignature time_sig);
    
    /**
     * @brief Set custom scale intervals
     * @param intervals Scale intervals in semitones from root
     */
    void set_custom_scale(const std::vector<int>& intervals);
    
    /**
     * @brief Get nearest note in scale
     * @param input_pitch Input frequency (Hz)
     * @param scale Musical scale
     * @param key_center Root note (MIDI)
     * @return Nearest note information
     */
    Note get_nearest_note(float input_pitch, Scale scale, int key_center = 60);
    
    /**
     * @brief Convert frequency to MIDI note number
     * @param frequency Frequency in Hz
     * @return MIDI note number (69 = A4 = 440Hz)
     */
    static float frequency_to_midi(float frequency);
    
    /**
     * @brief Convert MIDI note number to frequency
     * @param midi_note MIDI note number
     * @return Frequency in Hz
     */
    static float midi_to_frequency(float midi_note);
    
    /**
     * @brief Get current tempo
     * @return Tempo in BPM
     */
    float get_tempo() const { return tempo_; }
    
    /**
     * @brief Get current time signature
     * @return Current time signature
     */
    TimeSignature get_time_signature() const { return time_signature_; }
    
    /**
     * @brief Reset quantizer state
     */
    void reset();

private:
    SampleRate sample_rate_;
    float tempo_;
    TimeSignature time_signature_;
    
    // Scale definitions (intervals in semitones)
    std::array<std::vector<int>, 8> scale_intervals_;
    std::vector<int> custom_scale_intervals_;
    
    // Timing calculations
    float samples_per_beat_;
    float samples_per_measure_;
    
    /**
     * @brief Initialize built-in scales
     */
    void initialize_scales();
    
    /**
     * @brief Update timing calculations
     */
    void update_timing();
    
    /**
     * @brief Get scale intervals for given scale type
     * @param scale Scale type
     * @return Vector of intervals in semitones
     */
    const std::vector<int>& get_scale_intervals(Scale scale) const;
    
    /**
     * @brief Find nearest note in scale
     * @param midi_note Input MIDI note (can be fractional)
     * @param intervals Scale intervals
     * @param key_center Root note
     * @return Nearest MIDI note in scale
     */
    float find_nearest_scale_note(float midi_note, const std::vector<int>& intervals,
                                 int key_center) const;
    
    /**
     * @brief Calculate grid position for timing quantization
     * @param time_samples Time position in samples
     * @param resolution Grid resolution
     * @return Grid position
     */
    float calculate_grid_position(uint32_t time_samples, GridResolution resolution) const;
    
    /**
     * @brief Get samples per grid unit
     * @param resolution Grid resolution
     * @return Samples per grid unit
     */
    float get_samples_per_grid(GridResolution resolution) const;
    
    // Non-copyable
    Quantizer(const Quantizer&) = delete;
    Quantizer& operator=(const Quantizer&) = delete;
};

} // namespace autotune
