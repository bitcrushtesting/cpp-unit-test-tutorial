// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// iclock.h
// IClock interface and ManualClock fake for deterministic time control in tests.

#pragma once
#include <cstdint>

class IClock {
public:
    virtual ~IClock() = default;
    virtual int64_t nowMs() = 0;
};

// ManualClock: test-only fake. Time advances only when the test says so.
class ManualClock : public IClock {
public:
    void    setTime(int64_t ms)   { now_ = ms; }
    void    advanceBy(int64_t ms) { now_ += ms; }
    int64_t nowMs() override      { return now_; }
private:
    int64_t now_ = 0;
};
