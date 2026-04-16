// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// bad_qt_tests.cpp
// ❌ BAD: Common mistakes when testing Qt signals and slots with GTest.

#include <gtest/gtest.h>
#include <QObject>
#include <QString>

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

// ── Bad Tests ─────────────────────────────────────────────────────────────────

// ❌ TEST 1: No QCoreApplication
// Qt signal/slot machinery and QSignalSpy require a QCoreApplication to exist.
// Without it the test may crash or silently produce wrong results.
TEST(SensorBadTest, SignalEmittedOnHighTemp)
{
    Sensor sensor;
    bool signalReceived = false;

    // ❌ Manual flag via lambda — fragile and unnecessary with QSignalSpy
    QObject::connect(&sensor, &Sensor::overheating,
                     [&](float) { signalReceived = true; });

    sensor.setTemperature(90.0f);

    // ❌ This may pass or crash depending on Qt internals without QCoreApplication
    EXPECT_TRUE(signalReceived);
}

// ❌ TEST 2: Testing private state instead of the signal
// Accessing internal variables tells you nothing about whether the signal fired.
TEST(SensorBadTest, TemperatureStoredInternally)
{
    Sensor sensor;
    sensor.setTemperature(50.0f);

    // ❌ temperature_() is private — this only compiles if made public for tests,
    //    which pollutes the production API
    EXPECT_FLOAT_EQ(sensor.temperature_, 50.0f);
}

// ❌ TEST 3: Assuming queued connections are synchronous
// Queued connections are processed by the event loop. Without
// QCoreApplication::processEvents() the slot never runs before the assertion.
class Receiver : public QObject {
    Q_OBJECT
public:
    float lastTemp = 0.0f;
public slots:
    void onTemperature(float t) { lastTemp = t; }
};

TEST(SensorBadTest, QueuedSlotCalledBeforeAssertion)
{
    Sensor   sensor;
    Receiver receiver;

    // Qt::QueuedConnection — deferred until event loop processes it
    QObject::connect(&sensor, &Sensor::temperatureChanged,
                     &receiver, &Receiver::onTemperature,
                     Qt::QueuedConnection);

    sensor.setTemperature(55.0f);

    // ❌ Event loop never ran — receiver.lastTemp is still 0.0f
    EXPECT_FLOAT_EQ(receiver.lastTemp, 55.0f);
}

// ❌ TEST 4: No assertion on signal count — test proves nothing
TEST(SensorBadTest, SignalCountNotChecked)
{
    Sensor sensor;
    sensor.setTemperature(90.0f);
    sensor.setTemperature(95.0f);
    // ❌ No assertion — the test always passes, even if signals are never emitted
    SUCCEED();
}
