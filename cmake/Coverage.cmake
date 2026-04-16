# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# cmake/Coverage.cmake
#
# Provides a 'coverage' build target that:
#   1. Runs all unit and component tests via CTest (HwIntegration excluded)
#   2. Collects raw gcov data with lcov
#   3. Strips noise (GoogleTest internals, Qt framework headers, moc autogen)
#   4. Generates an HTML report under <build_dir>/coverage_html
#   5. Prints a line / function / branch summary to the terminal
#
# Usage in CMakeLists.txt:
#   include(Coverage)
#
# Build with coverage instrumentation:
#   cmake -B build \
#         -DCMAKE_BUILD_TYPE=Debug \
#         -DCMAKE_CXX_FLAGS="--coverage" \
#         -DCMAKE_EXE_LINKER_FLAGS="--coverage"
#   cmake --build build
#   cmake --build build --target coverage
#
# Requirements:
#   lcov and genhtml must be on PATH.
#   Install with:  brew install lcov      (macOS)
#                  apt  install lcov      (Linux)
#
# Compatibility note (lcov 2.x + clang/LLVM on macOS):
#   The --ignore-errors flags below suppress known incompatibilities that do
#   not affect report accuracy:
#
#   inconsistent  -- clang cannot derive function end lines
#   unsupported   -- gcov < 9 does not support function exclusions
#   gcov          -- moc-generated files produce empty .gcda files (harmless)
#   format        -- unexpected line number 0 for global constructors in clang
#   unused        -- source files with no coverage data
#   category      -- genhtml: unexpected category UNK in clang-generated data

find_program(LCOV_BIN    lcov)
find_program(GENHTML_BIN genhtml)

if(NOT LCOV_BIN OR NOT GENHTML_BIN)
    message(WARNING
        "Coverage: lcov or genhtml not found — 'coverage' target not available. "
        "Install with: brew install lcov  /  apt install lcov")
    return()
endif()

set(COVERAGE_RAW      "${CMAKE_BINARY_DIR}/coverage_raw.info")
set(COVERAGE_FILTERED "${CMAKE_BINARY_DIR}/coverage_filtered.info")
set(COVERAGE_HTML_DIR "${CMAKE_BINARY_DIR}/coverage_html")

# Suppress all known lcov 2.x / clang noise
set(LCOV_IGNORE_FLAGS
    --ignore-errors inconsistent,inconsistent
    --ignore-errors unsupported,unsupported
    --ignore-errors gcov,gcov
    --ignore-errors format,format
    --ignore-errors unused,unused
)

# genhtml also needs category for UNK lines in clang-generated data
set(GENHTML_IGNORE_FLAGS
    --ignore-errors inconsistent,inconsistent
    --ignore-errors unsupported,unsupported
    --ignore-errors format,format
    --ignore-errors category,category
)

add_custom_target(coverage
    COMMENT "Running tests and generating coverage report..."

    # Step 1: run tests, exclude hardware integration tests
    COMMAND ctest
                --test-dir      "${CMAKE_BINARY_DIR}"
                --label-exclude HwIntegration
                --output-on-failure

    # Step 2: collect raw coverage data (branch_coverage required here too)
    COMMAND "${LCOV_BIN}"
                --capture
                --directory     "${CMAKE_BINARY_DIR}"
                --output-file   "${COVERAGE_RAW}"
                --rc            branch_coverage=1
                ${LCOV_IGNORE_FLAGS}

    # Step 3: strip noise — GoogleTest, Qt framework headers, moc autogen files
    COMMAND "${LCOV_BIN}"
                --remove        "${COVERAGE_RAW}"
                "*/googletest/*"
                "*/googlemock/*"
                "*/_deps/*"
                "*/usr/*"
                "*/CMakeFiles/*"
                "*_autogen/*"
                "*/opt/homebrew/*"
                "*/QtCore.framework/*"
                "*/QtTest.framework/*"
                "*/build/_deps/*"
                --output-file   "${COVERAGE_FILTERED}"
                --rc            branch_coverage=1
                ${LCOV_IGNORE_FLAGS}

    # Step 4: generate HTML report with branch coverage visible
    COMMAND "${GENHTML_BIN}"
                "${COVERAGE_FILTERED}"
                --output-directory "${COVERAGE_HTML_DIR}"
                --rc            branch_coverage=1
                ${GENHTML_IGNORE_FLAGS}

    # Step 5: print line / function / branch summary to terminal
    COMMAND "${LCOV_BIN}"
                --summary       "${COVERAGE_FILTERED}"
                --rc            branch_coverage=1
                ${LCOV_IGNORE_FLAGS}

    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

message(STATUS "Coverage: target 'coverage' available. "
               "Build with --coverage flags to enable instrumentation.")