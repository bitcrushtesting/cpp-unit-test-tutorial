// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// main.cpp
// Composition root: constructs real hardware adapters and wires them into
// FlowController. This is the only file that knows about concrete types.

#include "interfaces.h"
#include "hardware_adapters.cpp"
#include "flow_controller_v2.cpp"
#include "vendor/flow_sensor.h"
#include "vendor/valve_driver.h"
#include <unistd.h>

int main()
{
    // Hardware initialisation belongs here, not in any adapter or controller
    flow_sensor_init(SENSOR_PORT_0);
    valve_driver_init(VALVE_PORT_0);

    HardwareFlowSensor sensor;
    HardwareValve      valve;
    PrintfLogger       logger;

    FlowController controller(sensor, valve, logger);

    while (true) {
        controller.update();
        usleep(100000); // 100 ms poll interval
    }

    // Cleanup on exit
    valve_driver_close(VALVE_PORT_0);
    flow_sensor_deinit(SENSOR_PORT_0);
}
