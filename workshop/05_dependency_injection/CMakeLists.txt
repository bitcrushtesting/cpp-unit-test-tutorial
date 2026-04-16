# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# Directory structure:
#   bad/bad_di.cpp          -- teaching artefact only, NOT compiled
#   good/good_di_plain.cpp  -- interfaces and business logic; #included by di_tests.cpp
#   good/good_di_qt.h       -- Qt interfaces and Q_OBJECT classes; #included by di_tests.cpp
#   good/good_di_qt.cpp     -- compiled separately: provides moc for good_di_qt.h
#   good/main.cpp           -- composition root for the production app, NOT compiled here
#
# Qt5 is optional — if not found only the plain C++ tests are built.
# Provides its own main() (for QCoreApplication) — do NOT link gtest_main.

find_package(Qt5 COMPONENTS Core QUIET)
find_package(Qt5 COMPONENTS Test QUIET)

if(NOT Qt5_FOUND)
    message(STATUS
        "05_dependency_injection: Qt5 not found, building plain C++ tests only.")
endif()

set(CMAKE_AUTOMOC ON)

add_executable(test_05_dependency_injection
    di_tests.cpp
    $<$<BOOL:${Qt5_FOUND}>:good/good_di_qt.cpp>  # compiled separately so AUTOMOC sees Q_OBJECT
)

target_compile_features(test_05_dependency_injection PRIVATE cxx_std_17)

target_include_directories(test_05_dependency_injection
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/good
)

target_link_libraries(test_05_dependency_injection
    PRIVATE
        GTest::gtest    # NOT gtest_main — di_tests.cpp provides main()
        GTest::gmock
        $<$<BOOL:${Qt5_FOUND}>:Qt5::Core>
        $<$<BOOL:${Qt5Test_FOUND}>:Qt5::Test>
)

if(Qt5_FOUND)
    target_compile_definitions(test_05_dependency_injection
        PRIVATE WORKSHOP_QT_ENABLED
    )
endif()

include(GoogleTest)
gtest_discover_tests(test_05_dependency_injection)
