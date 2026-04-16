# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# flow_controller_v1.cpp  — teaching artefact only, NOT compiled
# hardware_adapters.cpp   — requires vendor SDK, NOT compiled in test target
# main.cpp                — production binary, built only when VENDOR SDK present
#
# flow_controller_tests.cpp pulls in flow_controller_v2.cpp via #include,
# so flow_controller_v2.cpp is NOT listed as a separate source.

option(WORKSHOP_BUILD_REFACTORING_APP
    "Build the production FlowController binary (requires vendor SDK)" OFF)

# Test target: no vendor headers required
add_workshop_test(
    TARGET  test_06_refactoring
    SOURCES flow_controller_tests.cpp
)

target_include_directories(test_06_refactoring
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# Production binary: only when vendor SDK is available on target
if(WORKSHOP_BUILD_REFACTORING_APP)
    if(NOT DEFINED VENDOR_SDK_DIR)
        message(FATAL_ERROR
            "WORKSHOP_BUILD_REFACTORING_APP=ON requires -DVENDOR_SDK_DIR=<path>")
    endif()

    add_executable(flow_controller_app main.cpp hardware_adapters.cpp)

    target_compile_features(flow_controller_app PRIVATE cxx_std_17)

    target_include_directories(flow_controller_app
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${VENDOR_SDK_DIR}/include
    )
endif()
