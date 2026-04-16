// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// ❌ BAD unit tests — fragile, non-isolated, and low-value.

#include <gtest/gtest.h>
#include "vendor/sensor.h"
#include "vendor/gpio.h"

// ❌ TEST 1: Tests with real hardware
// If the sensor is disconnected, the test fails for the wrong reason.
// The test cannot run on a build server.
TEST(TemperatureTest, ReadsTemperature)
{
    float temp = sensor_read_temperature(SENSOR_CHANNEL_0); // real hardware call
    EXPECT_GT(temp, -40.0f); // passes as long as hardware exists — proves nothing about logic
}

// ❌ TEST 2: Tests implementation details
// This test breaks whenever the private variable is renamed, even if behaviour is correct.
TEST(MonitorTest, InternalCounterIncrementsOnAlert)
{
    TemperatureMonitor m;
    m.alertCount_ = 0;      // ❌ accessing private member via friend hack or public exposure
    processTemperature();
    EXPECT_EQ(m.alertCount_, 1);
}

// ❌ TEST 3: Multiple unrelated assertions — when it fails, you don't know which part broke
TEST(MonitorTest, Everything)
{
    EXPECT_GT(sensor_read_temperature(0), -40.0f);
    EXPECT_EQ(gpio_read(GPIO_PIN_ALERT), 0);
    processTemperature();
    // ... 10 more unrelated assertions
    EXPECT_EQ(gpio_read(GPIO_PIN_ALERT), 1); // only fails if temperature happens to be high
}

// ❌ TEST 4: Non-deterministic — depends on real sensor returning a specific value
TEST(MonitorTest, AlertActivatedAtHighTemp)
{
    processTemperature();   // actual temperature at test time unknown
    EXPECT_EQ(gpio_read(GPIO_PIN_ALERT), 1); // may pass or fail depending on environment
}

// ❌ TEST 5: No test isolation — state leaks between tests via global g_alert_count
TEST(MonitorTest, CountIsOneAfterOneAlert)
{
    processTemperature();
    EXPECT_EQ(g_alert_count, 1); // ❌ depends on test execution order; previous tests may have run processTemperature()
}
