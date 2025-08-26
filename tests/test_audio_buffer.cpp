#include "audio_buffer.h"
#include <iostream>

extern class TestRunner;

void test_audio_buffer() {
    using namespace autotune;
    
    // Test 1: Basic construction
    {
        AudioBuffer buffer(100, 2);
        TestRunner::run_test("AudioBuffer construction", 
                           buffer.capacity() == 100 && buffer.channels() == 2);
        TestRunner::run_test("AudioBuffer initial state - empty", buffer.empty());
        TestRunner::run_test("AudioBuffer initial state - not full", !buffer.full());
        TestRunner::run_test("AudioBuffer initial available", buffer.available() == 0);
    }
    
    // Test 2: Write and read operations
    {
        AudioBuffer buffer(10, 2);
        
        // Create test frames
        std::vector<AudioFrame> test_frames(5);
        for (size_t i = 0; i < test_frames.size(); ++i) {
            test_frames[i] = AudioFrame(2);
            test_frames[i][0] = static_cast<float>(i) * 0.1f;
            test_frames[i][1] = static_cast<float>(i) * 0.2f;
        }
        
        // Write frames
        uint32_t written = buffer.write(test_frames.data(), 5);
        TestRunner::run_test("AudioBuffer write operation", written == 5);
        TestRunner::run_test("AudioBuffer available after write", buffer.available() == 5);
        TestRunner::run_test("AudioBuffer not empty after write", !buffer.empty());
        
        // Read frames back
        std::vector<AudioFrame> read_frames(5);
        for (auto& frame : read_frames) {
            frame = AudioFrame(2);
        }
        
        uint32_t read = buffer.read(read_frames.data(), 5);
        TestRunner::run_test("AudioBuffer read operation", read == 5);
        TestRunner::run_test("AudioBuffer empty after read", buffer.empty());
        
        // Verify data integrity
        bool data_matches = true;
        for (size_t i = 0; i < 5; ++i) {
            if (std::abs(read_frames[i][0] - test_frames[i][0]) > 1e-6f ||
                std::abs(read_frames[i][1] - test_frames[i][1]) > 1e-6f) {
                data_matches = false;
                break;
            }
        }
        TestRunner::run_test("AudioBuffer data integrity", data_matches);
    }
    
    // Test 3: Circular buffer behavior
    {
        AudioBuffer buffer(5, 1);
        
        // Fill buffer completely
        std::vector<AudioFrame> frames(6); // More than capacity
        for (size_t i = 0; i < frames.size(); ++i) {
            frames[i] = AudioFrame(1);
            frames[i][0] = static_cast<float>(i);
        }
        
        uint32_t written = buffer.write(frames.data(), 6);
        TestRunner::run_test("AudioBuffer overflow protection", written < 6);
        TestRunner::run_test("AudioBuffer full after overflow", buffer.full());
    }
    
    // Test 4: Clear operation
    {
        AudioBuffer buffer(10, 2);
        
        // Add some data
        std::vector<AudioFrame> frames(3);
        for (auto& frame : frames) {
            frame = AudioFrame(2);
            frame[0] = 1.0f;
            frame[1] = 2.0f;
        }
        
        buffer.write(frames.data(), 3);
        TestRunner::run_test("AudioBuffer has data before clear", buffer.available() == 3);
        
        buffer.clear();
        TestRunner::run_test("AudioBuffer empty after clear", buffer.empty());
        TestRunner::run_test("AudioBuffer available zero after clear", buffer.available() == 0);
    }
    
    // Test 5: Edge cases
    {
        AudioBuffer buffer(10, 2);
        
        // Test null pointer handling
        uint32_t written = buffer.write(nullptr, 5);
        TestRunner::run_test("AudioBuffer null write protection", written == 0);
        
        uint32_t read = buffer.read(nullptr, 5);
        TestRunner::run_test("AudioBuffer null read protection", read == 0);
        
        // Test zero count
        std::vector<AudioFrame> frames(1);
        frames[0] = AudioFrame(2);
        written = buffer.write(frames.data(), 0);
        TestRunner::run_test("AudioBuffer zero write count", written == 0);
        
        read = buffer.read(frames.data(), 0);
        TestRunner::run_test("AudioBuffer zero read count", read == 0);
    }
}
