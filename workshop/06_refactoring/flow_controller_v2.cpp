// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// flow_controller_v2.cpp
// ✅ Step 3: FlowController refactored with constructor injection.
//    No vendor headers. Compiles and runs without hardware present.

#pragma once

#include "interfaces.h"
#include <cstring>

struct FlowConfig {
    float calibrationOffset = 0.5f;
    float flowThreshold     = 10.0f;
};

class FlowController {
public:
    FlowController(IFlowSensor& sensor, IValve& valve,
                   ILogger& logger, FlowConfig config = {})
        : sensor_(sensor), valve_(valve), logger_(logger), config_(config) {}

    void update() {
        float calibrated = sensor_.readLitresPerMinute() + config_.calibrationOffset;

        char msg[64];
        if (calibrated > config_.flowThreshold) {
            valve_.open();
            snprintf(msg, sizeof(msg),
                     "[FlowController] valve opened at %.2f L/min", calibrated);
        } else {
            valve_.close();
            snprintf(msg, sizeof(msg),
                     "[FlowController] valve closed at %.2f L/min", calibrated);
        }
        logger_.log(msg);
    }

private:
    IFlowSensor& sensor_;
    IValve&      valve_;
    ILogger&     logger_;
    FlowConfig   config_;
};
