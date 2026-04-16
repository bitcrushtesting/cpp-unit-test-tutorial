// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// ✅ GOOD: Vendor library is wrapped in an adapter behind an interface.
//          Business logic (DataLogger) has zero knowledge of vendor types.

#include <string>
#include <vector>
#include <cstdint>

// ─── Interface ────────────────────────────────────────────────────────────────

class ISerialPort {
public:
    virtual ~ISerialPort() = default;
    virtual void write(const uint8_t* data, std::size_t length) = 0;
};

// ─── Business Logic ───────────────────────────────────────────────────────────

// ✅ DataLogger only depends on ISerialPort — completely hardware-agnostic
class DataLogger {
public:
    explicit DataLogger(ISerialPort& port) : port_(port) {}

    void log(const std::string& message) {
        std::string framed = "[LOG] " + message + "\r\n";
        port_.write(reinterpret_cast<const uint8_t*>(framed.c_str()), framed.size());
    }

private:
    ISerialPort& port_;
};

// ─── Hardware Adapter (vendor SDK isolated here) ──────────────────────────────

// Guarded so the test section below compiles without the vendor SDK.
#ifdef WORKSHOP_HARDWARE_ENABLED
#include "vendor/uart_driver.h"  // only this file needs the vendor header

class UartSerialPort : public ISerialPort {
public:
    explicit UartSerialPort(int port, uint32_t baud) : port_(port) {
        uart_init(port_, baud);
    }

    void write(const uint8_t* data, std::size_t length) override {
        uart_write(port_,
                   data,
                   static_cast<uint16_t>(length));
    }

    ~UartSerialPort() {
        uart_deinit(port_);
    }

private:
    int port_;
};

// ─── Composition Root ─────────────────────────────────────────────────────────

void runApplication() {
    UartSerialPort uart(UART_PORT_1, 115200); // ✅ hardware only created here
    DataLogger logger(uart);
    logger.log("System started");
}
#endif // WORKSHOP_HARDWARE_ENABLED

// ════════════════════════════════════════════════════════════════════════════
// good_hw_tests.cpp — tests for DataLogger without real hardware
// ════════════════════════════════════════════════════════════════════════════

#include <gtest/gtest.h>

// ✅ In-memory fake — records bytes written, no hardware involved
class FakeSerialPort : public ISerialPort {
public:
    void write(const uint8_t* data, std::size_t length) override {
        written_.insert(written_.end(), data, data + length);
        callCount_++;
    }

    std::string writtenString() const {
        return std::string(written_.begin(), written_.end());
    }

    int callCount() const { return callCount_; }
    void reset() { written_.clear(); callCount_ = 0; }

private:
    std::vector<uint8_t> written_;
    int callCount_ = 0;
};

// ─── Tests ────────────────────────────────────────────────────────────────────

class DataLoggerTest : public ::testing::Test {
protected:
    FakeSerialPort port;
    DataLogger     logger{port};
};

// ✅ Verify framing format without any hardware
TEST_F(DataLoggerTest, LogMessageIsFramedWithPrefixAndLineEnding)
{
    logger.log("hello");
    EXPECT_EQ(port.writtenString(), "[LOG] hello\r\n");
}

// ✅ Verify empty message is handled gracefully
TEST_F(DataLoggerTest, EmptyMessageProducesOnlyFraming)
{
    logger.log("");
    EXPECT_EQ(port.writtenString(), "[LOG] \r\n");
}

// ✅ Verify each log call invokes write exactly once
TEST_F(DataLoggerTest, EachLogCallWritesOnce)
{
    logger.log("a");
    logger.log("b");
    EXPECT_EQ(port.callCount(), 2);
}

// ✅ Integration test — clearly tagged; only run on target with real UART
// Run with: ./tests --gtest_filter="*HwIntegration*"
// Skip with: ./tests --gtest_filter="-*HwIntegration*"
#ifdef WORKSHOP_HARDWARE_ENABLED
TEST(DataLoggerHwIntegration, WritesToRealUart)
{
    UartSerialPort uart(UART_PORT_1, 115200);
    DataLogger logger(uart);
    logger.log("HW integration test");
    // No assertion — success means no crash/timeout on real hardware
    SUCCEED();
}
#endif // WORKSHOP_HARDWARE_ENABLED
