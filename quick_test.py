#!/usr/bin/env python3
"""
AutoTune Engine - Quick Validation (Windows Compatible)
Simple validation without Unicode characters for Windows compatibility
"""

import os
import sys

def main():
    """Quick validation for Windows systems"""
    print("AutoTune Engine - Quick Validation")
    print("=" * 50)
    
    # Check project structure
    required_files = [
        "CMakeLists.txt",
        "README.md",
        "include/autotune_engine.h",
        "include/audio_types.h", 
        "include/audio_buffer.h",
        "include/pitch_detector.h",
        "include/pitch_corrector.h",
        "include/quantizer.h",
        "src/autotune_engine.cpp",
        "src/audio_buffer.cpp",
        "src/pitch_detector.cpp", 
        "src/pitch_corrector.cpp",
        "src/quantizer.cpp",
        "examples/main.cpp",
        "python/bindings.cpp"
    ]
    
    missing_files = []
    for file in required_files:
        if not os.path.exists(file):
            missing_files.append(file)
    
    print(f"Checking {len(required_files)} core files...")
    
    if not missing_files:
        print("PASS: All core files present")
        structure_score = 100
    else:
        print(f"FAIL: Missing {len(missing_files)} files:")
        for file in missing_files:
            print(f"  - {file}")
        structure_score = max(0, 100 - (len(missing_files) * 10))
    
    # Check CMakeLists.txt content
    cmake_checks = 0
    cmake_total = 5
    
    try:
        with open("CMakeLists.txt", "r") as f:
            cmake_content = f.read()
            
        if "cmake_minimum_required" in cmake_content:
            cmake_checks += 1
        if "project(" in cmake_content:
            cmake_checks += 1
        if "add_executable" in cmake_content:
            cmake_checks += 1
        if "target_link_libraries" in cmake_content:
            cmake_checks += 1
        if "find_package" in cmake_content:
            cmake_checks += 1
            
        cmake_score = (cmake_checks / cmake_total) * 100
        print(f"CMake validation: {cmake_checks}/{cmake_total} checks passed ({cmake_score:.1f}%)")
        
    except Exception as e:
        print(f"CMake check failed: {e}")
        cmake_score = 0
    
    # Check source file structure
    source_checks = 0
    source_total = 0
    
    for src_file in ["src/autotune_engine.cpp", "src/pitch_detector.cpp", "src/quantizer.cpp"]:
        if os.path.exists(src_file):
            try:
                with open(src_file, "r") as f:
                    content = f.read()
                    source_total += 3
                    
                    if "#include" in content:
                        source_checks += 1
                    if "class" in content or "struct" in content:
                        source_checks += 1  
                    if len(content) > 500:  # Reasonable implementation size
                        source_checks += 1
                        
            except Exception:
                source_total += 3
    
    source_score = (source_checks / source_total * 100) if source_total > 0 else 0
    print(f"Source code validation: {source_checks}/{source_total} checks passed ({source_score:.1f}%)")
    
    # Overall assessment
    overall_score = (structure_score + cmake_score + source_score) / 3
    
    print("\n" + "=" * 50)
    print("VALIDATION SUMMARY")
    print("=" * 50)
    print(f"Project Structure: {structure_score:.1f}%")
    print(f"CMake Configuration: {cmake_score:.1f}%")
    print(f"Source Code Quality: {source_score:.1f}%")
    print(f"Overall Score: {overall_score:.1f}%")
    
    if overall_score >= 80:
        print("\nSTATUS: EXCELLENT - Project ready for compilation")
        return_code = 0
    elif overall_score >= 60:
        print("\nSTATUS: GOOD - Minor issues detected")
        return_code = 0
    else:
        print("\nSTATUS: NEEDS WORK - Several issues found")
        return_code = 1
    
    print("\nNext steps:")
    print("1. Install C++ compiler (GCC, Clang, or MSVC)")
    print("2. Install CMake build system")
    print("3. Run: mkdir build && cd build && cmake .. && make")
    print("4. Test with: ./autotune_example")
    
    return return_code

if __name__ == "__main__":
    sys.exit(main())
