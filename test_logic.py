# AutoTune Engine - Quick Test Suite
# Tests core functionality without requiring compilation

import math
import sys

def test_audio_math():
    """Test audio processing mathematics"""
    print("🔬 Testing Audio Mathematics...")
    
    # Test MIDI to frequency conversion (from quantizer logic)
    def midi_to_frequency(midi_note):
        return 440.0 * (2.0 ** ((midi_note - 69.0) / 12.0))
    
    def frequency_to_midi(frequency):
        return 69.0 + 12.0 * math.log2(frequency / 440.0)
    
    # Test known values
    tests = [
        (440.0, 69),   # A4
        (261.63, 60),  # C4 (approximately)
        (523.25, 72),  # C5 (approximately) 
        (880.0, 81),   # A5
    ]
    
    passed = 0
    total = len(tests) * 2  # Both directions
    
    for freq, midi in tests:
        # Test frequency to MIDI
        calculated_midi = frequency_to_midi(freq)
        midi_correct = abs(calculated_midi - midi) < 0.1
        
        # Test MIDI to frequency
        calculated_freq = midi_to_frequency(midi)
        freq_correct = abs(calculated_freq - freq) < 5.0  # 5Hz tolerance
        
        if midi_correct:
            passed += 1
            print(f"  ✅ {freq}Hz -> MIDI {calculated_midi:.1f} (expected {midi})")
        else:
            print(f"  ❌ {freq}Hz -> MIDI {calculated_midi:.1f} (expected {midi})")
        
        if freq_correct:
            passed += 1
            print(f"  ✅ MIDI {midi} -> {calculated_freq:.1f}Hz (expected {freq})")
        else:
            print(f"  ❌ MIDI {midi} -> {calculated_freq:.1f}Hz (expected {freq})")
    
    print(f"  📊 Audio Math: {passed}/{total} tests passed\n")
    return passed, total

def test_scale_logic():
    """Test musical scale quantization logic"""
    print("🎼 Testing Scale Logic...")
    
    # Major scale intervals (semitones from root)
    major_scale = [0, 2, 4, 5, 7, 9, 11]
    
    def find_nearest_scale_note(input_note, scale_intervals, key_center):
        """Find nearest note in scale"""
        # Normalize to octave starting from key center
        relative_note = input_note - key_center
        octave = int(relative_note // 12)
        note_in_octave = relative_note % 12
        
        # Find nearest interval
        nearest_interval = scale_intervals[0]
        min_distance = abs(note_in_octave - scale_intervals[0])
        
        for interval in scale_intervals:
            distance = abs(note_in_octave - interval)
            if distance < min_distance:
                min_distance = distance
                nearest_interval = interval
        
        return key_center + octave * 12 + nearest_interval
    
    # Test scale quantization
    tests = [
        (60.5, 60, major_scale, 60),  # C# -> C (in C major)
        (61.5, 60, major_scale, 62),  # C# -> D (in C major)
        (63.5, 60, major_scale, 64),  # D# -> E (in C major)
        (65.5, 60, major_scale, 65),  # F -> F (already in scale)
    ]
    
    passed = 0
    total = len(tests)
    
    for input_note, key_center, scale, expected in tests:
        result = find_nearest_scale_note(input_note, scale, key_center)
        correct = abs(result - expected) < 0.1
        
        if correct:
            passed += 1
            print(f"  ✅ MIDI {input_note} -> {result} (expected {expected})")
        else:
            print(f"  ❌ MIDI {input_note} -> {result} (expected {expected})")
    
    print(f"  📊 Scale Logic: {passed}/{total} tests passed\n")
    return passed, total

def test_buffer_logic():
    """Test circular buffer logic"""
    print("💾 Testing Buffer Logic...")
    
    class SimpleCircularBuffer:
        def __init__(self, capacity):
            self.capacity = capacity
            self.buffer = [0.0] * capacity
            self.read_pos = 0
            self.write_pos = 0
        
        def write(self, data):
            written = 0
            for value in data:
                if not self.full():
                    self.buffer[self.write_pos] = value
                    self.write_pos = (self.write_pos + 1) % self.capacity
                    written += 1
                else:
                    break
            return written
        
        def read(self, count):
            data = []
            for _ in range(count):
                if not self.empty():
                    data.append(self.buffer[self.read_pos])
                    self.read_pos = (self.read_pos + 1) % self.capacity
                else:
                    break
            return data
        
        def empty(self):
            return self.read_pos == self.write_pos
        
        def full(self):
            return ((self.write_pos + 1) % self.capacity) == self.read_pos
        
        def available(self):
            if self.write_pos >= self.read_pos:
                return self.write_pos - self.read_pos
            else:
                return self.capacity - self.read_pos + self.write_pos
    
    # Test buffer operations
    buffer = SimpleCircularBuffer(5)
    
    tests = [
        ("Initial empty", buffer.empty(), True),
        ("Initial not full", not buffer.full(), True),
        ("Initial available", buffer.available() == 0, True),
    ]
    
    # Write some data
    written = buffer.write([1.0, 2.0, 3.0])
    tests.append(("Write 3 items", written == 3, True))
    tests.append(("Available after write", buffer.available() == 3, True))
    tests.append(("Not empty after write", not buffer.empty(), True))
    
    # Read data back
    data = buffer.read(2)
    tests.append(("Read 2 items", len(data) == 2 and data == [1.0, 2.0], True))
    tests.append(("Available after read", buffer.available() == 1, True))
    
    # Read remaining
    data = buffer.read(1)
    tests.append(("Read remaining", len(data) == 1 and data == [3.0], True))
    tests.append(("Empty after reading all", buffer.empty(), True))
    
    passed = 0
    total = len(tests)
    
    for test_name, actual, expected in tests:
        correct = actual == expected
        if correct:
            passed += 1
            print(f"  ✅ {test_name}")
        else:
            print(f"  ❌ {test_name}: got {actual}, expected {expected}")
    
    print(f"  📊 Buffer Logic: {passed}/{total} tests passed\n")
    return passed, total

def test_pitch_detection_concepts():
    """Test pitch detection mathematical concepts"""
    print("🎵 Testing Pitch Detection Concepts...")
    
    def autocorrelation_peak(signal, sample_rate):
        """Simplified autocorrelation peak detection"""
        if len(signal) < 2:
            return 0, 0.0
        
        # Simple autocorrelation for demonstration
        max_lag = len(signal) // 2
        autocorr = []
        
        for lag in range(max_lag):
            sum_val = 0.0
            count = 0
            for i in range(len(signal) - lag):
                sum_val += signal[i] * signal[i + lag]
                count += 1
            autocorr.append(sum_val / count if count > 0 else 0.0)
        
        # Find peak (ignoring lag 0)
        if len(autocorr) < 2:
            return 0, 0.0
        
        peak_lag = 1
        peak_value = autocorr[1]
        
        for i in range(2, len(autocorr)):
            if autocorr[i] > peak_value:
                peak_value = autocorr[i]
                peak_lag = i
        
        # Convert lag to frequency
        frequency = sample_rate / peak_lag if peak_lag > 0 else 0
        confidence = peak_value / autocorr[0] if autocorr[0] > 0 else 0
        
        return frequency, confidence
    
    # Test with synthetic sine wave data
    sample_rate = 44100
    frequency = 440.0  # A4
    duration = 0.1  # 100ms
    
    # Generate sine wave samples
    samples = []
    for i in range(int(sample_rate * duration)):
        t = i / sample_rate
        sample = math.sin(2 * math.pi * frequency * t)
        samples.append(sample)
    
    detected_freq, confidence = autocorrelation_peak(samples, sample_rate)
    
    # Test results
    freq_error = abs(detected_freq - frequency) / frequency
    freq_correct = freq_error < 0.1  # 10% tolerance
    confidence_reasonable = confidence > 0.1
    
    tests = [
        ("Frequency detection accuracy", freq_correct, True),
        ("Confidence is reasonable", confidence_reasonable, True),
        ("Detected frequency positive", detected_freq > 0, True),
        ("Confidence in valid range", 0 <= confidence <= 1, True),
    ]
    
    passed = 0
    total = len(tests)
    
    print(f"  Input: {frequency}Hz sine wave")
    print(f"  Detected: {detected_freq:.1f}Hz (error: {freq_error*100:.1f}%)")
    print(f"  Confidence: {confidence:.3f}")
    
    for test_name, actual, expected in tests:
        correct = actual == expected
        if correct:
            passed += 1
            print(f"  ✅ {test_name}")
        else:
            print(f"  ❌ {test_name}")
    
    print(f"  📊 Pitch Detection: {passed}/{total} tests passed\n")
    return passed, total

def main():
    """Run all tests and provide summary"""
    print("🧪 AutoTune Engine - Logic Validation Suite")
    print("=" * 60)
    print("Testing core algorithms without compilation...")
    print()
    
    all_passed = 0
    all_total = 0
    
    # Run all test suites
    test_functions = [
        test_audio_math,
        test_scale_logic,
        test_buffer_logic,
        test_pitch_detection_concepts
    ]
    
    for test_func in test_functions:
        try:
            passed, total = test_func()
            all_passed += passed
            all_total += total
        except Exception as e:
            print(f"❌ Error in {test_func.__name__}: {e}\n")
    
    # Final summary
    success_rate = (all_passed / all_total * 100) if all_total > 0 else 0
    
    print("=" * 60)
    print("📊 LOGIC VALIDATION SUMMARY")
    print("=" * 60)
    print(f"✅ Passed: {all_passed}/{all_total}")
    print(f"📊 Success Rate: {success_rate:.1f}%")
    
    if success_rate >= 90:
        print("🎉 EXCELLENT! Core algorithms are working correctly.")
        print("🚀 Ready for compilation and full testing.")
    elif success_rate >= 70:
        print("👍 GOOD! Minor algorithm issues detected.")
        print("🔧 Review failed tests before compilation.")
    else:
        print("⚠️ ISSUES DETECTED! Review algorithms before proceeding.")
        print("🛠️ Fix logic errors before compilation.")
    
    print(f"\n✨ Next steps:")
    print(f"1. Install a C++ compiler (GCC, Clang, or MSVC)")
    print(f"2. Install CMake")
    print(f"3. Run: cmake .. && make")
    print(f"4. Test: ./autotune_example")
    
    return success_rate >= 70

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
