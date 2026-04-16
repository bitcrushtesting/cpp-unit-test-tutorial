# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

cmake_minimum_required(VERSION 3.14)
project(cpp_unit_test_tutorial LANGUAGES CXX)

# ── shared module path ───────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ── options ──────────────────────────────────────────────────────────────────
option(BUILD_TESTS "Build unit tests" ON)

# ── module selection ─────────────────────────────────────────────────────────
# Set WORKSHOP_MODULE to a number (1-8) to build only that module.
# Leave empty (default) to build all modules.
#
# Examples:
#   cmake -B build -DWORKSHOP_MODULE=6
#   cmake -B build                          (builds all modules)
set(WORKSHOP_MODULE "" CACHE STRING
    "Build only this workshop module (1-8). Leave empty to build all.")

# Validate the value when provided
if(WORKSHOP_MODULE)
    if(NOT WORKSHOP_MODULE MATCHES "^[1-8]$")
        message(FATAL_ERROR
            "WORKSHOP_MODULE must be a number between 1 and 8, got '${WORKSHOP_MODULE}'.")
    endif()
    message(STATUS "Workshop: building module ${WORKSHOP_MODULE} only.")
else()
    message(STATUS "Workshop: building all modules.")
endif()

# ── helper macro ─────────────────────────────────────────────────────────────
# add_workshop_module(NUMBER DIRECTORY)
# Adds the subdirectory only when WORKSHOP_MODULE is empty or matches NUMBER.
macro(add_workshop_module number directory)
    if(NOT WORKSHOP_MODULE OR WORKSHOP_MODULE STREQUAL "${number}")
        add_subdirectory(${directory})
    endif()
endmacro()

# ── tests ─────────────────────────────────────────────────────────────────────
if(BUILD_TESTS)
    enable_testing()
    include(HomebrewQt5)
    include(GTestSetup)
    include(Coverage)
    include(ClangFormat)
    add_subdirectory(workshop)
endif()
