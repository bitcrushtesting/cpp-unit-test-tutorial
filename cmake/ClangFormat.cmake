# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

find_program(CLANG_FORMAT_BIN clang-format)

if(NOT CLANG_FORMAT_BIN)
    message(WARNING "clang-format not found — 'format' and 'format-check' targets not available.")
    return()
endif()

file(GLOB_RECURSE ALL_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/workshop/*.cpp"
    "${CMAKE_SOURCE_DIR}/workshop/*.h"
)

# Apply formatting in-place
add_custom_target(format
    COMMAND ${CLANG_FORMAT_BIN} -i ${ALL_SOURCE_FILES}
    COMMENT "Applying clang-format to all workshop source files..."
)

# Check formatting without modifying (useful as a CI gate)
add_custom_target(format-check
    COMMAND ${CLANG_FORMAT_BIN} --dry-run --Werror ${ALL_SOURCE_FILES}
    COMMENT "Checking clang-format on all workshop source files..."
)
