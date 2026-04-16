// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// concurrency_examples.h
// Patterns for making concurrent logic deterministically testable.
// The key principle: separate business logic from threading infrastructure.

#pragma once

#include <functional>
#include <string>

// ── Executor abstraction ──────────────────────────────────────────────────────

class IExecutor {
public:
    virtual ~IExecutor() = default;
    virtual void post(std::function<void()> task) = 0;
};

// Test fake: runs tasks immediately on the calling thread, in order.
// No threads. No scheduling. Fully deterministic.
class SynchronousExecutor : public IExecutor {
public:
    void post(std::function<void()> task) override { task(); }
};

// ── Output abstraction used by DataProcessor ─────────────────────────────────

class IOutput {
public:
    virtual ~IOutput() = default;
    virtual void write(float value) = 0;
};

class SpyOutput : public IOutput {
public:
    void write(float value) override {
        lastValue_ = value;
        ++writeCount_;
    }
    float lastValue()  const { return lastValue_;  }
    int   writeCount() const { return writeCount_; }
private:
    float lastValue_  = 0.0f;
    int   writeCount_ = 0;
};

// ── Data type ─────────────────────────────────────────────────────────────────

struct Data {
    float raw    = 0.0f;
    float offset = 0.0f;
};

// ── DataProcessor: pure business logic, no threading ─────────────────────────
// Testable by calling process() directly from a test.

class DataProcessor {
public:
    explicit DataProcessor(IOutput& output) : output_(output) {}

    void process(const Data& d) {
        float calibrated = d.raw + d.offset;
        output_.write(calibrated);
    }

private:
    IOutput& output_;
};

// ── EventProcessor: logic that uses an executor for deferred work ─────────────
// Inject SynchronousExecutor in tests to make deferred tasks run immediately.

class EventProcessor {
public:
    EventProcessor(IExecutor& executor, IOutput& output)
        : executor_(executor), output_(output) {}

    void onEvent(const Data& d) {
        // In production this defers work to a thread pool.
        // In tests, SynchronousExecutor runs it immediately.
        executor_.post([this, d]() {
            output_.write(d.raw + d.offset);
        });
    }

private:
    IExecutor& executor_;
    IOutput&   output_;
};
