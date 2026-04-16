# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# Requires Qt5::Core on the target.
# Set Qt5_DIR or CMAKE_PREFIX_PATH to the Qt5 installation if not found
# automatically, e.g.:
#   cmake -B build -DCMAKE_PREFIX_PATH=/opt/Qt/5.15.2/gcc_64
#
# bad_qt_tests.cpp is NOT compiled, it is a teaching artefact showing
# incorrect patterns that would crash or produce wrong results at runtime.
#
# good_qt_tests.cpp provides its own main() (required for QCoreApplication),
# so it must NOT be linked against gtest_main.

find_package(Qt5 COMPONENTS Core Test QUIET)

if(NOT Qt5_FOUND)
    message(WARNING
        "Qt5 not found, skipping 04_qt_signals_slots. "
        "Set CMAKE_PREFIX_PATH to your Qt5 installation to enable this module.")
    return()
endif()

# ── moc ───────────────────────────────────────────────────────────────────────
# AUTOMOC processes Q_OBJECT classes automatically. The #include "*.moc" at
# the bottom of good_qt_tests.cpp is needed because Q_OBJECT appears in a
# .cpp file rather than a header.
set(CMAKE_AUTOMOC ON)

# ── test target ───────────────────────────────────────────────────────────────
# Do NOT use add_workshop_test() here — that links gtest_main which provides
# its own main(). This target needs a custom main() for QCoreApplication.
add_executable(test_04_qt_signals_slots
    good_qt_tests.cpp
)

target_compile_features(test_04_qt_signals_slots PRIVATE cxx_std_17)

target_link_libraries(test_04_qt_signals_slots
    PRIVATE
        Qt5::Core
        Qt5::Test
        GTest::gtest   # gtest only — NOT gtest_main
        GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(test_04_qt_signals_slots)
