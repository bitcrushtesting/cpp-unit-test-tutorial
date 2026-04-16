# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# workshop/07_deterministic_testing/CMakeLists.txt
#
# All production classes are header-only in this module so the test
# target pulls everything in directly.
# No vendor headers required. No hardware needed.

add_workshop_test(
    TARGET  test_07_deterministic_testing
    SOURCES deterministic_tests.cpp
)

target_include_directories(test_07_deterministic_testing
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
