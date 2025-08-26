#include "pitch_detector.h"
#include <iostream>
#include <cmath>

extern class TestRunner;

void test_pitch_detector() {
    using namespace autotune;
    
    // Test 1: Basic construction
    {
        PitchDetector detector(44100, 512);
        TestRunner::run_test("PitchDetector construction", true);
        TestRunner::run_test("PitchDetector frequency range", 
                           detector.get_min_frequency() > 0 && 
                           detector.get_max_frequency() > detector.get_min_frequency());
    }
    
    // Test 2: Configuration
    {
        PitchDetector detector(44100, 512);
        
        detector.set_min_frequency(100.0f);
        detector.set_max_frequency(1000.0f);
        detector.set_confidence_threshold(0.5f);
        
        TestRunner::run_test("PitchDetector min frequency setting", 
                           detector.get_min_frequency() == 100.0f);
        TestRunner::run_test("PitchDetector max frequency setting", 
                           detector.get_max_frequency() == 1000.0f);
        TestRunner::run_test("PitchDetector confidence threshold setting", 
                           detector.get_confidence_threshold() == 0.5f);
    }
    
    // Test 3: Pitch detection with synthetic sine wave
    {
        PitchDetector detector(44100, 512);
        detector.set_min_frequency(200.0f);
        detector.set_max_frequency(800.0f);
        
        // Generate 440 Hz sine wave
        std::vector<Sample> samples(512);
        float frequency = 440.0f;
        float sample_rate = 44100.0f;
        
        for (size_t i = 0; i < samples.size(); ++i) {
            samples[i] = 0.5f * std::sin(2.0f * M_PI * frequency * i / sample_rate);
        }
        
        float confidence = 0.0f;
        float detected_pitch = detector.detect_pitch(samples.data(), 
                                                   static_cast<uint32_t>(samples.size()), 
                                                   confidence);
        
        // Allow for some tolerance in detection
        bool pitch_close = std::abs(detected_pitch - frequency) < 10.0f;
        TestRunner::run_test("PitchDetector sine wave detection", 
                           detected_pitch > 0 && pitch_close,
                           "Detected: " + std::to_string(detected_pitch) + " Hz, Expected: " + std::to_string(frequency) + " Hz");
        
        TestRunner::run_test("PitchDetector confidence output", confidence >= 0.0f && confidence <= 1.0f);
    }
    
    // Test 4: AudioFrame detection
    {
        PitchDetector detector(44100, 512);
        
        AudioFrame frame(2);
        frame[0] = 0.5f;
        frame[1] = 0.3f;
        
        float confidence = 0.0f;
        float pitch = detector.detect_pitch(frame, confidence);
        
        // Should handle single frame input gracefully
        TestRunner::run_test("PitchDetector AudioFrame input", pitch >= 0.0f);
        TestRunner::run_test("PitchDetector AudioFrame confidence", confidence >= 0.0f && confidence <= 1.0f);
    }
    
    // Test 5: Edge cases
    {
        PitchDetector detector(44100, 512);
        
        // Test with null input
        float confidence = 0.0f;
        float pitch = detector.detect_pitch(nullptr, 512, confidence);
        TestRunner::run_test("PitchDetector null input protection", pitch == 0.0f && confidence == 0.0f);
        
        // Test with zero samples
        std::vector<Sample> samples(10, 0.0f);
        pitch = detector.detect_pitch(samples.data(), 0, confidence);
        TestRunner::run_test("PitchDetector zero sample count", pitch == 0.0f && confidence == 0.0f);
        
        // Test reset functionality
        detector.reset();
        TestRunner::run_test("PitchDetector reset", true); // Just check it doesn't crash
    }
    
    // Test 6: Frequency range validation
    {
        PitchDetector detector(44100, 512);
        detector.set_min_frequency(200.0f);
        detector.set_max_frequency(800.0f);
        
        // Generate frequency outside range (100 Hz)
        std::vector<Sample> samples(512);
        float frequency = 100.0f; // Below min frequency
        float sample_rate = 44100.0f;
        
        for (size_t i = 0; i < samples.size(); ++i) {
            samples[i] = 0.5f * std::sin(2.0f * M_PI * frequency * i / sample_rate);
        }
        
        float confidence = 0.0f;
        float detected_pitch = detector.detect_pitch(samples.data(), 
                                                   static_cast<uint32_t>(samples.size()), 
                                                   confidence);
        
        TestRunner::run_test("PitchDetector frequency range filtering", 
                           detected_pitch == 0.0f || (detected_pitch >= 200.0f && detected_pitch <= 800.0f));
    }
}
