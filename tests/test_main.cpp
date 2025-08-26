#include <iostream>
#include <string>
#include <vector>

// Simple test framework
class TestRunner {
public:
    struct TestResult {
        std::string name;
        bool passed;
        std::string message;
    };
    
    static std::vector<TestResult> results;
    
    static void run_test(const std::string& name, bool condition, const std::string& message = "") {
        results.push_back({name, condition, message});
        if (condition) {
            std::cout << "[PASS] " << name << std::endl;
        } else {
            std::cout << "[FAIL] " << name;
            if (!message.empty()) {
                std::cout << " - " << message;
            }
            std::cout << std::endl;
        }
    }
    
    static void print_summary() {
        int passed = 0;
        int total = static_cast<int>(results.size());
        
        for (const auto& result : results) {
            if (result.passed) passed++;
        }
        
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passed << "/" << total << std::endl;
        
        if (passed == total) {
            std::cout << "All tests passed! ✓" << std::endl;
        } else {
            std::cout << "Some tests failed! ✗" << std::endl;
            
            std::cout << "\nFailed tests:" << std::endl;
            for (const auto& result : results) {
                if (!result.passed) {
                    std::cout << "  - " << result.name;
                    if (!result.message.empty()) {
                        std::cout << " (" << result.message << ")";
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    
    static bool all_passed() {
        for (const auto& result : results) {
            if (!result.passed) return false;
        }
        return true;
    }
};

std::vector<TestRunner::TestResult> TestRunner::results;

// Test function declarations
void test_audio_buffer();
void test_pitch_detector();
void test_quantizer();
void test_autotune_engine();

int main(int argc, char* argv[]) {
    std::cout << "AutoTune Engine Test Suite" << std::endl;
    std::cout << "=========================" << std::endl;
    
    // Check if specific test was requested
    std::string test_name = "";
    if (argc > 1) {
        test_name = argv[1];
    }
    
    try {
        if (test_name.empty() || test_name == "audio_buffer") {
            std::cout << "\nRunning AudioBuffer tests..." << std::endl;
            test_audio_buffer();
        }
        
        if (test_name.empty() || test_name == "pitch_detector") {
            std::cout << "\nRunning PitchDetector tests..." << std::endl;
            test_pitch_detector();
        }
        
        if (test_name.empty() || test_name == "quantizer") {
            std::cout << "\nRunning Quantizer tests..." << std::endl;
            test_quantizer();
        }
        
        if (test_name.empty() || test_name == "engine") {
            std::cout << "\nRunning AutotuneEngine tests..." << std::endl;
            test_autotune_engine();
        }
        
        TestRunner::print_summary();
        
        return TestRunner::all_passed() ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Test error: " << e.what() << std::endl;
        return 1;
    }
}
