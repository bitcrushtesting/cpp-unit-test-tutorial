// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// flow_controller_v1.cpp
// ❌ Starting point: everything hard-coded, nothing injectable.
//    This file is a teaching artefact and is NOT compiled by CMake.

#include <cstdio>
#include "vendor/flow_sensor.h"
#include "vendor/valve_driver.h"

static const float CALIBRATION_OFFSET = 0.5f;
static const float FLOW_THRESHOLD     = 10.0f;

class FlowController {
public:
    FlowController() {
        flow_sensor_init(SENSOR_PORT_0);  // hardware init in constructor
        valve_driver_init(VALVE_PORT_0);
    }

    void update() {
        float raw        = flow_sensor_read(SENSOR_PORT_0);
        float calibrated = raw + CALIBRATION_OFFSET;

        if (calibrated > FLOW_THRESHOLD) {
            valve_driver_open(VALVE_PORT_0);
            printf("[FlowController] valve opened at %.2f L/min\n", calibrated);
        } else {
            valve_driver_close(VALVE_PORT_0);
            printf("[FlowController] valve closed at %.2f L/min\n", calibrated);
        }
    }

    ~FlowController() {
        valve_driver_close(VALVE_PORT_0);
        flow_sensor_deinit(SENSOR_PORT_0);
    }
};
