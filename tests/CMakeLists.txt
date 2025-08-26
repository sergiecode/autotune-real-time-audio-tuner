cmake_minimum_required(VERSION 3.16)

# Find testing framework (we'll use simple assertions for this example)
# In a real project, you'd want to use Google Test, Catch2, or similar

# Test executable
add_executable(autotune_tests
    test_main.cpp
    test_audio_buffer.cpp
    test_pitch_detector.cpp
    test_quantizer.cpp
)

# Link against the autotune engine
target_link_libraries(autotune_tests autotune_engine)

# Add tests to CTest
add_test(NAME AutotuneTests COMMAND autotune_tests)

# Optional: Add individual test cases
add_test(NAME AudioBufferTest COMMAND autotune_tests audio_buffer)
add_test(NAME PitchDetectorTest COMMAND autotune_tests pitch_detector)
add_test(NAME QuantizerTest COMMAND autotune_tests quantizer)
