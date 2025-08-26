#include "autotune_engine.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace autotune;

/**
 * @brief Generate a test sine wave
 * @param frequency Frequency in Hz
 * @param sample_rate Sample rate
 * @param duration_seconds Duration in seconds
 * @param amplitude Amplitude (0.0 - 1.0)
 * @return Vector of audio samples
 */
std::vector<Sample> generate_sine_wave(float frequency, SampleRate sample_rate, 
                                       float duration_seconds, float amplitude = 0.5f) {
    uint32_t sample_count = static_cast<uint32_t>(duration_seconds * sample_rate);
    std::vector<Sample> samples(sample_count);
    
    float angular_frequency = 2.0f * M_PI * frequency / sample_rate;
    
    for (uint32_t i = 0; i < sample_count; ++i) {
        samples[i] = amplitude * std::sin(angular_frequency * i);
    }
    
    return samples;
}

/**
 * @brief Convert mono samples to stereo AudioFrames
 * @param mono_samples Mono audio samples
 * @return Vector of stereo AudioFrames
 */
std::vector<AudioFrame> mono_to_stereo_frames(const std::vector<Sample>& mono_samples) {
    std::vector<AudioFrame> frames(mono_samples.size());
    
    for (size_t i = 0; i < mono_samples.size(); ++i) {
        frames[i] = AudioFrame(2); // Stereo
        frames[i][0] = mono_samples[i]; // Left channel
        frames[i][1] = mono_samples[i]; // Right channel (same as left)
    }
    
    return frames;
}

/**
 * @brief Print processing results
 * @param result Processing result from AutotuneEngine
 * @param frame_number Frame number for reference
 */
void print_processing_result(const ProcessingResult& result, uint32_t frame_number) {
    std::cout << "Frame " << std::setw(4) << frame_number << ": ";
    
    if (result.success) {
        std::cout << "✓ ";
        std::cout << "Detected: " << std::setw(6) << std::fixed << std::setprecision(1) 
                  << result.detected_pitch << " Hz, ";
        std::cout << "Corrected: " << std::setw(6) << std::fixed << std::setprecision(1) 
                  << result.corrected_pitch << " Hz, ";
        std::cout << "Confidence: " << std::setw(4) << std::fixed << std::setprecision(2) 
                  << result.confidence;
    } else {
        std::cout << "✗ Processing failed";
    }
    
    std::cout << std::endl;
}

/**
 * @brief Demonstrate basic pitch correction
 */
void demo_pitch_correction() {
    std::cout << "\n=== Pitch Correction Demo ===" << std::endl;
    
    // Configuration
    SampleRate sample_rate = 44100;
    uint32_t buffer_size = 512;
    ChannelCount channels = 2;
    
    // Create AutoTune engine
    AutotuneEngine engine(sample_rate, buffer_size, channels);
    
    if (!engine.is_initialized()) {
        std::cout << "Failed to initialize AutoTune engine!" << std::endl;
        return;
    }
    
    // Configure engine
    ProcessingParams params;
    params.sample_rate = sample_rate;
    params.buffer_size = buffer_size;
    params.correction_strength = 0.8f;
    params.quantize_strength = 0.9f;
    engine.set_parameters(params);
    
    // Set to C Major scale
    engine.set_scale(Quantizer::Scale::MAJOR, 60); // C4 as root
    engine.set_mode(AutotuneEngine::Mode::FULL_AUTOTUNE);
    
    std::cout << "Engine initialized successfully!" << std::endl;
    std::cout << "Sample Rate: " << sample_rate << " Hz" << std::endl;
    std::cout << "Buffer Size: " << buffer_size << " samples" << std::endl;
    std::cout << "Mode: Full AutoTune (Pitch Correction + Quantization)" << std::endl;
    std::cout << "Scale: C Major" << std::endl;
    
    // Generate test audio - slightly off-pitch note
    float test_frequency = 265.0f; // Slightly flat C4 (should be ~261.63 Hz)
    float duration = 1.0f; // 1 second
    
    std::cout << "\nGenerating test audio:" << std::endl;
    std::cout << "Input frequency: " << test_frequency << " Hz (slightly flat C4)" << std::endl;
    std::cout << "Expected correction to: ~261.63 Hz (perfect C4)" << std::endl;
    
    auto mono_samples = generate_sine_wave(test_frequency, sample_rate, duration);
    auto input_frames = mono_to_stereo_frames(mono_samples);
    
    // Prepare output buffer
    std::vector<AudioFrame> output_frames(input_frames.size());
    for (auto& frame : output_frames) {
        frame = AudioFrame(channels);
    }
    
    // Process audio in chunks
    std::cout << "\nProcessing audio..." << std::endl;
    uint32_t frames_processed = 0;
    uint32_t chunk_count = 0;
    
    for (size_t i = 0; i < input_frames.size(); i += buffer_size) {
        uint32_t frames_to_process = std::min(buffer_size, 
                                             static_cast<uint32_t>(input_frames.size() - i));
        
        ProcessingResult result = engine.process(&input_frames[i], &output_frames[i], 
                                               frames_to_process);
        
        // Print results for first few chunks
        if (chunk_count < 5) {
            print_processing_result(result, chunk_count);
        }
        
        frames_processed += frames_to_process;
        chunk_count++;
    }
    
    std::cout << "\nProcessing complete!" << std::endl;
    std::cout << "Total frames processed: " << frames_processed << std::endl;
    std::cout << "Total chunks: " << chunk_count << std::endl;
    
    // Show performance metrics
    auto metrics = engine.get_performance_metrics();
    std::cout << "\nPerformance Metrics:" << std::endl;
    std::cout << "Average latency: " << std::fixed << std::setprecision(2) 
              << metrics.average_latency_ms << " ms" << std::endl;
    std::cout << "CPU usage: " << std::fixed << std::setprecision(1) 
              << metrics.cpu_usage_percent << "%" << std::endl;
    std::cout << "Frames processed: " << metrics.frames_processed << std::endl;
}

/**
 * @brief Demonstrate different scales and modes
 */
void demo_scales_and_modes() {
    std::cout << "\n=== Scales and Modes Demo ===" << std::endl;
    
    AutotuneEngine engine(44100, 512, 2);
    
    if (!engine.is_initialized()) {
        std::cout << "Failed to initialize engine!" << std::endl;
        return;
    }
    
    // Test different scales
    std::vector<std::pair<Quantizer::Scale, std::string>> scales = {
        {Quantizer::Scale::MAJOR, "Major"},
        {Quantizer::Scale::MINOR, "Minor"},
        {Quantizer::Scale::PENTATONIC, "Pentatonic"},
        {Quantizer::Scale::BLUES, "Blues"}
    };
    
    // Test with a slightly off-pitch note
    float test_freq = 445.0f; // Slightly sharp A4 (should be 440 Hz)
    AudioFrame input_frame(2);
    input_frame[0] = 0.5f * std::sin(0); // Placeholder sample
    input_frame[1] = input_frame[0];
    
    AudioFrame output_frame(2);
    
    std::cout << "Input frequency: " << test_freq << " Hz (slightly sharp A4)" << std::endl;
    std::cout << "\nQuantization results for different scales:" << std::endl;
    
    for (const auto& scale_pair : scales) {
        engine.set_scale(scale_pair.first, 60); // C as root
        engine.set_mode(AutotuneEngine::Mode::QUANTIZATION);
        
        ProcessingResult result = engine.process_frame(input_frame, output_frame);
        
        std::cout << "Scale: " << std::setw(12) << scale_pair.second;
        if (result.success) {
            std::cout << " -> " << std::fixed << std::setprecision(2) 
                      << result.corrected_pitch << " Hz";
        } else {
            std::cout << " -> Processing failed";
        }
        std::cout << std::endl;
    }
}

/**
 * @brief Demonstrate real-time processing simulation
 */
void demo_realtime_simulation() {
    std::cout << "\n=== Real-Time Processing Simulation ===" << std::endl;
    
    AutotuneEngine engine(44100, 256, 2); // Smaller buffer for lower latency
    
    if (!engine.is_initialized()) {
        std::cout << "Failed to initialize engine!" << std::endl;
        return;
    }
    
    engine.set_mode(AutotuneEngine::Mode::PITCH_CORRECTION);
    
    ProcessingParams params;
    params.correction_strength = 1.0f; // Full correction
    params.attack_time = 0.005f; // 5ms attack
    params.release_time = 0.05f; // 50ms release
    engine.set_parameters(params);
    
    std::cout << "Simulating real-time processing with 256-sample buffers..." << std::endl;
    std::cout << "Buffer duration: " << (256.0f / 44100.0f * 1000.0f) 
              << " ms (low latency)" << std::endl;
    
    // Simulate processing several buffers
    AudioFrame input_frame(2);
    AudioFrame output_frame(2);
    
    for (int i = 0; i < 10; ++i) {
        // Simulate input audio (varying pitch)
        float phase = i * 0.5f;
        input_frame[0] = 0.3f * std::sin(phase);
        input_frame[1] = input_frame[0];
        
        auto start = std::chrono::high_resolution_clock::now();
        ProcessingResult result = engine.process_frame(input_frame, output_frame);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Buffer " << std::setw(2) << i + 1 << ": " 
                  << duration.count() << " μs";
        
        if (result.success) {
            std::cout << " ✓";
        } else {
            std::cout << " ✗";
        }
        
        std::cout << std::endl;
    }
    
    auto metrics = engine.get_performance_metrics();
    std::cout << "\nFinal performance:" << std::endl;
    std::cout << "Average latency: " << metrics.average_latency_ms << " ms" << std::endl;
    std::cout << "CPU usage: " << metrics.cpu_usage_percent << "%" << std::endl;
}

/**
 * @brief Demonstrate ML model integration
 */
void demo_ml_integration() {
    std::cout << "\n=== ML Model Integration Demo ===" << std::endl;
    
    // Create engine
    AutotuneEngine engine(44100, 512, 1);
    
    std::cout << "ML model info: " << engine.get_ml_model_info() << std::endl;
    
    // Try to load ML model
    bool ml_loaded = engine.load_ml_model("models/pitch_corrector.onnx");
    if (ml_loaded) {
        std::cout << "✅ ML model loaded successfully!" << std::endl;
        engine.set_ml_processing_enabled(true);
        
        std::cout << "ML processing enabled: " << (engine.is_ml_processing_enabled() ? "Yes" : "No") << std::endl;
        std::cout << "Updated model info: " << engine.get_ml_model_info() << std::endl;
        
        // Generate test audio (slightly off-pitch)
        auto test_samples = generate_sine_wave(439.2f, 44100, 0.1f); // Slightly flat A
        auto test_frames = mono_to_stereo_frames(test_samples);
        
        std::cout << "\nProcessing with ML model..." << std::endl;
        
        // Process a few frames
        for (int i = 0; i < 5 && i < static_cast<int>(test_frames.size()); ++i) {
            AudioFrame output(1);
            auto start = std::chrono::high_resolution_clock::now();
            ProcessingResult result = engine.process_frame(test_frames[i], output);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            float time_ms = duration.count() / 1000.0f;
            
            std::cout << "Frame " << i << ": " << std::fixed << std::setprecision(3)
                      << time_ms << "ms, confidence: " << result.confidence 
                      << " (ML enhanced)" << std::endl;
        }
        
        // Compare performance with traditional processing
        engine.set_ml_processing_enabled(false);
        std::cout << "\nComparing with traditional processing..." << std::endl;
        
        for (int i = 0; i < 5 && i < static_cast<int>(test_frames.size()); ++i) {
            AudioFrame output(1);
            auto start = std::chrono::high_resolution_clock::now();
            ProcessingResult result = engine.process_frame(test_frames[i], output);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            float time_ms = duration.count() / 1000.0f;
            
            std::cout << "Frame " << i << ": " << std::fixed << std::setprecision(3)
                      << time_ms << "ms, confidence: " << result.confidence 
                      << " (traditional)" << std::endl;
        }
        
    } else {
        std::cout << "ℹ️ No ML model found - using traditional processing" << std::endl;
        std::cout << "To enable ML processing:" << std::endl;
        std::cout << "1. Train a model using autotune-audio-ml-trainer" << std::endl;
        std::cout << "2. Export to ONNX format" << std::endl;
        std::cout << "3. Copy to models/pitch_corrector.onnx" << std::endl;
        std::cout << "4. Rebuild with ONNX Runtime support" << std::endl;
    }
}

int main() {
    std::cout << "AutoTune Real-Time Audio Engine Demo" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Version 1.0.0" << std::endl;
    std::cout << "Created by Sergie Code" << std::endl;
    
    try {
        // Run all demos
        demo_pitch_correction();
        demo_scales_and_modes();
        demo_realtime_simulation();
        demo_ml_integration();
        
        std::cout << "\n=== Demo Complete ===" << std::endl;
        std::cout << "All demos completed successfully!" << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "1. Integrate with real audio I/O (ASIO, JACK, etc.)" << std::endl;
        std::cout << "2. Train ML models using autotune-audio-ml-trainer" << std::endl;
        std::cout << "3. Deploy ONNX models to models/ directory" << std::endl;
        std::cout << "4. Build GUI interface" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during demo: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
