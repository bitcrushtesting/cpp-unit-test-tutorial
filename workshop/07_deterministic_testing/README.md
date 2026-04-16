<!--
  Bitcrush Testing 2026
-->

# Module 07: Deterministic Testing of Complex Systems

## Files

| File | Description |
|------|-------------|
| `iclock.h` | `IClock` interface and `ManualClock` fake |
| `timing_examples.h` | `Watchdog` and `ConnectionManager` with injected clock |
| `hardware_interaction.h` | `FakeInterruptSource`, `StubGpioPin`, `SequencedReadable` |
| `concurrency_examples.h` | `IExecutor`, `SynchronousExecutor`, `DataProcessor`, `EventProcessor` |
| `deterministic_tests.cpp` | Full test suite covering all four patterns |

## Patterns covered

- Virtual time with `ManualClock`: test timeouts and deadlines in microseconds
- State machine sequencing: drive state transitions without `sleep()`
- Concurrency: replace thread pools with `SynchronousExecutor` in tests
- ISR simulation: fire hardware interrupts from test code with `FakeInterruptSource`
- GPIO abstraction: control pin state with `StubGpioPin`
- Sequenced hardware responses: pre-program a response sequence with `SequencedReadable`
