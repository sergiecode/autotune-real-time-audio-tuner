#include "quantizer.h"
#include "autotune_engine.h"
#include <iostream>
#include <cmath>

extern class TestRunner;

void test_quantizer() {
    using namespace autotune;
    
    // Test 1: Basic construction
    {
        Quantizer quantizer(44100, 120.0f);
        TestRunner::run_test("Quantizer construction", quantizer.get_tempo() == 120.0f);
    }
    
    // Test 2: MIDI conversion functions
    {
        // Test A4 = 440 Hz = MIDI 69
        float midi_note = Quantizer::frequency_to_midi(440.0f);
        TestRunner::run_test("Frequency to MIDI conversion (A4)", std::abs(midi_note - 69.0f) < 0.01f);
        
        float frequency = Quantizer::midi_to_frequency(69.0f);
        TestRunner::run_test("MIDI to frequency conversion (A4)", std::abs(frequency - 440.0f) < 0.1f);
        
        // Test C4 = ~261.63 Hz = MIDI 60
        midi_note = Quantizer::frequency_to_midi(261.63f);
        TestRunner::run_test("Frequency to MIDI conversion (C4)", std::abs(midi_note - 60.0f) < 0.1f);
        
        frequency = Quantizer::midi_to_frequency(60.0f);
        TestRunner::run_test("MIDI to frequency conversion (C4)", std::abs(frequency - 261.63f) < 1.0f);
    }
    
    // Test 3: Scale quantization
    {
        Quantizer quantizer(44100, 120.0f);
        
        // Test C Major scale quantization
        // Input slightly flat C4 (260 Hz instead of 261.63 Hz)
        float input_freq = 260.0f;
        float quantized = quantizer.quantize_pitch(input_freq, Quantizer::Scale::MAJOR, 60, 1.0f);
        
        // Should quantize to nearest note in C Major scale
        TestRunner::run_test("Scale quantization (C Major)", quantized != input_freq);
        TestRunner::run_test("Scale quantization result valid", quantized > 0.0f);
        
        // Test with zero strength (no quantization)
        float no_quantization = quantizer.quantize_pitch(input_freq, Quantizer::Scale::MAJOR, 60, 0.0f);
        TestRunner::run_test("Scale quantization with zero strength", no_quantization == input_freq);
        
        // Test chromatic scale (should allow all notes)
        float chromatic = quantizer.quantize_pitch(input_freq, Quantizer::Scale::CHROMATIC, 60, 1.0f);
        TestRunner::run_test("Chromatic scale quantization", chromatic > 0.0f);
    }
    
    // Test 4: Custom scale
    {
        Quantizer quantizer(44100, 120.0f);
        
        // Set pentatonic scale intervals
        std::vector<int> pentatonic = {0, 2, 4, 7, 9}; // C, D, E, G, A
        quantizer.set_custom_scale(pentatonic);
        
        float input_freq = 293.66f; // D4
        float quantized = quantizer.quantize_pitch(input_freq, Quantizer::Scale::CUSTOM, 60, 1.0f);
        
        TestRunner::run_test("Custom scale quantization", quantized > 0.0f);
    }
    
    // Test 5: Get nearest note
    {
        Quantizer quantizer(44100, 120.0f);
        
        Note note = quantizer.get_nearest_note(440.0f, Quantizer::Scale::MAJOR, 60);
        
        TestRunner::run_test("Get nearest note - frequency", note.frequency > 0.0f);
        TestRunner::run_test("Get nearest note - MIDI", note.midi_note >= 0 && note.midi_note <= 127);
        TestRunner::run_test("Get nearest note - cents", std::abs(note.cents) <= 1200.0f);
    }
    
    // Test 6: Tempo and timing
    {
        Quantizer quantizer(44100, 120.0f);
        
        // Test tempo setting
        quantizer.set_tempo(140.0f);
        TestRunner::run_test("Tempo setting", quantizer.get_tempo() == 140.0f);
        
        // Test timing quantization (basic functionality)
        uint32_t input_time = 1000;
        uint32_t quantized_time = quantizer.quantize_timing(input_time, 
                                                           Quantizer::GridResolution::QUARTER_NOTE, 
                                                           1.0f);
        TestRunner::run_test("Timing quantization", true); // Just check it doesn't crash
        
        // Test with zero strength
        uint32_t no_timing_quant = quantizer.quantize_timing(input_time, 
                                                            Quantizer::GridResolution::QUARTER_NOTE, 
                                                            0.0f);
        TestRunner::run_test("Timing quantization zero strength", no_timing_quant == input_time);
    }
    
    // Test 7: Edge cases
    {
        Quantizer quantizer(44100, 120.0f);
        
        // Test with zero frequency
        float zero_freq = quantizer.quantize_pitch(0.0f, Quantizer::Scale::MAJOR, 60, 1.0f);
        TestRunner::run_test("Zero frequency quantization", zero_freq == 0.0f);
        
        // Test with negative frequency
        float neg_freq = quantizer.quantize_pitch(-100.0f, Quantizer::Scale::MAJOR, 60, 1.0f);
        TestRunner::run_test("Negative frequency quantization", neg_freq <= 0.0f);
        
        // Test reset
        quantizer.reset();
        TestRunner::run_test("Quantizer reset", true); // Check it doesn't crash
    }
}

void test_autotune_engine() {
    using namespace autotune;
    
    // Test 1: Basic construction
    {
        AutotuneEngine engine(44100, 512, 2);
        TestRunner::run_test("AutotuneEngine construction", engine.is_initialized());
    }
    
    // Test 2: Mode setting
    {
        AutotuneEngine engine(44100, 512, 2);
        
        engine.set_mode(AutotuneEngine::Mode::PITCH_CORRECTION);
        TestRunner::run_test("Mode setting - Pitch Correction", 
                           engine.get_mode() == AutotuneEngine::Mode::PITCH_CORRECTION);
        
        engine.set_mode(AutotuneEngine::Mode::BYPASS);
        TestRunner::run_test("Mode setting - Bypass", 
                           engine.get_mode() == AutotuneEngine::Mode::BYPASS);
    }
    
    // Test 3: Parameter configuration
    {
        AutotuneEngine engine(44100, 512, 2);
        
        ProcessingParams params;
        params.sample_rate = 44100;
        params.correction_strength = 0.8f;
        params.quantize_strength = 0.9f;
        
        engine.set_parameters(params);
        
        const auto& current_params = engine.get_parameters();
        TestRunner::run_test("Parameter setting - sample rate", 
                           current_params.sample_rate == 44100);
        TestRunner::run_test("Parameter setting - correction strength", 
                           current_params.correction_strength == 0.8f);
    }
    
    // Test 4: Scale and tempo configuration
    {
        AutotuneEngine engine(44100, 512, 2);
        
        engine.set_scale(Quantizer::Scale::MAJOR, 60);
        engine.set_tempo(140.0f);
        
        TestRunner::run_test("Scale and tempo configuration", true); // Check no crash
    }
    
    // Test 5: Audio processing
    {
        AutotuneEngine engine(44100, 512, 2);
        
        // Create test audio frame
        AudioFrame input_frame(2);
        input_frame[0] = 0.5f;
        input_frame[1] = 0.3f;
        
        AudioFrame output_frame(2);
        
        ProcessingResult result = engine.process_frame(input_frame, output_frame);
        
        TestRunner::run_test("Single frame processing", result.success);
        TestRunner::run_test("Processing result validity", 
                           result.detected_pitch >= 0.0f && 
                           result.corrected_pitch >= 0.0f && 
                           result.confidence >= 0.0f && result.confidence <= 1.0f);
    }
    
    // Test 6: Performance metrics
    {
        AutotuneEngine engine(44100, 512, 2);
        
        // Process some frames to generate metrics
        AudioFrame input_frame(2);
        AudioFrame output_frame(2);
        
        for (int i = 0; i < 10; ++i) {
            engine.process_frame(input_frame, output_frame);
        }
        
        auto metrics = engine.get_performance_metrics();
        TestRunner::run_test("Performance metrics - latency", metrics.average_latency_ms >= 0.0f);
        TestRunner::run_test("Performance metrics - CPU usage", 
                           metrics.cpu_usage_percent >= 0.0f && metrics.cpu_usage_percent <= 1000.0f);
        TestRunner::run_test("Performance metrics - frames processed", metrics.frames_processed > 0);
    }
    
    // Test 7: Reset functionality
    {
        AutotuneEngine engine(44100, 512, 2);
        
        // Process some audio first
        AudioFrame frame(2);
        engine.process_frame(frame, frame);
        
        // Reset
        engine.reset();
        
        auto metrics = engine.get_performance_metrics();
        TestRunner::run_test("Reset functionality", metrics.frames_processed == 0);
    }
    
    // Test 8: Recommended buffer size
    {
        uint32_t buffer_44k = AutotuneEngine::get_recommended_buffer_size(44100);
        uint32_t buffer_48k = AutotuneEngine::get_recommended_buffer_size(48000);
        
        TestRunner::run_test("Recommended buffer size 44.1kHz", buffer_44k > 0 && buffer_44k <= 2048);
        TestRunner::run_test("Recommended buffer size 48kHz", buffer_48k > 0 && buffer_48k <= 2048);
        TestRunner::run_test("Buffer size scaling", buffer_48k >= buffer_44k);
    }
}
