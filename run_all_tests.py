#!/usr/bin/env python3
"""
AutoTune Engine - Master Test Runner
Runs all available tests and provides comprehensive validation
"""

import os
import sys
import subprocess
import time
from datetime import datetime

def run_test_script(script_name, description):
    """Run a test script and capture results"""
    print(f"\n🔄 Running {description}...")
    print("=" * 50)
    
    try:
        start_time = time.time()
        result = subprocess.run([sys.executable, script_name], 
                              capture_output=True, text=True, cwd=os.getcwd())
        end_time = time.time()
        
        duration = end_time - start_time
        
        if result.returncode == 0:
            print(result.stdout)
            print(f"✅ {description} completed successfully in {duration:.2f}s")
            return True, result.stdout, duration
        else:
            print(result.stdout)
            if result.stderr:
                print("STDERR:", result.stderr)
            print(f"❌ {description} failed in {duration:.2f}s")
            return False, result.stdout + result.stderr, duration
            
    except Exception as e:
        print(f"❌ Error running {description}: {e}")
        return False, str(e), 0

def check_build_prerequisites():
    """Check if build tools are available"""
    print("\n🔧 Checking Build Prerequisites...")
    print("=" * 50)
    
    tools = [
        ("cmake", "CMake build system"),
        ("git", "Git version control"),
        ("python", "Python interpreter")
    ]
    
    # Check for C++ compilers
    compilers = [
        ("g++", "GCC C++ compiler"),
        ("clang++", "Clang C++ compiler"), 
        ("cl", "MSVC C++ compiler")
    ]
    
    available_tools = []
    available_compilers = []
    
    for tool, description in tools:
        try:
            result = subprocess.run([tool, "--version"], capture_output=True, text=True)
            if result.returncode == 0:
                print(f"✅ {description} available")
                available_tools.append(tool)
            else:
                print(f"❌ {description} not found")
        except:
            print(f"❌ {description} not found")
    
    for compiler, description in compilers:
        try:
            if compiler == "cl":
                result = subprocess.run([compiler], capture_output=True, text=True)
                if "Microsoft" in result.stderr:
                    print(f"✅ {description} available")
                    available_compilers.append(compiler)
                else:
                    print(f"❌ {description} not found")
            else:
                result = subprocess.run([compiler, "--version"], capture_output=True, text=True)
                if result.returncode == 0:
                    print(f"✅ {description} available")
                    available_compilers.append(compiler)
                else:
                    print(f"❌ {description} not found")
        except:
            print(f"❌ {description} not found")
    
    return available_tools, available_compilers

def attempt_build():
    """Attempt to build the project if tools are available"""
    print("\n🏗️ Attempting Project Build...")
    print("=" * 50)
    
    available_tools, available_compilers = check_build_prerequisites()
    
    if "cmake" not in available_tools:
        print("⚠️ CMake not available - skipping build test")
        return False, "CMake not found"
    
    if not available_compilers:
        print("⚠️ No C++ compiler available - skipping build test")
        return False, "No C++ compiler found"
    
    try:
        # Create build directory
        if not os.path.exists("build"):
            os.makedirs("build")
            print("📁 Created build directory")
        
        # Run CMake configure
        print("🔧 Configuring with CMake...")
        os.chdir("build")
        
        result = subprocess.run(["cmake", ".."], capture_output=True, text=True)
        if result.returncode != 0:
            print(f"❌ CMake configuration failed:")
            print(result.stdout)
            print(result.stderr)
            os.chdir("..")
            return False, "CMake configuration failed"
        
        print("✅ CMake configuration successful")
        
        # Attempt to build
        print("🔨 Building project...")
        build_result = subprocess.run(["cmake", "--build", "."], capture_output=True, text=True)
        
        os.chdir("..")  # Return to project root
        
        if build_result.returncode == 0:
            print("✅ Build successful!")
            print(build_result.stdout)
            return True, "Build completed successfully"
        else:
            print("❌ Build failed:")
            print(build_result.stdout)
            print(build_result.stderr)
            return False, "Build failed"
            
    except Exception as e:
        if os.getcwd().endswith("build"):
            os.chdir("..")
        print(f"❌ Build error: {e}")
        return False, str(e)

def main():
    """Main test runner"""
    print("🧪 AutoTune Engine - Master Test Runner")
    print("=" * 60)
    print(f"Started at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("=" * 60)
    
    # Ensure we're in the right directory
    if not os.path.exists("CMakeLists.txt"):
        print("❌ Error: Not in AutoTune project directory")
        print("Please run this script from the project root directory.")
        sys.exit(1)
    
    # Test results tracking
    test_results = []
    total_duration = 0
    
    # Run all test suites
    test_suites = [
        ("validate_project.py", "Project Structure Validation"),
        ("test_comprehensive.py", "Comprehensive Integration Tests"),
        ("test_logic.py", "Algorithm Logic Validation")
    ]
    
    for script, description in test_suites:
        if os.path.exists(script):
            success, output, duration = run_test_script(script, description)
            test_results.append((description, success, duration))
            total_duration += duration
        else:
            print(f"⚠️ Test script {script} not found - skipping")
            test_results.append((description, False, 0))
    
    # Attempt compilation if tools are available
    build_success, build_message = attempt_build()
    if build_success is not None:
        test_results.append(("Build Test", build_success, 0))
    
    # Final summary
    print("\n" + "=" * 60)
    print("📊 MASTER TEST SUMMARY")
    print("=" * 60)
    
    total_tests = len(test_results)
    passed_tests = sum(1 for _, success, _ in test_results if success)
    
    print(f"📅 Test Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"⏱️ Total Duration: {total_duration:.2f} seconds")
    print(f"🔢 Total Test Suites: {total_tests}")
    print(f"✅ Passed: {passed_tests}")
    print(f"❌ Failed: {total_tests - passed_tests}")
    
    success_rate = (passed_tests / total_tests * 100) if total_tests > 0 else 0
    print(f"📊 Overall Success Rate: {success_rate:.1f}%")
    
    print(f"\n📋 Detailed Results:")
    for test_name, success, duration in test_results:
        status = "✅ PASS" if success else "❌ FAIL"
        print(f"  {status} {test_name} ({duration:.2f}s)")
    
    # Recommendations
    print(f"\n🎯 Assessment:")
    if success_rate >= 90:
        print("🎉 EXCELLENT! Project is ready for production use.")
        print("🚀 You can start building audio applications immediately.")
    elif success_rate >= 70:
        print("👍 GOOD! Project is functional with minor issues.")
        print("🔧 Review failed tests and address any concerns.")
    else:
        print("⚠️ NEEDS ATTENTION! Several issues detected.")
        print("🛠️ Please address failed tests before proceeding.")
    
    print(f"\n📚 Next Steps:")
    if build_success:
        print("1. ✅ Project builds successfully")
        print("2. 🎵 Run: ./build/autotune_example (or .exe on Windows)")
        print("3. 🧪 Run: ./build/autotune_tests")
        print("4. 🐍 Set up Python bindings for ML integration")
    else:
        print("1. 🔧 Install missing build tools (CMake, C++ compiler)")
        print("2. 📖 Review BUILD.md for detailed instructions")
        print("3. 🔨 Attempt manual compilation")
        print("4. 🆘 Check project documentation for troubleshooting")
    
    print(f"\n💡 Resources:")
    print("📄 README.md - Complete project documentation")
    print("🔨 BUILD.md - Detailed build instructions")
    print("📊 TEST_RESULTS.md - Comprehensive test analysis")
    print("🔗 GitHub repository - Latest updates and support")
    
    print(f"\n🌟 AutoTune Engine - Ready for the future of music technology!")
    
    # Exit with appropriate code
    sys.exit(0 if success_rate >= 70 else 1)

if __name__ == "__main__":
    main()
