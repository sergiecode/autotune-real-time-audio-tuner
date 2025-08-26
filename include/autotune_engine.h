#pragma once

#include "audio_types.h"
#include "audio_buffer.h"
#include "pitch_detector.h"
#include "pitch_corrector.h"
#include "quantizer.h"
#include <memory>
#include <vector>

namespace autotune {

/**
 * @brief Main AutoTune engine for real-time pitch correction and quantization
 * 
 * This is the primary interface for the real-time audio processing engine.
 * It combines pitch detection, correction, and quantization into a single
 * easy-to-use class optimized for low-latency performance.
 */
class AutotuneEngine {
public:
    /**
     * @brief Engine operating modes
     */
    enum class Mode {
        PITCH_CORRECTION,   // Only pitch correction
        QUANTIZATION,       // Only quantization
        FULL_AUTOTUNE,     // Both pitch correction and quantization
        BYPASS             // Pass-through mode
    };
    
    /**
     * @brief Construct AutotuneEngine
     * @param sample_rate Audio sample rate
     * @param buffer_size Processing buffer size (power of 2 recommended)
     * @param channels Number of audio channels
     */
    AutotuneEngine(SampleRate sample_rate, uint32_t buffer_size = 512, 
                   ChannelCount channels = 2);
    
    /**
     * @brief Destructor
     */
    ~AutotuneEngine();
    
    /**
     * @brief Process audio in real-time
     * @param input Input audio buffer
     * @param output Output audio buffer
     * @param frame_count Number of frames to process
     * @return Processing result with performance metrics
     */
    ProcessingResult process(const AudioFrame* input, AudioFrame* output, 
                           uint32_t frame_count);
    
    /**
     * @brief Process single audio frame
     * @param input Input audio frame
     * @param output Output audio frame
     * @return Processing result
     */
    ProcessingResult process_frame(const AudioFrame& input, AudioFrame& output);
    
    /**
     * @brief Set processing parameters
     * @param params Processing parameters
     */
    void set_parameters(const ProcessingParams& params);
    
    /**
     * @brief Get current processing parameters
     * @return Current parameters
     */
    const ProcessingParams& get_parameters() const;
    
    /**
     * @brief Set operating mode
     * @param mode Engine operating mode
     */
    void set_mode(Mode mode);
    
    /**
     * @brief Get current operating mode
     * @return Current mode
     */
    Mode get_mode() const { return mode_; }
    
    /**
     * @brief Set musical scale for quantization
     * @param scale Musical scale
     * @param key_center Root note (MIDI note number)
     */
    void set_scale(Quantizer::Scale scale, int key_center = 60);
    
    /**
     * @brief Set tempo for rhythmic quantization
     * @param tempo Tempo in BPM
     */
    void set_tempo(float tempo);
    
    /**
     * @brief Enable/disable specific processing features
     * @param enable_pitch_correction Enable pitch correction
     * @param enable_quantization Enable quantization
     * @param enable_formant_preservation Enable formant preservation
     */
    void configure_features(bool enable_pitch_correction = true,
                          bool enable_quantization = true,
                          bool enable_formant_preservation = true);
    
    /**
     * @brief Get current performance metrics
     * @return Performance metrics
     */
    struct PerformanceMetrics {
        float average_latency_ms;
        float cpu_usage_percent;
        uint32_t buffer_underruns;
        uint32_t frames_processed;
        
        PerformanceMetrics() : average_latency_ms(0.0f), cpu_usage_percent(0.0f),
                              buffer_underruns(0), frames_processed(0) {}
    };
    
    PerformanceMetrics get_performance_metrics() const;
    
    /**
     * @brief Reset all processing state
     */
    void reset();
    
    /**
     * @brief Check if engine is properly initialized
     * @return True if ready for processing
     */
    bool is_initialized() const { return initialized_; }
    
    /**
     * @brief Get recommended buffer size for given sample rate
     * @param sample_rate Sample rate
     * @return Recommended buffer size
     */
    static uint32_t get_recommended_buffer_size(SampleRate sample_rate);

private:
    // Core components
    std::unique_ptr<AudioBuffer> input_buffer_;
    std::unique_ptr<AudioBuffer> output_buffer_;
    std::unique_ptr<PitchDetector> pitch_detector_;
    std::unique_ptr<PitchCorrector> pitch_corrector_;
    std::unique_ptr<Quantizer> quantizer_;
    
    // Configuration
    SampleRate sample_rate_;
    uint32_t buffer_size_;
    ChannelCount channels_;
    Mode mode_;
    ProcessingParams params_;
    bool initialized_;
    
    // Processing state
    std::vector<AudioFrame> processing_buffer_;
    std::vector<float> mono_buffer_;
    float current_pitch_;
    float target_pitch_;
    float confidence_;
    
    // Performance monitoring
    mutable PerformanceMetrics metrics_;
    std::vector<float> latency_history_;
    uint32_t performance_counter_;
    
    // Scale and quantization settings
    Quantizer::Scale current_scale_;
    int key_center_;
    float tempo_;
    
    /**
     * @brief Initialize all components
     * @return True if successful
     */
    bool initialize_components();
    
    /**
     * @brief Process audio with pitch correction
     * @param input Input frames
     * @param output Output frames
     * @param frame_count Number of frames
     * @return Processing result
     */
    ProcessingResult process_pitch_correction(const AudioFrame* input, 
                                            AudioFrame* output, 
                                            uint32_t frame_count);
    
    /**
     * @brief Process audio with quantization
     * @param input Input frames
     * @param output Output frames
     * @param frame_count Number of frames
     * @return Processing result
     */
    ProcessingResult process_quantization(const AudioFrame* input, 
                                        AudioFrame* output, 
                                        uint32_t frame_count);
    
    /**
     * @brief Convert stereo to mono for pitch detection
     * @param input Input frames
     * @param frame_count Number of frames
     */
    void convert_to_mono(const AudioFrame* input, uint32_t frame_count);
    
    /**
     * @brief Update performance metrics
     * @param processing_time Processing time in seconds
     */
    void update_performance_metrics(float processing_time);
    
    /**
     * @brief Calculate target pitch based on quantization
     * @param detected_pitch Current detected pitch
     * @return Target pitch for correction
     */
    float calculate_target_pitch(float detected_pitch);
    
    // Non-copyable
    AutotuneEngine(const AutotuneEngine&) = delete;
    AutotuneEngine& operator=(const AutotuneEngine&) = delete;
};

} // namespace autotune
