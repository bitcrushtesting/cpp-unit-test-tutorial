# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# On macOS, Qt5 installed via Homebrew is keg-only and not on the default
# CMake search path. This module detects the Homebrew prefix for qt@5 and
# appends it to CMAKE_PREFIX_PATH so that find_package(Qt5) succeeds without
# requiring the user to pass -DCMAKE_PREFIX_PATH manually.
#
# Usage:
#   include(HomebrewQt5)   # before any find_package(Qt5 ...) call
#
# On non-macOS systems or when Homebrew is not installed this module is a
# no-op — find_package(Qt5) will proceed using the normal search paths.

if(NOT APPLE)
    return()
endif()

find_program(BREW_BIN brew)

if(NOT BREW_BIN)
    return()
endif()

execute_process(
    COMMAND "${BREW_BIN}" --prefix qt@5
    OUTPUT_VARIABLE QT5_HOMEBREW_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE BREW_RESULT
)

if(NOT BREW_RESULT EQUAL 0 OR NOT QT5_HOMEBREW_PREFIX)
    message(STATUS "HomebrewQt5: qt@5 not found via Homebrew — skipping prefix injection.")
    return()
endif()

list(APPEND CMAKE_PREFIX_PATH "${QT5_HOMEBREW_PREFIX}")
# Propagate to parent scope so subdirectories see the updated path
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE STRING
    "CMake prefix path (HomebrewQt5 appended ${QT5_HOMEBREW_PREFIX})" FORCE)

message(STATUS "HomebrewQt5: added ${QT5_HOMEBREW_PREFIX} to CMAKE_PREFIX_PATH")
