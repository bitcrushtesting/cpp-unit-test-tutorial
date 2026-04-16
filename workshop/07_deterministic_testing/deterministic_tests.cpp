// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// deterministic_tests.cpp
// Unit tests for timing, state machine sequencing, concurrency, and
// hardware interaction patterns.
// All tests are deterministic: no sleep(), no real hardware, no real threads.

#include <gtest/gtest.h>
#include "iclock.h"
#include "timing_examples.h"
#include "hardware_interaction.h"
#include "concurrency_examples.h"

// ══════════════════════════════════════════════════════════════════════════════
// Section 1: Virtual Time — Watchdog
// ══════════════════════════════════════════════════════════════════════════════

class WatchdogTest : public ::testing::Test {
protected:
    ManualClock clock;
    Watchdog    wdog{clock, 500};
};

TEST_F(WatchdogTest, DoesNotExpireBeforeTimeout)
{
    clock.advanceBy(499);
    EXPECT_FALSE(wdog.hasExpired());
}

TEST_F(WatchdogTest, ExpiresExactlyAtTimeout)
{
    clock.advanceBy(500);
    EXPECT_TRUE(wdog.hasExpired());
}

TEST_F(WatchdogTest, DoesNotExpireJustBeforeTimeout)
{
    clock.advanceBy(499);
    EXPECT_FALSE(wdog.hasExpired());
}

TEST_F(WatchdogTest, ResetRestartsCounting)
{
    clock.advanceBy(400);
    wdog.reset();
    clock.advanceBy(400); // 800 ms total elapsed, but reset at 400 ms
    EXPECT_FALSE(wdog.hasExpired());
}

TEST_F(WatchdogTest, ExpiresAfterResetIfTimeoutElapses)
{
    clock.advanceBy(400);
    wdog.reset();
    clock.advanceBy(500); // 500 ms after reset
    EXPECT_TRUE(wdog.hasExpired());
}

// ══════════════════════════════════════════════════════════════════════════════
// Section 2: State Machine Sequencing — ConnectionManager
// ══════════════════════════════════════════════════════════════════════════════

class ConnectionManagerTest : public ::testing::Test {
protected:
    ManualClock      clock;
    ConnectionManager cm{clock, 1000};
};

TEST_F(ConnectionManagerTest, InitialStateIsDisconnected)
{
    EXPECT_EQ(cm.state(), ConnectionState::Disconnected);
}

TEST_F(ConnectionManagerTest, ConnectTransitionsToConnecting)
{
    cm.connect();
    EXPECT_EQ(cm.state(), ConnectionState::Connecting);
}

TEST_F(ConnectionManagerTest, CallbackTransitionsToConnected)
{
    cm.connect();
    cm.onConnected();
    EXPECT_EQ(cm.state(), ConnectionState::Connected);
}

TEST_F(ConnectionManagerTest, TimeoutTransitionsToError)
{
    cm.connect();
    clock.advanceBy(1001);
    cm.poll();
    EXPECT_EQ(cm.state(), ConnectionState::Error);
}

TEST_F(ConnectionManagerTest, ConnectionBeforeDeadlinePreventsTimeout)
{
    cm.connect();
    clock.advanceBy(500);
    cm.onConnected();
    clock.advanceBy(600); // past original timeout, but already connected
    cm.poll();
    EXPECT_EQ(cm.state(), ConnectionState::Connected);
}

TEST_F(ConnectionManagerTest, PollBeforeTimeoutDoesNotChangeState)
{
    cm.connect();
    clock.advanceBy(500);
    cm.poll();
    EXPECT_EQ(cm.state(), ConnectionState::Connecting);
}

// ══════════════════════════════════════════════════════════════════════════════
// Section 3: Concurrency — SynchronousExecutor
// ══════════════════════════════════════════════════════════════════════════════

TEST(DataProcessorTest, ProcessAppliesOffsetAndWritesResult)
{
    SpyOutput     output;
    DataProcessor processor(output);

    processor.process({.raw = 10.0f, .offset = 2.5f});

    EXPECT_FLOAT_EQ(output.lastValue(), 12.5f);
    EXPECT_EQ(output.writeCount(), 1);
}

TEST(DataProcessorTest, ProcessIsCalledOncePerEvent)
{
    SpyOutput     output;
    DataProcessor processor(output);

    processor.process({.raw = 1.0f});
    processor.process({.raw = 2.0f});

    EXPECT_EQ(output.writeCount(), 2);
}

TEST(EventProcessorTest, DeferredTaskRunsImmediatelyWithSynchronousExecutor)
{
    SynchronousExecutor executor; // replaces real thread pool in tests
    SpyOutput           output;
    EventProcessor      ep(executor, output);

    ep.onEvent({.raw = 5.0f, .offset = 1.0f});

    // With SynchronousExecutor the task runs inline — no waiting
    EXPECT_FLOAT_EQ(output.lastValue(), 6.0f);
    EXPECT_EQ(output.writeCount(), 1);
}

TEST(EventProcessorTest, MultipleEventsEachWriteOnce)
{
    SynchronousExecutor executor;
    SpyOutput           output;
    EventProcessor      ep(executor, output);

    ep.onEvent({.raw = 1.0f});
    ep.onEvent({.raw = 2.0f});
    ep.onEvent({.raw = 3.0f});

    EXPECT_EQ(output.writeCount(), 3);
    EXPECT_FLOAT_EQ(output.lastValue(), 3.0f);
}

// ══════════════════════════════════════════════════════════════════════════════
// Section 4: Hardware Interaction
// ══════════════════════════════════════════════════════════════════════════════

// ISR simulation

TEST(ButtonHandlerTest, NoPressesOnInit)
{
    FakeInterruptSource irq;
    ButtonHandler       handler(irq);
    EXPECT_EQ(handler.pressCount(), 0);
}

TEST(ButtonHandlerTest, SingleInterruptIncrementsPressCount)
{
    FakeInterruptSource irq;
    ButtonHandler       handler(irq);

    irq.fireInterrupt();

    EXPECT_EQ(handler.pressCount(), 1);
}

TEST(ButtonHandlerTest, MultipleInterruptsAccumulateCorrectly)
{
    FakeInterruptSource irq;
    ButtonHandler       handler(irq);

    irq.fireInterrupt();
    irq.fireInterrupt();
    irq.fireInterrupt();

    EXPECT_EQ(handler.pressCount(), 3);
}

// GPIO pin state

TEST(GpioPinTest, IsDeviceReadyReturnsFalseWhenPinLow)
{
    StubGpioPin pin;
    pin.setState(false);
    EXPECT_FALSE(pin.isHigh());
}

TEST(GpioPinTest, IsDeviceReadyReturnsTrueWhenPinHigh)
{
    StubGpioPin pin;
    pin.setState(true);
    EXPECT_TRUE(pin.isHigh());
}

// Sequenced hardware response

TEST(SequencedReadableTest, ReturnsValuesInOrder)
{
    SequencedReadable sensor;
    sensor.addReading(1.0f);
    sensor.addReading(5.0f);
    sensor.addReading(12.0f);

    EXPECT_FLOAT_EQ(sensor.read(), 1.0f);
    EXPECT_FLOAT_EQ(sensor.read(), 5.0f);
    EXPECT_FLOAT_EQ(sensor.read(), 12.0f);
}

TEST(SequencedReadableTest, HoldsLastValueAfterSequenceExhausted)
{
    SequencedReadable sensor;
    sensor.addReading(7.0f);

    sensor.read(); // consumes the one reading
    EXPECT_FLOAT_EQ(sensor.read(), 7.0f); // holds last value
    EXPECT_FLOAT_EQ(sensor.read(), 7.0f);
}
