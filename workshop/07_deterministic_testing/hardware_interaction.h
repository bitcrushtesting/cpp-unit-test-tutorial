// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// hardware_interaction.h
// Abstractions and fakes for ISR callbacks, GPIO pin state,
// and sequenced hardware responses.

#pragma once

#include <functional>
#include <vector>
#include <cstddef>

// ── ISR / Interrupt source ────────────────────────────────────────────────────

class IInterruptSource {
public:
    virtual ~IInterruptSource() = default;
    using Callback = std::function<void()>;
    virtual void registerCallback(Callback cb) = 0;
};

// Fake: stores the registered callback so tests can fire it on demand
class FakeInterruptSource : public IInterruptSource {
public:
    void registerCallback(Callback cb) override { callback_ = cb; }
    void fireInterrupt() { if (callback_) callback_(); }
private:
    Callback callback_;
};

// Example class under test: counts button presses driven by interrupts
class ButtonHandler {
public:
    explicit ButtonHandler(IInterruptSource& irq) {
        irq.registerCallback([this]() { ++pressCount_; });
    }
    int pressCount() const { return pressCount_; }
private:
    int pressCount_ = 0;
};

// ── GPIO pin ──────────────────────────────────────────────────────────────────

class IGpioPin {
public:
    virtual ~IGpioPin() = default;
    virtual bool isHigh() = 0;
};

// Stub: test sets the pin state directly
class StubGpioPin : public IGpioPin {
public:
    void setState(bool high) { high_ = high; }
    bool isHigh() override   { return high_; }
private:
    bool high_ = false;
};

// ── Sequenced sensor ──────────────────────────────────────────────────────────

// Re-use IFlowSensor concept inline for self-containment in this module
class IReadable {
public:
    virtual ~IReadable() = default;
    virtual float read() = 0;
};

// Fake: returns a pre-programmed sequence of values, then holds the last one
class SequencedReadable : public IReadable {
public:
    void addReading(float value) { readings_.push_back(value); }

    float read() override {
        if (index_ < readings_.size()) {
            return readings_[index_++];
        }
        return readings_.empty() ? 0.0f : readings_.back();
    }

private:
    std::vector<float> readings_;
    std::size_t        index_ = 0;
};
