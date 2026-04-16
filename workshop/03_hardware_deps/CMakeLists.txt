# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# good_hw_dependency.cpp contains both the production code (DataLogger,
# ISerialPort, FakeSerialPort) and the unit + integration tests in one file.
# The hardware adapter (UartSerialPort) is guard-compiled only when
# WORKSHOP_BUILD_HW_ADAPTER is ON — i.e. when building on the real target
# with the vendor UART SDK available.
#
# bad_hw_dependency.cpp is NOT compiled — vendor headers unavailable off-target.

option(WORKSHOP_BUILD_HW_ADAPTER
    "Include UartSerialPort (requires vendor uart_driver.h — target only)" OFF)

add_workshop_test(
    TARGET  test_03_hardware_deps
    SOURCES good_hw_dependency.cpp
    LABELS  "HwIntegration"   # integration tests inside this file carry this label
)

target_include_directories(test_03_hardware_deps
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

if(WORKSHOP_BUILD_HW_ADAPTER)
    # Vendor SDK include path — set via -DVENDOR_SDK_DIR=<path> on the target
    if(NOT DEFINED VENDOR_SDK_DIR)
        message(FATAL_ERROR
            "WORKSHOP_BUILD_HW_ADAPTER=ON requires -DVENDOR_SDK_DIR=<path>")
    endif()
    target_include_directories(test_03_hardware_deps
        PRIVATE ${VENDOR_SDK_DIR}/include
    )
    target_compile_definitions(test_03_hardware_deps
        PRIVATE WORKSHOP_HW_ADAPTER_ENABLED
    )
endif()
