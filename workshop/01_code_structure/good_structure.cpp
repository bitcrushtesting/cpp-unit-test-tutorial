// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// ✅ GOOD: Business logic is isolated behind interfaces.
//          Hardware and library calls are confined to concrete adapters.
//          The TemperatureMonitor class is fully testable without hardware.

#include <cstdint>
#include <cstdio>
#include <memory>

// ─── Interfaces (pure abstractions) ──────────────────────────────────────────

class ITemperatureSensor {
public:
    virtual ~ITemperatureSensor() = default;
    virtual float readCelsius() = 0;
};

class IAlertOutput {
public:
    virtual ~IAlertOutput() = default;
    virtual void setAlert(bool active) = 0;
};

// ─── Business Logic (no hardware dependency) ─────────────────────────────────

struct MonitorConfig {
    float thresholdCelsius = 80.0f;
};

class TemperatureMonitor {
public:
    TemperatureMonitor(ITemperatureSensor& sensor,
                       IAlertOutput&       alert,
                       MonitorConfig       config = {})
        : sensor_(sensor), alert_(alert), config_(config) {}

    // ✅ Pure logic — testable with any fake sensor/alert
    bool check()
    {
        lastTemp_ = sensor_.readCelsius();
        bool over = lastTemp_ > config_.thresholdCelsius;
        alert_.setAlert(over);
        if (over) { ++alertCount_; }
        return over;
    }

    float lastTemperature() const { return lastTemp_; }
    int   alertCount()      const { return alertCount_; }

private:
    ITemperatureSensor& sensor_;
    IAlertOutput&       alert_;
    MonitorConfig       config_;
    float               lastTemp_  = 0.0f;
    int                 alertCount_ = 0;
};

// ─── Concrete Adapters (hardware/library calls isolated here) ─────────────────

// Guarded so this file can be #included from unit tests without requiring
// the vendor SDK.
#ifdef WORKSHOP_HARDWARE_ENABLED
#include "vendor/sensor.h"
#include "vendor/gpio.h"

class HardwareSensor : public ITemperatureSensor {
public:
    float readCelsius() override {
        return sensor_read_temperature(SENSOR_CHANNEL_0); // only place hardware is touched
    }
};

class GpioAlert : public IAlertOutput {
public:
    void setAlert(bool active) override {
        gpio_write(GPIO_PIN_ALERT, active ? 1 : 0);      // only place GPIO is touched
    }
};

// ─── Composition Root ─────────────────────────────────────────────────────────

#include <unistd.h>

int main()
{
    HardwareSensor sensor;
    GpioAlert      alert;
    TemperatureMonitor monitor(sensor, alert, {.thresholdCelsius = 80.0f});

    while (true) {
        if (monitor.check()) {
            printf("ALERT: %.1f°C (count=%d)\n",
                   monitor.lastTemperature(), monitor.alertCount());
        }
        sleep(1);
    }
}
#endif // WORKSHOP_HARDWARE_ENABLED
