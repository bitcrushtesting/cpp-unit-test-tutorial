// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// good_di_plain.cpp
// ✅ GOOD: Constructor injection with pure C++ interfaces.
//          DeviceMonitor has no knowledge of concrete implementations.

#include <cstdio>
#include <fstream>
#include <string>

// ── Interfaces ────────────────────────────────────────────────────────────────

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
};

class IClock {
public:
    virtual ~IClock() = default;
    virtual double uptimeSeconds() = 0;
};

// ── Business Logic ────────────────────────────────────────────────────────────

// ✅ DeviceMonitor depends only on interfaces — it never creates collaborators
class DeviceMonitor {
public:
    // ✅ Dependencies injected at construction — explicit and mandatory
    DeviceMonitor(ILogger& logger, IClock& clock)
        : logger_(logger), clock_(clock) {}

    void reportStatus() {
        double uptime = clock_.uptimeSeconds();
        logger_.log("Uptime: " + std::to_string(uptime) + "s");
    }

    // ✅ Logic the test can exercise without any I/O
    bool isOverdue(double thresholdSeconds) {
        return clock_.uptimeSeconds() > thresholdSeconds;
    }

private:
    ILogger& logger_;  // ✅ reference to interface — not owned, not concrete
    IClock&  clock_;
};

// ── Concrete Implementations (production only) ────────────────────────────────

class FileLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::ofstream f("app.log", std::ios::app);
        f << message << "\n";
    }
};

class SystemClock : public IClock {
public:
    double uptimeSeconds() override {
        std::ifstream f("/proc/uptime");
        double up = 0.0;
        f >> up;
        return up;
    }
};
