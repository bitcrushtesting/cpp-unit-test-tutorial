<!--
  Bitcrush Testing 2026
-->

# Module 04: Qt Signals & Slots with Google Test

## Files

| File | Description |
|------|-------------|
| `bad_qt_tests.cpp` | Testing Qt signals by coupling tests to internals or using no event loop |
| `good_qt_tests.cpp` | Correct signal/slot testing with `QSignalSpy` and a `QCoreApplication` |

## Key Lessons

1. **Qt requires a `QCoreApplication`** — without one, signals, the event loop, and queued connections do not work
2. **Use `QSignalSpy`** — it captures signal emissions without needing to connect a slot manually
3. **Direct connections fire synchronously** — no `QCoreApplication::processEvents()` needed for direct connections; queued connections require it
4. **Do not test private Qt internals** — test observable signal emissions and slot side-effects only
5. **`Q_OBJECT` macro is required** — any class emitting or receiving signals must have it and be processed by `moc`
6. **CMake must run `moc`** — use `set(CMAKE_AUTOMOC ON)` or `qt5_wrap_cpp()` explicitly

## Qt + GTest Integration Notes

GTest and Qt can coexist cleanly but require a small bootstrap:

```cpp
// In main() or a shared test helper
int argc = 0;
QCoreApplication app(argc, nullptr);  // required before any Qt test runs
::testing::InitGoogleTest(&argc, nullptr);
return RUN_ALL_TESTS();
```

GTest's own `main` (from `gtest_main`) does **not** create a `QCoreApplication`,  
so for Qt tests you must provide your own `main`.
