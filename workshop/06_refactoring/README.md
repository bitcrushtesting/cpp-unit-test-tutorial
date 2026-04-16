<!--
  Bitcrush Testing 2026
-->

# Module 06: Refactoring Toward Testability

## Files

| File | Description |
|------|-------------|
| `flow_controller_v1.cpp` | Starting point: untestable legacy code |
| `interfaces.h` | Step 1: extracted interfaces for sensor, valve, logger |
| `hardware_adapters.cpp` | Step 2: thin adapters wrapping the vendor SDK |
| `flow_controller_v2.cpp` | Step 3: refactored controller with constructor injection |
| `flow_controller_test_doubles.h` | Step 4: stub, spy implementations for tests |
| `flow_controller_tests.cpp` | Step 5: full unit test suite |
| `main.cpp` | Composition root wiring real hardware |
