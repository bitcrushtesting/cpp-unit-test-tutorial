# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# good_tests.cpp #includes good_structure.cpp directly to pull in the
# interfaces and TemperatureMonitor.  good_structure.cpp is therefore NOT
# listed as a separate source — doing so would cause duplicate symbol errors.
#
# bad_tests.cpp is NOT compiled:
#   - it calls real hardware APIs (sensor_read_temperature, gpio_read)
#   - it accesses private members — it is a teaching artefact only

add_workshop_test(
    TARGET  test_02_unit_tests
    SOURCES good_tests.cpp
    # good_structure.cpp is pulled in via #include inside good_tests.cpp
)

# Make the source directory visible so the #include "good_structure.cpp"
# inside good_tests.cpp resolves correctly.
target_include_directories(test_02_unit_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/workshop/01_code_structure
)
