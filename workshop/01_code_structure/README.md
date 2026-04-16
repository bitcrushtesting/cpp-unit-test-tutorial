# Module 01: Code Structure

## Files

| File | Description |
|------|-------------|
| `bad_structure.cpp` | Monolithic, untestable code — hardware calls mixed with logic |
| `good_structure.cpp` | Layered design with interfaces and dependency injection |

## Key Lessons

1. **Separate I/O from logic** — never call hardware APIs directly inside business logic
2. **Depend on abstractions** — classes should take interfaces, not concrete drivers
3. **Single Responsibility** — one class, one job
4. **Avoid global state** — it makes tests order-dependent and fragile
