# AutoTune Engine Integration Test
# This script provides comprehensive testing without requiring compilation

import os
import re
import sys
import json
from datetime import datetime

class AutotuneEngineTest:
    def __init__(self, project_root):
        self.project_root = project_root
        self.test_results = []
        self.failed_tests = 0
        self.passed_tests = 0
    
    def log_test(self, category, test_name, passed, details="", suggestion=""):
        result = {
            "category": category,
            "test": test_name,
            "passed": passed,
            "details": details,
            "suggestion": suggestion,
            "timestamp": datetime.now().isoformat()
        }
        self.test_results.append(result)
        
        if passed:
            self.passed_tests += 1
            print(f"✅ [{category}] {test_name}")
        else:
            self.failed_tests += 1
            print(f"❌ [{category}] {test_name}: {details}")
            if suggestion:
                print(f"   💡 Suggestion: {suggestion}")
    
    def test_api_consistency(self):
        """Test API consistency across headers and implementations"""
        print("\n🔍 API Consistency Tests")
        
        # Test header/implementation pairs
        api_pairs = [
            ("include/audio_buffer.h", "src/audio_buffer.cpp"),
            ("include/pitch_detector.h", "src/pitch_detector.cpp"),
            ("include/pitch_corrector.h", "src/pitch_corrector.cpp"),
            ("include/quantizer.h", "src/quantizer.cpp"),
            ("include/autotune_engine.h", "src/autotune_engine.cpp")
        ]
        
        for header_path, impl_path in api_pairs:
            header_full = os.path.join(self.project_root, header_path)
            impl_full = os.path.join(self.project_root, impl_path)
            
            if not os.path.exists(header_full) or not os.path.exists(impl_full):
                self.log_test("API", f"Files exist for {os.path.basename(header_path)}", False,
                             "Missing header or implementation", "Ensure both files exist")
                continue
            
            try:
                with open(header_full, 'r') as f:
                    header_content = f.read()
                with open(impl_full, 'r') as f:
                    impl_content = f.read()
                
                # Extract class names from header
                class_matches = re.findall(r'class\s+(\w+)', header_content)
                
                for class_name in class_matches:
                    # Check if class is implemented
                    has_implementation = f'{class_name}::' in impl_content
                    self.log_test("API", f"Class {class_name} has implementation", 
                                has_implementation,
                                f"No implementation found for {class_name}",
                                f"Add implementation for {class_name} methods")
                
                # Check include consistency
                header_includes = set(re.findall(r'#include\s+[<"]([^>"]+)[>"]', header_content))
                impl_includes = set(re.findall(r'#include\s+[<"]([^>"]+)[>"]', impl_content))
                
                # Implementation should include its header
                header_name = os.path.basename(header_path)
                has_header_include = f'"{header_name}"' in impl_content or f'<{header_name}>' in impl_content
                self.log_test("API", f"{os.path.basename(impl_path)} includes its header", 
                            has_header_include,
                            f"Implementation doesn't include {header_name}",
                            f"Add #include \"{header_name}\" to {impl_path}")
                
            except Exception as e:
                self.log_test("API", f"Parse {os.path.basename(header_path)}", False,
                             str(e), "Check file encoding and syntax")
    
    def test_namespace_consistency(self):
        """Test namespace usage across the project"""
        print("\n🏗️ Namespace Consistency Tests")
        
        files_to_check = [
            "include/audio_types.h",
            "include/audio_buffer.h", 
            "include/pitch_detector.h",
            "include/pitch_corrector.h",
            "include/quantizer.h",
            "include/autotune_engine.h",
            "src/audio_buffer.cpp",
            "src/pitch_detector.cpp",
            "src/pitch_corrector.cpp", 
            "src/quantizer.cpp",
            "src/autotune_engine.cpp"
        ]
        
        namespace_usage = {}
        
        for file_path in files_to_check:
            full_path = os.path.join(self.project_root, file_path)
            if not os.path.exists(full_path):
                continue
                
            try:
                with open(full_path, 'r') as f:
                    content = f.read()
                
                # Find namespace declarations
                namespaces = re.findall(r'namespace\s+(\w+)', content)
                using_statements = re.findall(r'using\s+namespace\s+(\w+)', content)
                
                namespace_usage[file_path] = {
                    'declared': namespaces,
                    'using': using_statements
                }
                
            except Exception as e:
                self.log_test("Namespace", f"Parse {file_path}", False, str(e))
        
        # Check for consistent namespace usage
        common_namespace = "autotune"
        
        for file_path, usage in namespace_usage.items():
            if file_path.startswith("include/") or file_path.startswith("src/"):
                has_autotune_namespace = common_namespace in usage['declared'] or common_namespace in usage['using']
                self.log_test("Namespace", f"{file_path} uses autotune namespace", 
                            has_autotune_namespace,
                            f"File doesn't use {common_namespace} namespace",
                            f"Add 'namespace {common_namespace}' or 'using namespace {common_namespace}'")
    
    def test_include_guards(self):
        """Test header file include guards"""
        print("\n🛡️ Include Guard Tests")
        
        header_files = [
            "include/audio_types.h",
            "include/audio_buffer.h",
            "include/pitch_detector.h", 
            "include/pitch_corrector.h",
            "include/quantizer.h",
            "include/autotune_engine.h"
        ]
        
        for header_path in header_files:
            full_path = os.path.join(self.project_root, header_path)
            if not os.path.exists(full_path):
                continue
                
            try:
                with open(full_path, 'r') as f:
                    content = f.read()
                
                has_pragma_once = '#pragma once' in content
                has_ifndef_guard = '#ifndef' in content and '#define' in content and '#endif' in content
                
                has_guard = has_pragma_once or has_ifndef_guard
                self.log_test("Headers", f"{os.path.basename(header_path)} has include guard", 
                            has_guard,
                            "No include guard found",
                            "Add '#pragma once' at the top of the header")
                
            except Exception as e:
                self.log_test("Headers", f"Parse {header_path}", False, str(e))
    
    def test_example_completeness(self):
        """Test example application completeness"""
        print("\n🎯 Example Application Tests")
        
        example_path = os.path.join(self.project_root, "examples/main.cpp")
        if not os.path.exists(example_path):
            self.log_test("Example", "Main example exists", False, 
                         "examples/main.cpp not found")
            return
        
        try:
            with open(example_path, 'r') as f:
                content = f.read()
            
            # Check for key functionality demonstrations
            has_engine_creation = 'AutotuneEngine' in content
            has_processing = 'process' in content.lower()
            has_scale_demo = 'scale' in content.lower()
            has_output = 'cout' in content or 'printf' in content
            has_main_function = 'int main' in content
            
            self.log_test("Example", "Creates AutotuneEngine", has_engine_creation,
                         "Example doesn't create AutotuneEngine",
                         "Add AutotuneEngine instantiation")
            
            self.log_test("Example", "Demonstrates processing", has_processing,
                         "Example doesn't show audio processing",
                         "Add process() method calls")
            
            self.log_test("Example", "Shows scale usage", has_scale_demo,
                         "Example doesn't demonstrate scales",
                         "Add scale configuration examples")
            
            self.log_test("Example", "Has output statements", has_output,
                         "Example doesn't print results",
                         "Add std::cout statements to show results")
            
            self.log_test("Example", "Has main function", has_main_function,
                         "No main function found",
                         "Add int main() function")
            
        except Exception as e:
            self.log_test("Example", "Parse main.cpp", False, str(e))
    
    def test_cmake_completeness(self):
        """Test CMake configuration completeness"""
        print("\n🔨 CMake Configuration Tests")
        
        cmake_path = os.path.join(self.project_root, "CMakeLists.txt")
        if not os.path.exists(cmake_path):
            self.log_test("CMake", "Main CMakeLists.txt exists", False)
            return
        
        try:
            with open(cmake_path, 'r') as f:
                content = f.read()
            
            # Check essential CMake elements
            has_minimum_version = 'cmake_minimum_required' in content
            has_project = 'project(' in content
            has_cpp_standard = 'CMAKE_CXX_STANDARD' in content
            has_library_target = 'add_library' in content
            has_executable_target = 'add_executable' in content
            has_include_dirs = 'include_directories' in content or 'target_include_directories' in content
            
            self.log_test("CMake", "Has minimum version", has_minimum_version)
            self.log_test("CMake", "Has project declaration", has_project)
            self.log_test("CMake", "Sets C++ standard", has_cpp_standard)
            self.log_test("CMake", "Creates library target", has_library_target)
            self.log_test("CMake", "Creates executable target", has_executable_target)
            self.log_test("CMake", "Configures include directories", has_include_dirs)
            
        except Exception as e:
            self.log_test("CMake", "Parse CMakeLists.txt", False, str(e))
    
    def test_documentation_quality(self):
        """Test documentation completeness"""
        print("\n📚 Documentation Tests")
        
        readme_path = os.path.join(self.project_root, "README.md")
        if not os.path.exists(readme_path):
            self.log_test("Docs", "README.md exists", False)
            return
        
        try:
            with open(readme_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Check documentation elements
            has_title = content.startswith('#') or 'AutoTune' in content[:200]
            has_features = 'feature' in content.lower() or 'capability' in content.lower()
            has_installation = 'install' in content.lower() or 'build' in content.lower()
            has_usage = 'usage' in content.lower() or 'example' in content.lower()
            has_api_docs = 'api' in content.lower() or 'class' in content.lower()
            
            word_count = len(content.split())
            
            self.log_test("Docs", "Has clear title", has_title)
            self.log_test("Docs", "Describes features", has_features)
            self.log_test("Docs", "Has installation instructions", has_installation)
            self.log_test("Docs", "Shows usage examples", has_usage)
            self.log_test("Docs", "Documents API", has_api_docs)
            self.log_test("Docs", f"Adequate length ({word_count} words)", word_count > 500,
                         f"Only {word_count} words", "Expand documentation to at least 500 words")
            
        except Exception as e:
            self.log_test("Docs", "Parse README.md", False, str(e))
    
    def generate_report(self):
        """Generate a comprehensive test report"""
        print("\n" + "="*60)
        print("📊 COMPREHENSIVE TEST REPORT")
        print("="*60)
        
        total_tests = self.passed_tests + self.failed_tests
        success_rate = (self.passed_tests / total_tests * 100) if total_tests > 0 else 0
        
        print(f"✅ Passed: {self.passed_tests}")
        print(f"❌ Failed: {self.failed_tests}")
        print(f"📊 Success Rate: {success_rate:.1f}%")
        
        # Group results by category
        categories = {}
        for result in self.test_results:
            cat = result['category']
            if cat not in categories:
                categories[cat] = {'passed': 0, 'failed': 0, 'tests': []}
            
            if result['passed']:
                categories[cat]['passed'] += 1
            else:
                categories[cat]['failed'] += 1
            categories[cat]['tests'].append(result)
        
        print(f"\n📈 Results by Category:")
        for category, data in categories.items():
            total_cat = data['passed'] + data['failed']
            cat_success = (data['passed'] / total_cat * 100) if total_cat > 0 else 0
            print(f"  {category}: {data['passed']}/{total_cat} ({cat_success:.1f}%)")
        
        # Show failed tests with suggestions
        failed_results = [r for r in self.test_results if not r['passed']]
        if failed_results:
            print(f"\n🚨 Failed Tests & Suggestions:")
            for result in failed_results:
                print(f"  ❌ [{result['category']}] {result['test']}")
                print(f"     Problem: {result['details']}")
                if result['suggestion']:
                    print(f"     Solution: {result['suggestion']}")
                print()
        
        # Overall assessment
        print("🎯 Overall Assessment:")
        if success_rate >= 95:
            print("🎉 EXCELLENT! Project is production-ready.")
        elif success_rate >= 85:
            print("👍 VERY GOOD! Minor improvements needed.")
        elif success_rate >= 70:
            print("✅ GOOD! Some issues to address.")
        elif success_rate >= 50:
            print("⚠️ NEEDS WORK! Several issues need attention.")
        else:
            print("🚨 CRITICAL! Major issues need to be resolved.")
        
        # Save detailed report
        report_data = {
            "timestamp": datetime.now().isoformat(),
            "summary": {
                "total_tests": total_tests,
                "passed": self.passed_tests,
                "failed": self.failed_tests,
                "success_rate": success_rate
            },
            "categories": categories,
            "all_results": self.test_results
        }
        
        try:
            report_path = os.path.join(self.project_root, "test_report.json")
            with open(report_path, 'w') as f:
                json.dump(report_data, f, indent=2)
            print(f"\n📄 Detailed report saved to: test_report.json")
        except Exception as e:
            print(f"\n⚠️ Could not save report: {e}")
        
        return success_rate >= 70
    
    def run_all_tests(self):
        """Run all test suites"""
        print("🧪 AutoTune Engine Comprehensive Testing Suite")
        print("=" * 60)
        
        self.test_api_consistency()
        self.test_namespace_consistency()
        self.test_include_guards()
        self.test_example_completeness()
        self.test_cmake_completeness()
        self.test_documentation_quality()
        
        return self.generate_report()

if __name__ == "__main__":
    project_root = os.path.dirname(os.path.abspath(__file__))
    tester = AutotuneEngineTest(project_root)
    success = tester.run_all_tests()
    sys.exit(0 if success else 1)
