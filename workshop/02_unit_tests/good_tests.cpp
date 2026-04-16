// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// ✅ GOOD unit tests — isolated, deterministic, and behaviour-focused.
// These tests use the interfaces from good_structure.cpp.
// They run identically on a build server or on the target hardware.

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "good_structure.cpp" // include the production code under test

using ::testing::Return;
using ::testing::StrictMock;

// ─── Test Doubles ─────────────────────────────────────────────────────────────

// ✅ Fake sensor: returns whatever value the test sets — no hardware needed
class FakeSensor : public ITemperatureSensor {
public:
    void setTemperature(float t) { temp_ = t; }
    float readCelsius() override { return temp_; }
private:
    float temp_ = 20.0f;
};

// ✅ Spy alert: records calls so tests can assert on them
class SpyAlert : public IAlertOutput {
public:
    void setAlert(bool active) override { lastState_ = active; ++callCount_; }
    bool lastState() const { return lastState_; }
    int  callCount() const { return callCount_; }
private:
    bool lastState_ = false;
    int  callCount_ = 0;
};

// ─── Test Fixture ─────────────────────────────────────────────────────────────

// ✅ Fixture sets up fresh objects for every test — no shared state
class TemperatureMonitorTest : public ::testing::Test {
protected:
    FakeSensor sensor;
    SpyAlert   alert;
    // Default threshold = 80°C from MonitorConfig
    TemperatureMonitor monitor{sensor, alert};
};

// ─── Tests ────────────────────────────────────────────────────────────────────

// ✅ One concept per test; name reads as a specification
TEST_F(TemperatureMonitorTest, GivenTempBelowThreshold_WhenChecked_AlertIsInactive)
{
    sensor.setTemperature(50.0f);
    monitor.check();
    EXPECT_FALSE(alert.lastState());
}

TEST_F(TemperatureMonitorTest, GivenTempAboveThreshold_WhenChecked_AlertIsActive)
{
    sensor.setTemperature(85.0f);
    monitor.check();
    EXPECT_TRUE(alert.lastState());
}

// ✅ Tests boundary — the exact threshold value
TEST_F(TemperatureMonitorTest, GivenTempExactlyAtThreshold_AlertIsInactive)
{
    sensor.setTemperature(80.0f); // not *above* threshold
    monitor.check();
    EXPECT_FALSE(alert.lastState());
}

// ✅ Tests that alert count accumulates correctly across multiple checks
TEST_F(TemperatureMonitorTest, AlertCountReflectsNumberOfHighTempChecks)
{
    sensor.setTemperature(90.0f);
    monitor.check();
    monitor.check();
    EXPECT_EQ(monitor.alertCount(), 2);
}

// ✅ Tests that normal-temperature check does NOT increment alert count
TEST_F(TemperatureMonitorTest, AlertCountDoesNotIncrementForNormalTemp)
{
    sensor.setTemperature(20.0f);
    monitor.check();
    EXPECT_EQ(monitor.alertCount(), 0);
}

// ✅ Tests that lastTemperature() reflects the last reading
TEST_F(TemperatureMonitorTest, LastTemperatureMatchesMostRecentSensorValue)
{
    sensor.setTemperature(72.5f);
    monitor.check();
    EXPECT_FLOAT_EQ(monitor.lastTemperature(), 72.5f);
}

// ✅ Tests configurable threshold
TEST_F(TemperatureMonitorTest, CustomThresholdIsRespected)
{
    TemperatureMonitor hotMonitor{sensor, alert, {.thresholdCelsius = 100.0f}};
    sensor.setTemperature(90.0f); // below custom threshold
    hotMonitor.check();
    EXPECT_FALSE(alert.lastState());
}

// ✅ Tests that setAlert is called on every check (not just on transitions)
TEST_F(TemperatureMonitorTest, AlertOutputIsCalledOnEveryCheck)
{
    sensor.setTemperature(50.0f);
    monitor.check();
    monitor.check();
    EXPECT_EQ(alert.callCount(), 2);
}
