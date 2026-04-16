// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// ❌ BAD: Business logic is tangled with hardware I/O and global state.
//         This code cannot be unit tested without real hardware.

#include <cstdint>
#include <cstdio>
#include <unistd.h>         // hardware: read/write
#include "vendor/gpio.h"    // hardware library
#include "vendor/sensor.h"  // hardware library

// ❌ Global mutable state — tests cannot run in isolation
static float g_last_temperature = 0.0f;
static int   g_alert_count      = 0;

// ❌ Single function doing everything:
//    - reads hardware sensor
//    - applies business logic (threshold check)
//    - writes to hardware GPIO
//    - logs to stdout
//    - updates global state
void processTemperature()
{
    // Direct hardware call — impossible to replace in tests
    float temp = sensor_read_temperature(SENSOR_CHANNEL_0);

    g_last_temperature = temp;

    if (temp > 80.0f) {
        ++g_alert_count;
        // Direct hardware call mixed into business logic
        gpio_write(GPIO_PIN_ALERT, 1);
        printf("ALERT: temperature %.1f°C exceeds threshold\n", temp);
    } else {
        gpio_write(GPIO_PIN_ALERT, 0);
    }
}

// ❌ Caller has no way to inject a fake sensor or fake GPIO for testing
int main()
{
    while (true) {
        processTemperature();
        sleep(1);
    }
}
