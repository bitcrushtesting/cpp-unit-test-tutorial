# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# Two test sources are supported:
#
#   example_tests/          -- hand-written reference tests (always compiled)
#   generated_tests/        -- output of generate_blackbox_tests.py (compiled
#                              when the directory exists and contains .cpp files)
#
# The implementation (pressure_regulator.cpp) is compiled as a separate
# object and linked into both targets — test authors never read it.

# ── Helper: collect .cpp files from a directory if it exists ─────────────────
function(collect_test_sources dir out_var)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
        file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp")
        set(${out_var} "${sources}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

# ── Example tests (reference / hand-written) ─────────────────────────────────

collect_test_sources(example_tests EXAMPLE_SOURCES)

if(EXAMPLE_SOURCES)
    add_workshop_test(
        TARGET  test_08_example_tests
        SOURCES
            ${EXAMPLE_SOURCES}
            pressure_regulator.cpp
    )
    target_include_directories(test_08_example_tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    message(STATUS "08_blackbox_testing: no example_tests found — skipping example test target")
endif()

# ── Generated tests (output of generate_blackbox_tests.py) ───────────────────

collect_test_sources(generated_tests GENERATED_SOURCES)

if(GENERATED_SOURCES)
    add_workshop_test(
        TARGET  test_08_generated_tests
        SOURCES
            ${GENERATED_SOURCES}
            pressure_regulator.cpp
    )
    target_include_directories(test_08_generated_tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    message(STATUS "08_blackbox_testing: generated_tests found — building test_08_generated_tests")
else()
    message(STATUS "08_blackbox_testing: no generated_tests found — run generate_blackbox_tests.py to populate")
endif()