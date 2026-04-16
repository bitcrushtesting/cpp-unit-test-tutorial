// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// good_qt_tests.cpp
// ✅ GOOD: Correct patterns for testing Qt signals and slots with GTest.
//
// Compile requirements:
//   - CMAKE_AUTOMOC ON  (or run moc manually on this file)
//   - Link: Qt5::Core, gtest, gmock

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QTimer>

// ── Production class under test ───────────────────────────────────────────────

class Sensor : public QObject {
    Q_OBJECT
public:
    explicit Sensor(QObject* parent = nullptr) : QObject(parent) {}

    void setTemperature(float temp) {
        temperature_ = temp;
        if (temp > 80.0f) {
            emit overheating(temp);
        }
        emit temperatureChanged(temp);
    }

    float temperature() const { return temperature_; }

signals:
    void temperatureChanged(float temp);
    void overheating(float temp);

private:
    float temperature_ = 0.0f;
};

// ── Test Fixture ──────────────────────────────────────────────────────────────

// ✅ Fresh Sensor per test — no shared state between tests
class SensorTest : public ::testing::Test {
protected:
    Sensor sensor;
};

// ── Tests: Direct Connections (no event loop needed) ─────────────────────────

// ✅ QSignalSpy captures emissions — no manual flags or slot boilerplate
TEST_F(SensorTest, TemperatureChangedEmittedOnEverySetCall)
{
    QSignalSpy spy(&sensor, &Sensor::temperatureChanged);

    sensor.setTemperature(50.0f);
    sensor.setTemperature(60.0f);

    EXPECT_EQ(spy.count(), 2);
}

// ✅ Verify no spurious emissions below threshold
TEST_F(SensorTest, OverheatingNotEmittedBelowThreshold)
{
    QSignalSpy spy(&sensor, &Sensor::overheating);

    sensor.setTemperature(79.9f);

    EXPECT_EQ(spy.count(), 0);
}

// ✅ Verify overheating fires exactly once above threshold
TEST_F(SensorTest, OverheatingEmittedOnceAboveThreshold)
{
    QSignalSpy spy(&sensor, &Sensor::overheating);

    sensor.setTemperature(85.0f);

    ASSERT_EQ(spy.count(), 1);
}

// ✅ Inspect the signal argument via QSignalSpy's captured list
TEST_F(SensorTest, OverheatingSignalCarriesCorrectTemperature)
{
    QSignalSpy spy(&sensor, &Sensor::overheating);

    sensor.setTemperature(92.5f);

    ASSERT_EQ(spy.count(), 1);
    // QSignalSpy stores each emission as a QList<QVariant> of arguments
    float emittedTemp = spy.at(0).at(0).toFloat();
    EXPECT_FLOAT_EQ(emittedTemp, 92.5f);
}

// ✅ Multiple overheating calls each produce one emission
TEST_F(SensorTest, OverheatingEmittedForEachHighTempSet)
{
    QSignalSpy spy(&sensor, &Sensor::overheating);

    sensor.setTemperature(81.0f);
    sensor.setTemperature(82.0f);
    sensor.setTemperature(83.0f);

    EXPECT_EQ(spy.count(), 3);
}

// ── Tests: Queued Connections (event loop required) ───────────────────────────

// ✅ Receiver slot connected with Qt::QueuedConnection
class Receiver : public QObject {
    Q_OBJECT
public:
    float lastTemp = 0.0f;
    int   callCount = 0;
public slots:
    void onTemperature(float t) { lastTemp = t; ++callCount; }
};

TEST_F(SensorTest, QueuedSlotReceivesValueAfterEventLoopProcessing)
{
    Receiver receiver;
    QObject::connect(&sensor, &Sensor::temperatureChanged,
                     &receiver, &Receiver::onTemperature,
                     Qt::QueuedConnection);

    sensor.setTemperature(55.0f);

    // ✅ Process pending queued events before asserting
    QCoreApplication::processEvents();

    EXPECT_FLOAT_EQ(receiver.lastTemp, 55.0f);
    EXPECT_EQ(receiver.callCount, 1);
}

// ✅ QSignalSpy also works with queued connections after processEvents()
TEST_F(SensorTest, SpyWorksWithQueuedConnectionAfterProcessEvents)
{
    // Reconnect as queued for this test only
    Sensor queuedSensor;
    QSignalSpy spy(&queuedSensor, &Sensor::temperatureChanged);

    // QTimer::singleShot uses the event loop — good stress test
    QTimer::singleShot(0, [&]() { queuedSensor.setTemperature(70.0f); });

    // ✅ processEvents() drains the timer and the queued signal
    QCoreApplication::processEvents();

    ASSERT_EQ(spy.count(), 1);
    EXPECT_FLOAT_EQ(spy.at(0).at(0).toFloat(), 70.0f);
}

// ── Custom main — required to create QCoreApplication before GTest runs ───────

// ✅ Do NOT link gtest_main — provide main() here so QCoreApplication
//    exists for the entire test session.
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv); // ✅ must exist before any Qt test runs
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "good_qt_tests.moc" // ✅ required when Q_OBJECT is in a .cpp file
