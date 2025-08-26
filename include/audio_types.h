#pragma once

#include <vector>
#include <cstdint>

namespace autotune {

// Audio sample type (32-bit float for high quality)
using Sample = float;
using SampleRate = uint32_t;
using ChannelCount = uint32_t;

// Audio buffer structure for efficient processing
struct AudioFrame {
    std::vector<Sample> channels;
    
    AudioFrame(ChannelCount channel_count = 2) 
        : channels(channel_count, 0.0f) {}
    
    Sample& operator[](ChannelCount channel) { return channels[channel]; }
    const Sample& operator[](ChannelCount channel) const { return channels[channel]; }
    
    ChannelCount size() const { return static_cast<ChannelCount>(channels.size()); }
};

// Musical note representation
struct Note {
    float frequency;    // Hz
    int midi_note;     // MIDI note number (0-127)
    float cents;       // Cents deviation from equal temperament
    
    Note(float freq = 440.0f, int midi = 69, float cent_dev = 0.0f)
        : frequency(freq), midi_note(midi), cents(cent_dev) {}
};

// Processing parameters
struct ProcessingParams {
    SampleRate sample_rate = 44100;
    uint32_t buffer_size = 512;    // Samples per buffer (low latency)
    float correction_strength = 1.0f;  // 0.0 = no correction, 1.0 = full correction
    float attack_time = 0.01f;     // Seconds
    float release_time = 0.1f;     // Seconds
    bool enable_quantization = true;
    float quantize_strength = 0.8f;
};

// Audio processing result
struct ProcessingResult {
    bool success = false;
    float detected_pitch = 0.0f;    // Hz
    float corrected_pitch = 0.0f;   // Hz
    float confidence = 0.0f;        // 0.0 - 1.0
    uint32_t latency_samples = 0;   // Processing latency
};

} // namespace autotune
