<!--
  Bitcrush Testing 2026
-->

# Embedded C++ Workshop: Code Structure & Unit Testing

Workshop materials for writing testable, well-structured C++ on embedded Linux targets with hardware and library dependencies.

## Repository Structure

```
cmake/
├── Coverage.cmake              # Coverage report target (lcov/genhtml)
└── GTestSetup.cmake            # GoogleTest fetch and add_workshop_test() helper

workshop/
├── 01_code_structure/          # Layered design vs monolithic code
├── 02_unit_tests/              # Good and bad unit test patterns
├── 03_hardware_deps/           # Adapter pattern for vendor libraries
├── 04_qt_signals_slots/        # Testing Qt signals with QSignalSpy
├── 05_dependency_injection/    # Constructor injection, plain C++ and Qt
├── 06_refactoring/             # Step-by-step refactoring toward testability
├── 07_deterministic_testing/   # Timing, concurrency, hardware interaction
└── 08_blackbox_testing/        # Black-box tests from Doxygen documentation

wiki/
└── Home.md                     # Wiki entry point (mirrored on GitHub Wiki)
```

## Prerequisites

- C++17 compiler
- CMake 3.14+
- [GoogleTest](https://github.com/google/googletest): fetched automatically via CMake FetchContent
- lcov and genhtml for coverage reports (`brew install lcov` / `apt install lcov`)
- Qt5 Core for modules 04 and 05 (optional; modules are skipped if Qt5 is not found)
- Access to the target hardware for running integration tests

## Workshop Guide

Full explanations, module walkthroughs, and exercises are on the [project wiki](../../wiki).

## Building

```bash
# Build all modules
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Build a single module (1 to 8)
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DWORKSHOP_MODULE=6
cmake --build build
```

## Running the Tests

```bash
# All unit and component tests (no hardware required)
ctest --test-dir build --label-exclude HwIntegration --output-on-failure

# Hardware integration tests only (target must be connected)
ctest --test-dir build --label-regex HwIntegration --output-on-failure

# Single module
ctest --test-dir build --tests-regex "test_06" --output-on-failure
```

## Coverage Report

Build with coverage instrumentation, run the tests, and generate the report:

```bash
cmake -B build \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="--coverage" \
      -DCMAKE_EXE_LINKER_FLAGS="--coverage"
cmake --build build
cmake --build build --target coverage
```

The HTML report is written to `build/coverage_html/index.html`.
