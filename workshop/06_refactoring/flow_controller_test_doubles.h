// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// flow_controller_test_doubles.h
// ✅ Step 4: lightweight test doubles for FlowController's dependencies.
//    No hardware. No vendor headers.

#pragma once

#include "interfaces.h"
#include <string>
#include <vector>

// Stub: returns a controlled sensor reading
class StubSensor : public IFlowSensor {
public:
    void setReading(float value) { value_ = value; }
    float readLitresPerMinute() override { return value_; }
private:
    float value_ = 0.0f;
};

// Spy: records open/close calls for assertion
class SpyValve : public IValve {
public:
    void open()  override { ++openCount_;  lastAction_ = "open";  }
    void close() override { ++closeCount_; lastAction_ = "close"; }
    int openCount()  const { return openCount_;  }
    int closeCount() const { return closeCount_; }
    const std::string& lastAction() const { return lastAction_; }
private:
    int openCount_  = 0;
    int closeCount_ = 0;
    std::string lastAction_;
};

// Spy: records log messages for assertion
class SpyLogger : public ILogger {
public:
    void log(const char* message) override { messages_.emplace_back(message); }
    int callCount() const { return static_cast<int>(messages_.size()); }
    const std::string& lastMessage() const { return messages_.back(); }
    const std::vector<std::string>& allMessages() const { return messages_; }
private:
    std::vector<std::string> messages_;
};
