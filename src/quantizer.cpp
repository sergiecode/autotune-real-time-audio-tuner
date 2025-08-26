#include "quantizer.h"
#include <algorithm>
#include <cmath>

namespace autotune {

Quantizer::Quantizer(SampleRate sample_rate, float tempo)
    : sample_rate_(sample_rate), tempo_(tempo), time_signature_(TimeSignature::FOUR_FOUR) {
    
    initialize_scales();
    update_timing();
}

Quantizer::~Quantizer() = default;

float Quantizer::quantize_pitch(float input_pitch, Scale scale, int key_center, float strength) {
    if (input_pitch <= 0.0f || strength <= 0.0f) {
        return input_pitch;
    }
    
    // Convert frequency to MIDI note
    float input_midi = frequency_to_midi(input_pitch);
    
    // Get scale intervals
    const auto& intervals = get_scale_intervals(scale);
    
    // Find nearest note in scale
    float quantized_midi = find_nearest_scale_note(input_midi, intervals, key_center);
    
    // Apply quantization strength
    float result_midi = input_midi + strength * (quantized_midi - input_midi);
    
    // Convert back to frequency
    return midi_to_frequency(result_midi);
}

uint32_t Quantizer::quantize_timing(uint32_t input_time, GridResolution grid_resolution, float strength) {
    if (strength <= 0.0f) {
        return input_time;
    }
    
    // Calculate grid position
    float grid_position = calculate_grid_position(input_time, grid_resolution);
    
    // Find nearest grid point
    float nearest_grid = std::round(grid_position);
    
    // Apply quantization strength
    float quantized_position = grid_position + strength * (nearest_grid - grid_position);
    
    // Convert back to samples
    float samples_per_grid = get_samples_per_grid(grid_resolution);
    uint32_t quantized_time = static_cast<uint32_t>(quantized_position * samples_per_grid);
    
    return quantized_time;
}

void Quantizer::set_tempo(float tempo) {
    tempo_ = std::clamp(tempo, 60.0f, 200.0f);
    update_timing();
}

void Quantizer::set_time_signature(TimeSignature time_sig) {
    time_signature_ = time_sig;
    update_timing();
}

void Quantizer::set_custom_scale(const std::vector<int>& intervals) {
    custom_scale_intervals_ = intervals;
    std::sort(custom_scale_intervals_.begin(), custom_scale_intervals_.end());
}

Note Quantizer::get_nearest_note(float input_pitch, Scale scale, int key_center) {
    if (input_pitch <= 0.0f) {
        return Note(0.0f, 0, 0.0f);
    }
    
    float input_midi = frequency_to_midi(input_pitch);
    const auto& intervals = get_scale_intervals(scale);
    float quantized_midi = find_nearest_scale_note(input_midi, intervals, key_center);
    float quantized_freq = midi_to_frequency(quantized_midi);
    
    // Calculate cents deviation
    float cents = 1200.0f * std::log2(input_pitch / quantized_freq);
    
    return Note(quantized_freq, static_cast<int>(quantized_midi), cents);
}

float Quantizer::frequency_to_midi(float frequency) {
    if (frequency <= 0.0f) return 0.0f;
    return 69.0f + 12.0f * std::log2(frequency / 440.0f);
}

float Quantizer::midi_to_frequency(float midi_note) {
    return 440.0f * std::pow(2.0f, (midi_note - 69.0f) / 12.0f);
}

void Quantizer::reset() {
    // Reset any internal state if needed
}

void Quantizer::initialize_scales() {
    // Chromatic scale (all semitones)
    scale_intervals_[0] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    
    // Major scale (Ionian)
    scale_intervals_[1] = {0, 2, 4, 5, 7, 9, 11};
    
    // Natural minor scale (Aeolian)
    scale_intervals_[2] = {0, 2, 3, 5, 7, 8, 10};
    
    // Pentatonic major
    scale_intervals_[3] = {0, 2, 4, 7, 9};
    
    // Blues scale
    scale_intervals_[4] = {0, 3, 5, 6, 7, 10};
    
    // Dorian mode
    scale_intervals_[5] = {0, 2, 3, 5, 7, 9, 10};
    
    // Mixolydian mode
    scale_intervals_[6] = {0, 2, 4, 5, 7, 9, 10};
    
    // Custom scale (initialized empty)
    scale_intervals_[7] = {};
}

void Quantizer::update_timing() {
    // Calculate samples per beat based on tempo
    samples_per_beat_ = (60.0f / tempo_) * sample_rate_;
    
    // Calculate samples per measure based on time signature
    float beats_per_measure = 4.0f; // Default for 4/4
    
    switch (time_signature_) {
        case TimeSignature::FOUR_FOUR:
            beats_per_measure = 4.0f;
            break;
        case TimeSignature::THREE_FOUR:
            beats_per_measure = 3.0f;
            break;
        case TimeSignature::TWO_FOUR:
            beats_per_measure = 2.0f;
            break;
        case TimeSignature::SIX_EIGHT:
            beats_per_measure = 6.0f;
            samples_per_beat_ /= 2.0f; // Eighth note gets the beat
            break;
        case TimeSignature::TWELVE_EIGHT:
            beats_per_measure = 12.0f;
            samples_per_beat_ /= 2.0f; // Eighth note gets the beat
            break;
    }
    
    samples_per_measure_ = samples_per_beat_ * beats_per_measure;
}

const std::vector<int>& Quantizer::get_scale_intervals(Scale scale) const {
    switch (scale) {
        case Scale::CHROMATIC: return scale_intervals_[0];
        case Scale::MAJOR: return scale_intervals_[1];
        case Scale::MINOR: return scale_intervals_[2];
        case Scale::PENTATONIC: return scale_intervals_[3];
        case Scale::BLUES: return scale_intervals_[4];
        case Scale::DORIAN: return scale_intervals_[5];
        case Scale::MIXOLYDIAN: return scale_intervals_[6];
        case Scale::CUSTOM: return custom_scale_intervals_;
        default: return scale_intervals_[0]; // Default to chromatic
    }
}

float Quantizer::find_nearest_scale_note(float midi_note, const std::vector<int>& intervals, int key_center) const {
    if (intervals.empty()) {
        return midi_note; // No quantization if no scale intervals
    }
    
    // Normalize to octave starting from key center
    float relative_note = midi_note - key_center;
    int octave = static_cast<int>(std::floor(relative_note / 12.0f));
    float note_in_octave = relative_note - octave * 12.0f;
    
    // Find nearest interval in scale
    float nearest_interval = intervals[0];
    float min_distance = std::abs(note_in_octave - intervals[0]);
    
    for (int interval : intervals) {
        float distance = std::abs(note_in_octave - interval);
        if (distance < min_distance) {
            min_distance = distance;
            nearest_interval = interval;
        }
        
        // Check octave above
        float upper_distance = std::abs(note_in_octave - (interval + 12));
        if (upper_distance < min_distance) {
            min_distance = upper_distance;
            nearest_interval = interval + 12;
        }
    }
    
    // Reconstruct MIDI note
    return key_center + octave * 12.0f + nearest_interval;
}

float Quantizer::calculate_grid_position(uint32_t time_samples, GridResolution resolution) const {
    float samples_per_grid = get_samples_per_grid(resolution);
    return static_cast<float>(time_samples) / samples_per_grid;
}

float Quantizer::get_samples_per_grid(GridResolution resolution) const {
    switch (resolution) {
        case GridResolution::QUARTER_NOTE:
            return samples_per_beat_;
        case GridResolution::EIGHTH_NOTE:
            return samples_per_beat_ / 2.0f;
        case GridResolution::SIXTEENTH_NOTE:
            return samples_per_beat_ / 4.0f;
        case GridResolution::TRIPLET:
            return samples_per_beat_ / 3.0f;
        case GridResolution::DOTTED:
            return samples_per_beat_ * 1.5f; // Dotted quarter
        default:
            return samples_per_beat_;
    }
}

} // namespace autotune
