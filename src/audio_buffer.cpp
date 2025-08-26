#include "audio_buffer.h"
#include <algorithm>
#include <cstring>

namespace autotune {

AudioBuffer::AudioBuffer(uint32_t capacity, ChannelCount channels)
    : capacity_(capacity), channels_(channels), read_pos_(0), write_pos_(0) {
    buffer_.resize(capacity);
    for (auto& frame : buffer_) {
        frame = AudioFrame(channels);
    }
}

AudioBuffer::~AudioBuffer() = default;

uint32_t AudioBuffer::write(const AudioFrame* frames, uint32_t frame_count) {
    if (!frames || frame_count == 0) return 0;
    
    uint32_t written = 0;
    uint32_t current_write = write_pos_;
    
    for (uint32_t i = 0; i < frame_count && !full(); ++i) {
        // Copy frame data
        for (ChannelCount ch = 0; ch < channels_ && ch < frames[i].size(); ++ch) {
            buffer_[current_write][ch] = frames[i][ch];
        }
        
        current_write = (current_write + 1) % capacity_;
        ++written;
    }
    
    write_pos_ = current_write;
    return written;
}

uint32_t AudioBuffer::read(AudioFrame* frames, uint32_t frame_count) {
    if (!frames || frame_count == 0) return 0;
    
    uint32_t read = 0;
    uint32_t current_read = read_pos_;
    
    for (uint32_t i = 0; i < frame_count && !empty(); ++i) {
        // Copy frame data
        frames[i] = AudioFrame(channels_);
        for (ChannelCount ch = 0; ch < channels_; ++ch) {
            frames[i][ch] = buffer_[current_read][ch];
        }
        
        current_read = (current_read + 1) % capacity_;
        ++read;
    }
    
    read_pos_ = current_read;
    return read;
}

uint32_t AudioBuffer::available() const {
    if (write_pos_ >= read_pos_) {
        return write_pos_ - read_pos_;
    } else {
        return capacity_ - read_pos_ + write_pos_;
    }
}

uint32_t AudioBuffer::space() const {
    return capacity_ - available() - 1; // -1 to distinguish full from empty
}

bool AudioBuffer::empty() const {
    return read_pos_ == write_pos_;
}

bool AudioBuffer::full() const {
    return ((write_pos_ + 1) % capacity_) == read_pos_;
}

void AudioBuffer::clear() {
    read_pos_ = 0;
    write_pos_ = 0;
    
    // Clear buffer contents
    for (auto& frame : buffer_) {
        for (ChannelCount ch = 0; ch < channels_; ++ch) {
            frame[ch] = 0.0f;
        }
    }
}

} // namespace autotune
