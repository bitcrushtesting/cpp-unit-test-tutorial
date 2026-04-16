# Module 02: Unit Tests

## Files

| File | Description |
|------|-------------|
| `bad_tests.cpp` | Tests that are fragile, non-isolated, and barely verify anything |
| `good_tests.cpp` | Tests that are focused, hermetic, and document behaviour |

## Key Lessons

1. **One assertion concept per test** — a failing test should tell you exactly what broke
2. **Tests must be deterministic** — no real time, no real hardware, no shared state
3. **Test behaviour, not implementation** — don't test private internals
4. **Name tests as sentences** — `GivenTempAboveThreshold_WhenChecked_AlertIsActive`
5. **Use test doubles** — fakes, stubs, mocks replace hardware in unit tests
6. **Since tests run on target hardware** — keep each test fast and independent so the full suite finishes in seconds even on constrained hardware
