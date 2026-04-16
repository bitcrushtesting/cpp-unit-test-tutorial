// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

/// @file pressure_regulator_tests.cpp
/// @brief Black-box unit tests for PressureRegulator.
///
/// These tests are written against the PUBLIC API and Doxygen documentation
/// in pressure_regulator.h ONLY. The implementation file
/// (pressure_regulator.cpp) was not read when authoring these tests.
///
/// Each test references the specific documentation clause it is derived from.
/// This makes the relationship between specification and test explicit and
/// reviewable without access to the implementation.

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "pressure_regulator.h"

// ── Test doubles ──────────────────────────────────────────────────────────────

class StubPressureSensor : public IPressureSensor {
public:
    void  setReading(float mbar) { reading_ = mbar; }
    float readMbar() override    { return reading_; }
private:
    float reading_ = 0.0f;
};

class SpyValve : public IValve {
public:
    void open()           override { isOpen_ = true;  ++openCount_;  }
    void close()          override { isOpen_ = false; ++closeCount_; }
    bool isOpen()   const override { return isOpen_; }
    int  openCount()      const    { return openCount_;  }
    int  closeCount()     const    { return closeCount_; }
private:
    bool isOpen_     = false;
    int  openCount_  = 0;
    int  closeCount_ = 0;
};

// ── Fixture ───────────────────────────────────────────────────────────────────

class PressureRegulatorTest : public ::testing::Test {
protected:
    StubPressureSensor sensor;
    SpyValve           valve;
    // Doc: RegulatorConfig defaults — target 1000, deadband 50, max 1500
    RegulatorConfig    config{
        .targetMbar   = 1000.0f,
        .deadbandMbar =   50.0f,
        .maxSafeMbar  = 1500.0f
    };
    PressureRegulator regulator{sensor, valve, config};
};

// ══════════════════════════════════════════════════════════════════════════════
// Lifecycle
// Doc: "Construct → start() → update() periodically → stop()"
// ══════════════════════════════════════════════════════════════════════════════

/// Doc: state() — initial state is Idle.
TEST_F(PressureRegulatorTest, InitialStateIsIdle)
{
    EXPECT_EQ(regulator.state(), RegulatorState::Idle);
}

/// Doc: start() — "Transitions from Idle to active regulation."
TEST_F(PressureRegulatorTest, StartTransitionsFromIdleToRegulating)
{
    sensor.setReading(1000.0f); // within deadband
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Regulating);
}

/// Doc: start() — "Has no effect if the regulator is already active."
TEST_F(PressureRegulatorTest, StartHasNoEffectWhenAlreadyActive)
{
    regulator.start();
    regulator.start(); // second call must not crash or reset state
    EXPECT_NE(regulator.state(), RegulatorState::Idle);
}

/// Doc: stop() — "Transitions to Idle and closes the valve regardless
///               of current pressure."
TEST_F(PressureRegulatorTest, StopClosesValveAndTransitionsToIdle)
{
    sensor.setReading(800.0f); // below deadband — valve will be opened
    regulator.start();
    regulator.update();
    regulator.stop();
    
    EXPECT_EQ(regulator.state(), RegulatorState::Idle);
    EXPECT_FALSE(valve.isOpen());
}

/// Doc: stop() — "Has no effect if already idle."
TEST_F(PressureRegulatorTest, StopHasNoEffectWhenAlreadyIdle)
{
    regulator.stop(); // called before start — must not crash
    EXPECT_EQ(regulator.state(), RegulatorState::Idle);
}

/// Doc: update() — "Has no effect when the state is Idle."
TEST_F(PressureRegulatorTest, UpdateHasNoEffectWhenIdle)
{
    sensor.setReading(500.0f);
    regulator.update(); // called without start()
    EXPECT_EQ(regulator.state(), RegulatorState::Idle);
    EXPECT_EQ(valve.openCount(), 0);
    EXPECT_EQ(valve.closeCount(), 0);
}

// ══════════════════════════════════════════════════════════════════════════════
// Deadband behaviour
// Doc: "While measured pressure is within [target - deadband, target + deadband]
//       the valve is not changed and the state is Regulating."
// Deadband range with defaults: [950, 1050]
// ══════════════════════════════════════════════════════════════════════════════

/// Doc: pressure exactly at target — within deadband.
TEST_F(PressureRegulatorTest, PressureAtTargetProducesRegulatingState)
{
    sensor.setReading(1000.0f);
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Regulating);
}

/// Doc: pressure at lower deadband boundary — within range, no valve change.
TEST_F(PressureRegulatorTest, PressureAtLowerDeadbandBoundaryIsWithinRange)
{
    sensor.setReading(950.0f); // target - deadband = 1000 - 50 = 950
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Regulating);
}

/// Doc: pressure at upper deadband boundary — within range, no valve change.
TEST_F(PressureRegulatorTest, PressureAtUpperDeadbandBoundaryIsWithinRange)
{
    sensor.setReading(1050.0f); // target + deadband = 1000 + 50 = 1050
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Regulating);
}

// ══════════════════════════════════════════════════════════════════════════════
// Valve control: Increasing
// Doc: pressure below deadband lower bound → valve opens, state Increasing
// ══════════════════════════════════════════════════════════════════════════════

/// Doc: pressure below lower deadband — valve opens, state Increasing.
TEST_F(PressureRegulatorTest, PressureBelowDeadbandOpensValve)
{
    sensor.setReading(949.9f); // just below 950
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Increasing);
    EXPECT_TRUE(valve.isOpen());
}

/// Doc: pressure well below target.
TEST_F(PressureRegulatorTest, PressureWellBelowTargetOpensValve)
{
    sensor.setReading(200.0f);
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Increasing);
    EXPECT_EQ(valve.openCount(), 1);
}

// ══════════════════════════════════════════════════════════════════════════════
// Valve control: Decreasing
// Doc: pressure above deadband upper bound → valve closes, state Decreasing
// ══════════════════════════════════════════════════════════════════════════════

/// Doc: pressure above upper deadband — valve closes, state Decreasing.
TEST_F(PressureRegulatorTest, PressureAboveDeadbandClosesValve)
{
    sensor.setReading(1050.1f); // just above 1050
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Decreasing);
    EXPECT_FALSE(valve.isOpen());
}

// ══════════════════════════════════════════════════════════════════════════════
// Fault condition
// Doc: "If update() reads a pressure above maxSafeMbar, the valve is closed
//       immediately and the state transitions to Fault."
// ══════════════════════════════════════════════════════════════════════════════

/// Doc: pressure exceeds maxSafeMbar → state becomes Fault.
TEST_F(PressureRegulatorTest, PressureAboveMaxSafeTransitionsToFault)
{
    sensor.setReading(1501.0f); // above maxSafeMbar 1500
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Fault);
}

/// Doc: "The valve is closed immediately" on fault.
TEST_F(PressureRegulatorTest, ValveIsClosedImmediatelyOnFault)
{
    sensor.setReading(1501.0f);
    regulator.start();
    regulator.update();
    EXPECT_FALSE(valve.isOpen());
}

/// Doc: "No further valve changes occur until reset() is called."
TEST_F(PressureRegulatorTest, UpdateHasNoEffectInFaultState)
{
    sensor.setReading(1501.0f);
    regulator.start();
    regulator.update(); // triggers fault
    
    int opensBefore  = valve.openCount();
    int closesBefore = valve.closeCount();
    
    sensor.setReading(800.0f); // low pressure — would open valve if active
    regulator.update();        // must have no effect in Fault state
    
    EXPECT_EQ(valve.openCount(),  opensBefore);
    EXPECT_EQ(valve.closeCount(), closesBefore);
}

/// Doc: faultCount() — "Never decremented by reset()."
TEST_F(PressureRegulatorTest, FaultCountIncrementsOnEachFault)
{
    regulator.start();
    
    sensor.setReading(1501.0f);
    regulator.update();
    EXPECT_EQ(regulator.faultCount(), 1);
    
    regulator.reset();
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.faultCount(), 2);
}

/// Doc: faultCount() — initial value is zero.
TEST_F(PressureRegulatorTest, FaultCountIsZeroOnConstruction)
{
    EXPECT_EQ(regulator.faultCount(), 0);
}

// ══════════════════════════════════════════════════════════════════════════════
// reset()
// Doc: "Transitions from Fault to Idle. Has no effect in any other state.
//       The valve remains closed after reset."
// ══════════════════════════════════════════════════════════════════════════════

/// Doc: reset() transitions from Fault to Idle.
TEST_F(PressureRegulatorTest, ResetTransitionsFromFaultToIdle)
{
    sensor.setReading(1501.0f);
    regulator.start();
    regulator.update();
    EXPECT_EQ(regulator.state(), RegulatorState::Fault);
    
    regulator.reset();
    EXPECT_EQ(regulator.state(), RegulatorState::Idle);
}

/// Doc: reset() has no effect in any state other than Fault.
TEST_F(PressureRegulatorTest, ResetHasNoEffectWhenIdle)
{
    regulator.reset(); // called when already idle
    EXPECT_EQ(regulator.state(), RegulatorState::Idle);
}

/// Doc: "The valve remains closed after reset."
TEST_F(PressureRegulatorTest, ValveRemainsClosedAfterReset)
{
    sensor.setReading(1501.0f);
    regulator.start();
    regulator.update();
    regulator.reset();
    EXPECT_FALSE(valve.isOpen());
}

// ══════════════════════════════════════════════════════════════════════════════
// lastPressureMbar()
// Doc: "Returns 0.0f if update() has not been called since construction
//       or since the last reset()."
// Doc: "@post lastPressureMbar() returns the value read during this call."
// ══════════════════════════════════════════════════════════════════════════════

/// Doc: returns 0.0f before first update().
TEST_F(PressureRegulatorTest, LastPressureIsZeroBeforeFirstUpdate)
{
    EXPECT_FLOAT_EQ(regulator.lastPressureMbar(), 0.0f);
}

/// Doc: returns 0.0f after reset().
TEST_F(PressureRegulatorTest, LastPressureIsZeroAfterReset)
{
    sensor.setReading(1501.0f);
    regulator.start();
    regulator.update();
    regulator.reset();
    EXPECT_FLOAT_EQ(regulator.lastPressureMbar(), 0.0f);
}

/// Doc: @post returns the value read during the last update() call.
TEST_F(PressureRegulatorTest, LastPressureReflectsMostRecentUpdateReading)
{
    sensor.setReading(875.0f);
    regulator.start();
    regulator.update();
    EXPECT_FLOAT_EQ(regulator.lastPressureMbar(), 875.0f);
}
