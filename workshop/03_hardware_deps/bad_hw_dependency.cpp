// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// ❌ BAD: Direct dependency on vendor UART library.
//         Cannot be compiled or tested without the vendor SDK installed.
//         Cannot be tested without physical UART hardware connected.

#include <string>
#include "vendor/uart_driver.h"   // vendor library — may not exist on dev machine

class DataLogger {
public:
    DataLogger() {
        // ❌ Hardware initialisation in constructor — no way to skip for tests
        uart_init(UART_PORT_1, 115200);
    }

    // ❌ Mixed concern: formatting + hardware transmission in one function
    void log(const std::string& message) {
        std::string framed = "[LOG] " + message + "\r\n";
        // ❌ Direct vendor call — untestable without hardware
        uart_write(UART_PORT_1,
                   reinterpret_cast<const uint8_t*>(framed.c_str()),
                   static_cast<uint16_t>(framed.size()));
    }

    ~DataLogger() {
        uart_deinit(UART_PORT_1); // ❌ hardware teardown in destructor
    }
};

// ❌ Callers cannot substitute a different output (file, mock, network)
void runApplication() {
    DataLogger logger;
    logger.log("System started");
    // ... application code
}
