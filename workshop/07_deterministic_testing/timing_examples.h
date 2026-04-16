// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// timing_examples.h
// Watchdog and ConnectionManager: examples of timing-dependent logic
// made deterministic by injecting IClock.

#pragma once
#include "iclock.h"

// ── Watchdog ──────────────────────────────────────────────────────────────────

class Watchdog {
public:
    Watchdog(IClock& clock, int64_t timeoutMs)
        : clock_(clock), timeoutMs_(timeoutMs), startMs_(clock.nowMs()) {}

    void reset() { startMs_ = clock_.nowMs(); }

    bool hasExpired() const {
        return (clock_.nowMs() - startMs_) >= timeoutMs_;
    }

private:
    IClock&  clock_;
    int64_t  timeoutMs_;
    int64_t  startMs_;
};

// ── ConnectionManager ────────────────────────────────────────────────────────

enum class ConnectionState { Disconnected, Connecting, Connected, Error };

class ConnectionManager {
public:
    explicit ConnectionManager(IClock& clock, int64_t timeoutMs = 1000)
        : clock_(clock), timeoutMs_(timeoutMs) {}

    void connect() {
        if (state_ == ConnectionState::Disconnected) {
            state_   = ConnectionState::Connecting;
            startMs_ = clock_.nowMs();
        }
    }

    void poll() {
        if (state_ == ConnectionState::Connecting) {
            if (clock_.nowMs() - startMs_ > timeoutMs_) {
                state_ = ConnectionState::Error;
            }
        }
    }

    void onConnected()    { state_ = ConnectionState::Connected;    }
    void onDisconnected() { state_ = ConnectionState::Disconnected; }

    ConnectionState state() const { return state_; }

private:
    IClock&         clock_;
    ConnectionState state_    = ConnectionState::Disconnected;
    int64_t         startMs_  = 0;
    int64_t         timeoutMs_;
};
