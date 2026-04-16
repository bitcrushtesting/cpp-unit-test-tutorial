# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# Fetches GoogleTest and provides a helper function to register test targets.
# Usage:
#   include(GTestSetup)
#   add_workshop_test(
#       TARGET  test_02_unit_tests
#       SOURCES workshop/02_unit_tests/good_tests.cpp
#   )

include(FetchContent)
include(GoogleTest)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
# Prevent GTest from overriding our compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ---------------------------------------------------------------------------
# add_workshop_test(TARGET <name> SOURCES <file> [<file>...] [LABELS <label>...])
#
# Creates a test executable, links GTest + GMock, enables CTest discovery,
# and optionally attaches labels (e.g. "HwIntegration").
# ---------------------------------------------------------------------------
function(add_workshop_test)
    cmake_parse_arguments(AWT "" "TARGET" "SOURCES;LABELS" ${ARGN})

    add_executable(${AWT_TARGET} ${AWT_SOURCES})

    target_compile_features(${AWT_TARGET} PRIVATE cxx_std_17)

    target_link_libraries(${AWT_TARGET}
        PRIVATE
            GTest::gtest_main
            GTest::gmock
    )

    # Discover tests at build time so CTest knows each TEST_F individually
    gtest_discover_tests(${AWT_TARGET}
        PROPERTIES LABELS "${AWT_LABELS}"
    )
endfunction()
