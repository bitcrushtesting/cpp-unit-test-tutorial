// ==========================
//   Bitcrush Testing 2026
// ==========================

// di_tests.cpp
// Tests for both the plain C++ and Qt dependency injection examples.
// Demonstrates fakes (hand-written) and mocks (GMock) for injected deps.

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include "good_di_plain.cpp"
#include "good_di_qt.cpp"

using ::testing::_;
using ::testing::Exactly;
using ::testing::HasSubstr;

// ══════════════════════════════════════════════════════════════════════════════
// Part 1 — Plain C++ Dependency Injection
// ══════════════════════════════════════════════════════════════════════════════

// ── Fakes ─────────────────────────────────────────────────────────────────────

// ✅ Fake logger: captures messages for assertions — no file I/O
class FakeLogger : public ILogger {
public:
    void log(const std::string& message) override {
        messages.push_back(message);
    }
    std::vector<std::string> messages;
};

// ✅ Fake clock: returns a controlled value — no OS dependency
class FakeClock : public IClock {
public:
    void setUptime(double seconds) { uptime_ = seconds; }
    double uptimeSeconds() override { return uptime_; }
private:
    double uptime_ = 0.0;
};

// ── GMock alternative ─────────────────────────────────────────────────────────

// ✅ Mock logger: verifies interactions (was log() called? with what argument?)
class MockLogger : public ILogger {
public:
    MOCK_METHOD(void, log, (const std::string& message), (override));
};

// ── Plain C++ Test Fixture ────────────────────────────────────────────────────

class DeviceMonitorTest : public ::testing::Test {
protected:
    FakeLogger  logger;
    FakeClock   clock;
    DeviceMonitor monitor{logger, clock};
};

// ✅ Verify log output contains the injected clock value
TEST_F(DeviceMonitorTest, ReportStatusLogsUptimeFromInjectedClock)
{
    clock.setUptime(42.0);
    monitor.reportStatus();

    ASSERT_EQ(logger.messages.size(), 1u);
    EXPECT_THAT(logger.messages[0], HasSubstr("42"));
}

// ✅ Verify log is called exactly once per reportStatus()
TEST_F(DeviceMonitorTest, ReportStatusLogsExactlyOnce)
{
    monitor.reportStatus();
    monitor.reportStatus();
    EXPECT_EQ(logger.messages.size(), 2u);
}

// ✅ isOverdue() uses the injected clock — no real time involved
TEST_F(DeviceMonitorTest, IsOverdueReturnsTrueWhenUptimeExceedsThreshold)
{
    clock.setUptime(100.0);
    EXPECT_TRUE(monitor.isOverdue(50.0));
}

TEST_F(DeviceMonitorTest, IsOverdueReturnsFalseWhenUptimeBelowThreshold)
{
    clock.setUptime(10.0);
    EXPECT_FALSE(monitor.isOverdue(50.0));
}

// ✅ GMock: verify log() is called once with a message containing "5"
TEST(DeviceMonitorMockTest, ReportStatusCallsLoggerWithUptimeValue)
{
    MockLogger mock;
    FakeClock  clock;
    clock.setUptime(5.0);
    DeviceMonitor monitor(mock, clock);

    EXPECT_CALL(mock, log(HasSubstr("5"))).Times(Exactly(1));

    monitor.reportStatus();
}

// ✅ GMock: verify no log call happens if reportStatus() is never invoked
TEST(DeviceMonitorMockTest, LoggerNotCalledIfReportStatusNeverCalled)
{
    MockLogger mock;
    FakeClock  clock;
    DeviceMonitor monitor(mock, clock);

    EXPECT_CALL(mock, log(_)).Times(0);
    // reportStatus() deliberately not called
}

// ══════════════════════════════════════════════════════════════════════════════
// Part 2 — Qt Dependency Injection
// ══════════════════════════════════════════════════════════════════════════════

// ── Qt Fakes ──────────────────────────────────────────────────────────────────

class FakeQLogger : public QObject, public IQLogger {
    Q_OBJECT
public:
    void log(const QString& message) override { messages.append(message); }
    QStringList messages;
};

class FakeQClock : public QObject, public IQClock {
    Q_OBJECT
public:
    void setElapsed(qint64 ms) { elapsed_ = ms; }
    qint64 elapsedMs() override { return elapsed_; }
private:
    qint64 elapsed_ = 0;
};

// ── Qt Test Fixture ───────────────────────────────────────────────────────────

class QtDeviceMonitorTest : public ::testing::Test {
protected:
    FakeQLogger  logger;
    FakeQClock   clock;
    QtDeviceMonitor monitor{logger, clock};
};

// ✅ reportStatus() logs the injected clock value
TEST_F(QtDeviceMonitorTest, ReportStatusLogsElapsedFromInjectedClock)
{
    clock.setElapsed(123);
    monitor.reportStatus();

    ASSERT_EQ(logger.messages.size(), 1);
    EXPECT_TRUE(logger.messages[0].contains("123"));
}

// ✅ reportStatus() emits statusReported signal with the clock value
TEST_F(QtDeviceMonitorTest, ReportStatusEmitsSignalWithElapsedMs)
{
    clock.setElapsed(250);
    QSignalSpy spy(&monitor, &QtDeviceMonitor::statusReported);

    monitor.reportStatus();

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toLongLong(), 250LL);
}

// ✅ isOverdue() delegates entirely to the injected clock
TEST_F(QtDeviceMonitorTest, IsOverdueUsesInjectedClock)
{
    clock.setElapsed(1000);
    EXPECT_TRUE(monitor.isOverdue(500));
    EXPECT_FALSE(monitor.isOverdue(2000));
}

// ✅ Multiple reportStatus() calls each emit one signal
TEST_F(QtDeviceMonitorTest, EachReportCallEmitsExactlyOneSignal)
{
    QSignalSpy spy(&monitor, &QtDeviceMonitor::statusReported);
    monitor.reportStatus();
    monitor.reportStatus();
    EXPECT_EQ(spy.count(), 2);
}

// ── Custom main — QCoreApplication required for Qt tests ─────────────────────

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "di_tests.moc"
