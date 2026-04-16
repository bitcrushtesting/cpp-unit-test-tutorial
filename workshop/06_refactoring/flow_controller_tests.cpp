// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// flow_controller_tests.cpp
// ✅ Step 5: full unit test suite for FlowController.
//    Runs without hardware. All dependencies injected as test doubles.

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "flow_controller_v2.cpp"
#include "flow_controller_test_doubles.h"

using ::testing::HasSubstr;

// ── Fixture ───────────────────────────────────────────────────────────────────

class FlowControllerTest : public ::testing::Test {
protected:
    StubSensor sensor;
    SpyValve   valve;
    SpyLogger  logger;
    FlowConfig config{.calibrationOffset = 0.5f, .flowThreshold = 10.0f};
    FlowController controller{sensor, valve, logger, config};
};

// ── Valve behaviour ───────────────────────────────────────────────────────────

TEST_F(FlowControllerTest, ValveOpensWhenCalibratedFlowExceedsThreshold)
{
    sensor.setReading(10.0f); // calibrated = 10.5, above threshold 10.0
    controller.update();
    EXPECT_EQ(valve.lastAction(), "open");
}

TEST_F(FlowControllerTest, ValveClosesWhenCalibratedFlowIsBelowThreshold)
{
    sensor.setReading(9.0f); // calibrated = 9.5, below threshold 10.0
    controller.update();
    EXPECT_EQ(valve.lastAction(), "close");
}

TEST_F(FlowControllerTest, ValveClosesWhenCalibratedFlowIsExactlyAtThreshold)
{
    sensor.setReading(9.5f); // calibrated = 10.0, not *above* threshold
    controller.update();
    EXPECT_EQ(valve.lastAction(), "close");
}

TEST_F(FlowControllerTest, ValveIsOnlyOpenedOnce_PerUpdateAboveThreshold)
{
    sensor.setReading(15.0f);
    controller.update();
    EXPECT_EQ(valve.openCount(), 1);
    EXPECT_EQ(valve.closeCount(), 0);
}

// ── Calibration offset ────────────────────────────────────────────────────────

TEST_F(FlowControllerTest, CalibrationOffsetIsAppliedBeforeThresholdCheck)
{
    // Raw reading is below threshold but calibrated value pushes it above
    FlowConfig cfg{.calibrationOffset = 2.0f, .flowThreshold = 10.0f};
    FlowController c{sensor, valve, logger, cfg};
    sensor.setReading(9.0f); // calibrated = 11.0, above threshold
    c.update();
    EXPECT_EQ(valve.lastAction(), "open");
}

TEST_F(FlowControllerTest, NegativeCalibrationOffsetReducesEffectiveReading)
{
    FlowConfig cfg{.calibrationOffset = -1.0f, .flowThreshold = 10.0f};
    FlowController c{sensor, valve, logger, cfg};
    sensor.setReading(11.0f); // calibrated = 10.0, not above threshold
    c.update();
    EXPECT_EQ(valve.lastAction(), "close");
}

// ── Logging ───────────────────────────────────────────────────────────────────

TEST_F(FlowControllerTest, UpdateLogsExactlyOnce)
{
    sensor.setReading(5.0f);
    controller.update();
    EXPECT_EQ(logger.callCount(), 1);
}

TEST_F(FlowControllerTest, LogMessageContainsCalibratedFlowValue)
{
    sensor.setReading(8.0f); // calibrated = 8.5
    controller.update();
    EXPECT_THAT(logger.lastMessage(), HasSubstr("8.50"));
}

TEST_F(FlowControllerTest, LogMessageIndicatesOpenedWhenValveOpens)
{
    sensor.setReading(10.5f);
    controller.update();
    EXPECT_THAT(logger.lastMessage(), HasSubstr("opened"));
}

TEST_F(FlowControllerTest, LogMessageIndicatesClosedWhenValveCloses)
{
    sensor.setReading(5.0f);
    controller.update();
    EXPECT_THAT(logger.lastMessage(), HasSubstr("closed"));
}

// ── Configurable threshold ────────────────────────────────────────────────────

TEST_F(FlowControllerTest, CustomHighThresholdKeepsValveClosedAtNormalFlow)
{
    FlowConfig cfg{.calibrationOffset = 0.0f, .flowThreshold = 50.0f};
    FlowController c{sensor, valve, logger, cfg};
    sensor.setReading(30.0f); // below custom threshold of 50
    c.update();
    EXPECT_EQ(valve.lastAction(), "close");
}

TEST_F(FlowControllerTest, CustomLowThresholdOpensValveAtLowFlow)
{
    FlowConfig cfg{.calibrationOffset = 0.0f, .flowThreshold = 1.0f};
    FlowController c{sensor, valve, logger, cfg};
    sensor.setReading(2.0f); // above custom threshold of 1
    c.update();
    EXPECT_EQ(valve.lastAction(), "open");
}
