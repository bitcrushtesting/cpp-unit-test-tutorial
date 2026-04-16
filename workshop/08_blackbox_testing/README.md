<!--
  Bitcrush Testing 2026
-->

# Module 08: Black-Box Testing from Doxygen Documentation

## The Approach

Tests in this module are written against the **public header and Doxygen
documentation only**. The implementation file (`pressure_regulator.cpp`)
is treated as a closed binary: test authors do not read it.

This models the workflow where:
- A team does not want to expose production code to external tools or services
- Tests are derived from the specification (the documented contract), not from
  reading the implementation
- Each test references the specific documentation clause it verifies

## Files

| File | Visible to test author? | Description |
|------|------------------------|-------------|
| `pressure_regulator.h` | Yes | Public API with full Doxygen documentation |
| `pressure_regulator.cpp` | No | Implementation — compiled and linked, not read |
| `pressure_regulator_tests.cpp` | Authored | Black-box tests derived from the header docs |

## Key Technique: Pimpl Idiom

`PressureRegulator` uses the pimpl (pointer-to-implementation) idiom.
All private state lives in `PressureRegulator::Impl` inside the `.cpp` file.
The public header declares only the interface. Test authors cannot access,
inspect, or depend on any private member.

## Writing Tests from Documentation

For each documented behaviour in the header, ask:

1. What is the precondition described in the docs?
2. What action does the documentation say triggers the behaviour?
3. What observable outcome does the documentation promise?

Each answer maps directly to the Arrange / Act / Assert structure of a test.
