#pragma once

#include "audio_types.h"
#include <vector>
#include <memory>

namespace autotune {

/**
 * @brief Real-time audio buffer for circular buffering and efficient processing
 * 
 * This class provides a lock-free circular buffer optimized for real-time audio
 * processing with minimal latency and memory allocations.
 */
class AudioBuffer {
public:
    /**
     * @brief Construct AudioBuffer
     * @param capacity Maximum number of audio frames to store
     * @param channels Number of audio channels
     */
    AudioBuffer(uint32_t capacity, ChannelCount channels = 2);
    
    /**
     * @brief Destructor
     */
    ~AudioBuffer();
    
    /**
     * @brief Write audio frames to the buffer
     * @param frames Pointer to audio frames
     * @param frame_count Number of frames to write
     * @return Number of frames actually written
     */
    uint32_t write(const AudioFrame* frames, uint32_t frame_count);
    
    /**
     * @brief Read audio frames from the buffer
     * @param frames Pointer to output buffer
     * @param frame_count Number of frames to read
     * @return Number of frames actually read
     */
    uint32_t read(AudioFrame* frames, uint32_t frame_count);
    
    /**
     * @brief Get number of available frames for reading
     * @return Available frame count
     */
    uint32_t available() const;
    
    /**
     * @brief Get remaining space for writing
     * @return Available space for writing
     */
    uint32_t space() const;
    
    /**
     * @brief Check if buffer is empty
     * @return True if empty
     */
    bool empty() const;
    
    /**
     * @brief Check if buffer is full
     * @return True if full
     */
    bool full() const;
    
    /**
     * @brief Clear the buffer
     */
    void clear();
    
    /**
     * @brief Get buffer capacity
     * @return Maximum number of frames
     */
    uint32_t capacity() const { return capacity_; }
    
    /**
     * @brief Get channel count
     * @return Number of audio channels
     */
    ChannelCount channels() const { return channels_; }

private:
    std::vector<AudioFrame> buffer_;
    uint32_t capacity_;
    ChannelCount channels_;
    volatile uint32_t read_pos_;
    volatile uint32_t write_pos_;
    
    // Non-copyable
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer& operator=(const AudioBuffer&) = delete;
};

} // namespace autotune
