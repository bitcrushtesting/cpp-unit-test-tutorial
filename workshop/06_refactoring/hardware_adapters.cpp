// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// hardware_adapters.cpp
// ✅ Step 2: concrete adapters wrapping the vendor SDK.
//    This is the ONLY file that includes vendor hardware headers.

#include "interfaces.h"
#include "vendor/flow_sensor.h"
#include "vendor/valve_driver.h"
#include <cstdio>

class HardwareFlowSensor : public IFlowSensor {
public:
    float readLitresPerMinute() override {
        return flow_sensor_read(SENSOR_PORT_0);
    }
};

class HardwareValve : public IValve {
public:
    void open()  override { valve_driver_open(VALVE_PORT_0);  }
    void close() override { valve_driver_close(VALVE_PORT_0); }
};

class PrintfLogger : public ILogger {
public:
    void log(const char* message) override { printf("%s\n", message); }
};
