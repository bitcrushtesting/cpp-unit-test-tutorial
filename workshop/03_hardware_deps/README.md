# Module 03: Hardware & Library Dependencies

## Files

| File | Description |
|------|-------------|
| `bad_hw_dependency.cpp` | Hard dependency on vendor library — no abstraction |
| `good_hw_dependency.cpp` | Adapter pattern wrapping the vendor library |
| `good_hw_tests.cpp` | Tests using the adapter; no real hardware required |

## Key Lessons

### The Problem

Vendor libraries and hardware APIs change, break, or are unavailable outside the target.  
If your code calls them directly, **nothing is testable off-target**.

### The Solution: Adapter Pattern

```
[Your Logic] → [Your Interface] ← [HardwareAdapter] → [Vendor Library / Hardware]
                                ← [FakeAdapter]      → (used in tests)
```

- Your logic only knows about your interface.
- `HardwareAdapter` is the only class that touches the vendor library.
- `FakeAdapter` is used in tests — it never calls real hardware.

### On Running Tests Only on Target

Since your test suite runs on embedded Linux hardware, apply these rules to keep it practical:

| Rule | Why |
|------|-----|
| Each test is independent (no shared state) | A single failure doesn't cascade |
| No `sleep()` in tests | Target hardware is slow — avoid wasted seconds |
| Fake hardware in unit tests | Keeps each test under 1 ms even on target |
| Reserve real-hardware tests for integration tests | Clearly separate unit vs integration |
| Tag integration tests (e.g. `[hw]`) | Allows running unit tests only with `--gtest_filter=-*Hw*` |
