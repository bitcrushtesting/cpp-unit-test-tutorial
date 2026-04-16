// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// ❌ BAD: Classes create or locate their own dependencies.
//         Nothing can be substituted — impossible to test in isolation.

#include <cstdio>
#include <fstream>
#include <string>
#include <ctime>

// ── Concrete dependencies (no interfaces) ────────────────────────────────────

// ❌ Writes directly to a file — no abstraction
class FileLogger {
public:
    void log(const std::string& message) {
        std::ofstream f("app.log", std::ios::app);
        f << message << "\n";
    }
};

// ❌ Reads directly from /proc/uptime — real hardware/OS dependency
class SystemClock {
public:
    double uptimeSeconds() {
        std::ifstream f("/proc/uptime");
        double up = 0.0;
        f >> up;
        return up;
    }
};

// ── Business logic with hard-coded dependencies ───────────────────────────────

class DeviceMonitor {
public:
    // ❌ DeviceMonitor constructs its own collaborators — callers have no say
    DeviceMonitor()
        : logger_(new FileLogger())
        , clock_(new SystemClock())
    {}

    ~DeviceMonitor() {
        delete logger_;
        delete clock_;
    }

    void reportStatus() {
        // ❌ Cannot test this without a real filesystem and a real clock
        double uptime = clock_->uptimeSeconds();
        logger_->log("Uptime: " + std::to_string(uptime) + "s");
    }

private:
    FileLogger*  logger_;   // ❌ owns concrete type — not replaceable
    SystemClock* clock_;    // ❌ owns concrete type — not replaceable

    // non-copyable but no rule-of-five — another code smell
};

// ── Caller has no control ─────────────────────────────────────────────────────

int main()
{
    DeviceMonitor monitor; // ❌ always uses file + real clock — no test seam
    monitor.reportStatus();
}
