# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

# bad_structure.cpp is NOT compiled:
#   - it contains a main() that calls vendor hardware APIs
#   - it exists as a teaching artefact, not a runnable target
#
# good_structure.cpp also has a main() (the composition root) and vendor
# headers, so it is likewise not compiled as a standalone target here.
# The interfaces and TemperatureMonitor class it defines are pulled into
# the 02_unit_tests target via #include.
#
# Nothing to build in this module — sources are illustration only.
message(STATUS "01_code_structure: source files are illustration-only (see 02_unit_tests for compiled tests)")
