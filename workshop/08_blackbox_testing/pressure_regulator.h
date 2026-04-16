// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

#pragma once

/// @file pressure_regulator.h
/// @brief Pressure regulation logic for a closed-loop fluid system.
///
/// This module implements a software pressure regulator that reads a pressure
/// sensor, applies a configurable deadband, and drives an output valve to keep
/// the measured pressure within a target range.
///
/// The implementation is hidden. Tests are written against this header only.

#include <cstdint>
#include <memory>

// ── Configuration ─────────────────────────────────────────────────────────────

/// @brief Configuration parameters for PressureRegulator.
///
/// All pressure values are in millibar (mbar).
struct RegulatorConfig {
    /// Target pressure the regulator aims to maintain (mbar).
    /// Must be greater than zero.
    float targetMbar = 1000.0f;

    /// Symmetric deadband around the target (mbar).
    /// The valve is not adjusted while measured pressure is within
    /// [target - deadband, target + deadband].
    /// Must be greater than or equal to zero.
    float deadbandMbar = 50.0f;

    /// Maximum pressure the system is permitted to reach (mbar).
    /// If measured pressure exceeds this value the regulator enters
    /// the @ref RegulatorState::Fault state and closes the valve immediately.
    /// Must be greater than targetMbar.
    float maxSafeMbar = 1500.0f;
};

// ── State ─────────────────────────────────────────────────────────────────────

/// @brief Operational states of the PressureRegulator.
enum class RegulatorState {
    /// The regulator has not been started or has been explicitly stopped.
    Idle,

    /// The regulator is active and pressure is within the deadband.
    Regulating,

    /// Pressure is above the target deadband; the valve is being opened.
    Increasing,

    /// Pressure is below the target deadband; the valve is being closed.
    Decreasing,

    /// Measured pressure exceeded @ref RegulatorConfig::maxSafeMbar.
    /// The valve is held closed. Call @ref PressureRegulator::reset() to
    /// return to @ref RegulatorState::Idle.
    Fault,
};

// ── Interfaces ────────────────────────────────────────────────────────────────

/// @brief Pressure sensor interface.
///
/// Implementations read the current system pressure in millibar.
class IPressureSensor {
public:
    virtual ~IPressureSensor() = default;

    /// @brief Read the current pressure.
    /// @return Pressure in millibar (mbar). Returns 0.0f on sensor failure.
    virtual float readMbar() = 0;
};

/// @brief Valve control interface.
///
/// The valve controls the flow of fluid into the regulated system.
/// Opening the valve increases pressure; closing it allows pressure to drop.
class IValve {
public:
    virtual ~IValve() = default;

    /// @brief Open the valve to allow fluid flow.
    virtual void open() = 0;

    /// @brief Close the valve to stop fluid flow.
    virtual void close() = 0;

    /// @brief Query the current valve state.
    /// @return true if the valve is currently open.
    virtual bool isOpen() const = 0;
};

// ── PressureRegulator ─────────────────────────────────────────────────────────

/// @brief Closed-loop pressure regulator.
///
/// PressureRegulator reads a pressure sensor on each call to @ref update()
/// and adjusts a valve to maintain the configured target pressure.
///
/// ### Lifecycle
///
/// 1. Construct with a sensor, valve, and configuration.
/// 2. Call @ref start() to begin regulation.
/// 3. Call @ref update() periodically (e.g. from a timer callback).
/// 4. Call @ref stop() to halt regulation and close the valve.
/// 5. If @ref state() returns @ref RegulatorState::Fault, call @ref reset()
///    before restarting.
///
/// ### Deadband behaviour
///
/// While the measured pressure is within
/// [@ref RegulatorConfig::targetMbar - @ref RegulatorConfig::deadbandMbar,
///  @ref RegulatorConfig::targetMbar + @ref RegulatorConfig::deadbandMbar],
/// the valve position is not changed and the state is @ref RegulatorState::Regulating.
///
/// ### Fault condition
///
/// If @ref update() reads a pressure above @ref RegulatorConfig::maxSafeMbar,
/// the valve is closed immediately and the state transitions to
/// @ref RegulatorState::Fault. No further valve changes occur until
/// @ref reset() is called.
///
/// ### Thread safety
///
/// PressureRegulator is not thread-safe. The caller is responsible for
/// ensuring that @ref update(), @ref stop(), and @ref reset() are not called
/// concurrently.
class PressureRegulator {
public:
    /// @brief Construct a PressureRegulator.
    /// @param sensor  Pressure sensor to read from. Must outlive this object.
    /// @param valve   Valve to control. Must outlive this object.
    /// @param config  Regulator configuration. Copied at construction.
    PressureRegulator(IPressureSensor& sensor,
                      IValve&          valve,
                      RegulatorConfig  config = {});

    /// @brief Start pressure regulation.
    ///
    /// Transitions from @ref RegulatorState::Idle to active regulation.
    /// Has no effect if the regulator is already active or in
    /// @ref RegulatorState::Fault.
    void start();

    /// @brief Stop pressure regulation and close the valve.
    ///
    /// Transitions to @ref RegulatorState::Idle and closes the valve
    /// regardless of current pressure. Has no effect if already idle.
    void stop();

    /// @brief Reset a faulted regulator.
    ///
    /// Transitions from @ref RegulatorState::Fault to
    /// @ref RegulatorState::Idle. Has no effect in any other state.
    /// The valve remains closed after reset.
    void reset();

    /// @brief Read the sensor and adjust the valve accordingly.
    ///
    /// Must be called periodically while the regulator is active.
    /// Has no effect when the state is @ref RegulatorState::Idle or
    /// @ref RegulatorState::Fault.
    ///
    /// @post @ref lastPressureMbar() returns the value read during this call.
    void update();

    /// @brief Return the current operational state.
    /// @return The current @ref RegulatorState.
    RegulatorState state() const;

    /// @brief Return the pressure reading from the most recent @ref update() call.
    ///
    /// Returns 0.0f if @ref update() has not been called since construction
    /// or since the last @ref reset().
    /// @return Last measured pressure in mbar.
    float lastPressureMbar() const;

    /// @brief Return the number of times the fault condition has been triggered
    ///        since construction.
    /// @return Fault count. Never decremented by @ref reset().
    int faultCount() const;

    /// @cond INTERNAL
    ~PressureRegulator();  // required by pimpl; defined in .cpp
    /// @endcond

private:
    struct Impl;                    ///< @cond INTERNAL
    std::unique_ptr<Impl> impl_;    ///< @endcond
};
