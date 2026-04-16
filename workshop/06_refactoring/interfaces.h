// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// interfaces.h
// ✅ Step 1: pure abstract interfaces describing what FlowController needs.
//    No vendor headers included here.

#pragma once

class IFlowSensor {
public:
    virtual ~IFlowSensor() = default;
    virtual float readLitresPerMinute() = 0;
};

class IValve {
public:
    virtual ~IValve() = default;
    virtual void open()  = 0;
    virtual void close() = 0;
};

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const char* message) = 0;
};
