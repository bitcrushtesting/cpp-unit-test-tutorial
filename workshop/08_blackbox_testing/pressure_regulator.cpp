// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

/// @file pressure_regulator.cpp
/// @brief Implementation of PressureRegulator.
///
/// This file is intentionally not distributed to workshop participants
/// writing black-box tests. Tests are written against pressure_regulator.h
/// and the Doxygen documentation only.

#include "pressure_regulator.h"
#include <memory>

// ── Pimpl idiom: private state hidden from the public header ──────────────────
// Black-box testers see only the public interface in pressure_regulator.h.
// They have no access to this struct and cannot depend on its layout.

struct PressureRegulator::Impl {
    IPressureSensor& sensor;
    IValve&          valve;
    RegulatorConfig  config;
    RegulatorState   state        = RegulatorState::Idle;
    float            lastPressure = 0.0f;
    int              faultCount   = 0;

    Impl(IPressureSensor& s, IValve& v, RegulatorConfig c)
        : sensor(s), valve(v), config(c) {}
};

// ── Public API ────────────────────────────────────────────────────────────────

PressureRegulator::PressureRegulator(IPressureSensor& sensor,
                                     IValve&          valve,
                                     RegulatorConfig  config)
    : impl_(std::make_unique<Impl>(sensor, valve, config))
{}

PressureRegulator::~PressureRegulator() = default;

void PressureRegulator::start()
{
    if (impl_->state == RegulatorState::Idle) {
        impl_->state = RegulatorState::Regulating;
    }
}

void PressureRegulator::stop()
{
    if (impl_->state != RegulatorState::Idle) {
        impl_->valve.close();
        impl_->state = RegulatorState::Idle;
    }
}

void PressureRegulator::reset()
{
    if (impl_->state == RegulatorState::Fault) {
        impl_->lastPressure = 0.0f;
        impl_->state = RegulatorState::Idle;
    }
}

void PressureRegulator::update()
{
    if (impl_->state == RegulatorState::Idle ||
        impl_->state == RegulatorState::Fault) {
        return;
    }

    impl_->lastPressure = impl_->sensor.readMbar();

    if (impl_->lastPressure > impl_->config.maxSafeMbar) {
        impl_->valve.close();
        ++impl_->faultCount;
        impl_->state = RegulatorState::Fault;
        return;
    }

    float low  = impl_->config.targetMbar - impl_->config.deadbandMbar;
    float high = impl_->config.targetMbar + impl_->config.deadbandMbar;

    if (impl_->lastPressure < low) {
        impl_->valve.open();
        impl_->state = RegulatorState::Increasing;
    } else if (impl_->lastPressure > high) {
        impl_->valve.close();
        impl_->state = RegulatorState::Decreasing;
    } else {
        impl_->state = RegulatorState::Regulating;
    }
}

RegulatorState PressureRegulator::state() const { return impl_->state;        }
float          PressureRegulator::lastPressureMbar() const { return impl_->lastPressure; }
int            PressureRegulator::faultCount() const { return impl_->faultCount; }
