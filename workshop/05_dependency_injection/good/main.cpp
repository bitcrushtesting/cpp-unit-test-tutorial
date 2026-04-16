// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// main.cpp
// Composition root for the dependency injection example.
//
// ✅ This is the ONLY file that:
//    - knows about concrete types (FileLogger, SystemClock)
//    - contains main()
//
// All other files depend only on interfaces (ILogger, IClock).

#include "good_di_plain.cpp"

int main()
{
    FileLogger    logger; // concrete — created here, nowhere else
    SystemClock   clock;  // concrete — created here, nowhere else
    DeviceMonitor monitor(logger, clock);
    monitor.reportStatus();
}
