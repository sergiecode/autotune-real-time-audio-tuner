# Simple Code Structure Validation Test
# This Python script validates the C++ project structure and basic syntax

import os
import re
import sys

class ProjectValidator:
    def __init__(self, project_root):
        self.project_root = project_root
        self.errors = []
        self.warnings = []
        self.passed_tests = 0
        self.total_tests = 0
    
    def log_test(self, test_name, passed, message=""):
        self.total_tests += 1
        if passed:
            self.passed_tests += 1
            print(f"✅ {test_name}")
        else:
            print(f"❌ {test_name}: {message}")
            self.errors.append(f"{test_name}: {message}")
    
    def check_file_exists(self, filepath, description):
        full_path = os.path.join(self.project_root, filepath)
        exists = os.path.exists(full_path)
        self.log_test(f"File exists: {description}", exists, f"Missing: {filepath}")
        return exists
    
    def check_directory_exists(self, dirpath, description):
        full_path = os.path.join(self.project_root, dirpath)
        exists = os.path.isdir(full_path)
        self.log_test(f"Directory exists: {description}", exists, f"Missing: {dirpath}")
        return exists
    
    def validate_cpp_syntax(self, filepath):
        """Basic C++ syntax validation"""
        try:
            full_path = os.path.join(self.project_root, filepath)
            if not os.path.exists(full_path):
                return False
                
            with open(full_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Check for basic C++ patterns
            has_includes = '#include' in content
            has_namespace = 'namespace' in content or '::' in content
            has_class_or_function = 'class ' in content or 'void ' in content or 'int ' in content
            
            # Check for proper header guards or pragma once
            if filepath.endswith('.h'):
                has_header_guard = '#pragma once' in content or '#ifndef' in content
                self.log_test(f"Header guard in {filepath}", has_header_guard)
            
            # Check for basic syntax errors
            brace_count = content.count('{') - content.count('}')
            balanced_braces = brace_count == 0
            
            paren_count = content.count('(') - content.count(')')
            balanced_parens = paren_count == 0
            
            self.log_test(f"Basic syntax in {filepath}", 
                         has_includes and has_namespace and has_class_or_function and balanced_braces,
                         "Missing includes, namespace, or unbalanced braces")
            
            return True
            
        except Exception as e:
            self.log_test(f"Read {filepath}", False, str(e))
            return False
    
    def validate_cmake_syntax(self, filepath):
        """Basic CMakeLists.txt validation"""
        try:
            full_path = os.path.join(self.project_root, filepath)
            with open(full_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            has_cmake_minimum = 'cmake_minimum_required' in content
            has_project = 'project(' in content
            has_cpp_standard = 'CMAKE_CXX_STANDARD' in content
            has_targets = 'add_executable' in content or 'add_library' in content
            
            self.log_test(f"CMake syntax in {filepath}", 
                         has_cmake_minimum and has_project and has_cpp_standard and has_targets,
                         "Missing essential CMake directives")
            
            return True
            
        except Exception as e:
            self.log_test(f"Read CMakeLists.txt", False, str(e))
            return False
    
    def run_validation(self):
        print("🔍 AutoTune Engine Project Validation")
        print("=" * 50)
        
        # Check project structure
        print("\n📁 Project Structure:")
        self.check_directory_exists("src", "Source directory")
        self.check_directory_exists("include", "Include directory")
        self.check_directory_exists("tests", "Tests directory")
        self.check_directory_exists("examples", "Examples directory")
        self.check_directory_exists("extern", "External dependencies directory")
        
        # Check essential files
        print("\n📄 Essential Files:")
        self.check_file_exists("CMakeLists.txt", "Main CMake file")
        self.check_file_exists("README.md", "Project documentation")
        self.check_file_exists(".gitignore", "Git ignore file")
        
        # Check header files
        print("\n🔧 Header Files:")
        headers = [
            "include/audio_types.h",
            "include/audio_buffer.h", 
            "include/pitch_detector.h",
            "include/pitch_corrector.h",
            "include/quantizer.h",
            "include/autotune_engine.h"
        ]
        
        for header in headers:
            if self.check_file_exists(header, f"Header: {os.path.basename(header)}"):
                self.validate_cpp_syntax(header)
        
        # Check source files
        print("\n⚙️ Source Files:")
        sources = [
            "src/audio_buffer.cpp",
            "src/pitch_detector.cpp", 
            "src/pitch_corrector.cpp",
            "src/quantizer.cpp",
            "src/autotune_engine.cpp"
        ]
        
        for source in sources:
            if self.check_file_exists(source, f"Source: {os.path.basename(source)}"):
                self.validate_cpp_syntax(source)
        
        # Check example
        print("\n🎯 Examples:")
        if self.check_file_exists("examples/main.cpp", "Main example"):
            self.validate_cpp_syntax("examples/main.cpp")
        
        # Check test files
        print("\n🧪 Test Files:")
        test_files = [
            "tests/test_main.cpp",
            "tests/test_audio_buffer.cpp",
            "tests/test_pitch_detector.cpp", 
            "tests/test_quantizer.cpp"
        ]
        
        for test_file in test_files:
            if self.check_file_exists(test_file, f"Test: {os.path.basename(test_file)}"):
                self.validate_cpp_syntax(test_file)
        
        # Check CMake files
        print("\n🔨 Build System:")
        if self.check_file_exists("CMakeLists.txt", "Main CMake"):
            self.validate_cmake_syntax("CMakeLists.txt")
        
        if self.check_file_exists("tests/CMakeLists.txt", "Tests CMake"):
            self.validate_cmake_syntax("tests/CMakeLists.txt")
        
        # Check Python bindings
        print("\n🐍 Python Integration:")
        self.check_file_exists("src/python_bindings.cpp", "Python bindings source")
        self.check_file_exists("extern/pybind11/README.md", "PyBind11 setup instructions")
        
        # Summary
        print("\n" + "=" * 50)
        print("📊 VALIDATION SUMMARY")
        print("=" * 50)
        print(f"✅ Passed: {self.passed_tests}/{self.total_tests}")
        print(f"❌ Failed: {len(self.errors)}")
        
        if self.errors:
            print("\n🚨 Issues Found:")
            for error in self.errors:
                print(f"  • {error}")
        
        if self.warnings:
            print("\n⚠️ Warnings:")
            for warning in self.warnings:
                print(f"  • {warning}")
        
        success_rate = (self.passed_tests / self.total_tests) * 100 if self.total_tests > 0 else 0
        print(f"\n🎯 Success Rate: {success_rate:.1f}%")
        
        if success_rate >= 90:
            print("🎉 EXCELLENT! Project structure is ready for development.")
        elif success_rate >= 70:
            print("👍 GOOD! Minor issues to address.")
        else:
            print("⚠️ NEEDS WORK! Several issues need attention.")
        
        return len(self.errors) == 0

if __name__ == "__main__":
    project_root = os.path.dirname(os.path.abspath(__file__))
    validator = ProjectValidator(project_root)
    success = validator.run_validation()
    sys.exit(0 if success else 1)
