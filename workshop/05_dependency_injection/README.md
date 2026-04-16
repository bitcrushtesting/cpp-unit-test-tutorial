<!--
  Bitcrush Testing 2026
-->

# Module 05: Dependency Injection

## Files

| File | Description |
|------|-------------|
| `bad_di.cpp` | Hard-coded dependencies — untestable, inflexible |
| `good_di_plain.cpp` | Constructor injection in plain C++ |
| `good_di_qt.cpp` | Constructor injection with QObject-based dependencies |
| `di_tests.cpp` | Tests for both plain and Qt variants using fakes and GMock |

## What Is Dependency Injection?

A class practises dependency injection when it **receives** its collaborators from the outside rather than creating or finding them itself.

```
❌  MyClass creates its own Logger  →  coupled, untestable
✅  MyClass receives a Logger&      →  decoupled, testable
```

## Three Forms (this module covers constructor injection)

| Form | How | When |
|------|-----|------|
| **Constructor injection** | Pass via constructor | Mandatory, lifetime = object lifetime |
| Setter injection | Pass via setter method | Optional or replaceable dependencies |
| Parameter injection | Pass per method call | Dependency only needed for one operation |

## Key Lessons

1. **Inject interfaces, not concrete types** — the caller decides which implementation to provide
2. **Constructor injection makes dependencies explicit** — nothing hidden, no surprises
3. **Fakes and mocks replace real dependencies in tests** — no filesystem, no hardware, no network
4. **Qt classes can be injected too** — pass `QObject`-derived collaborators by pointer through the constructor
5. **GMock lets you verify interactions** — not just state, but whether a method was called with the right arguments
