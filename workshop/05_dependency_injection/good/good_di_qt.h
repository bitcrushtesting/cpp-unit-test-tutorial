// =============================================================================
// Bitcrush Testing 2026
// =============================================================================

// good_di_qt.h
// Qt-compatible interfaces for dependency injection.
// Included by both good_di_qt.cpp (implementations) and di_tests.cpp (tests).
//
// Pure abstract bases have no Q_OBJECT — only concrete subclasses do.

#pragma once

#include <QObject>
#include <QString>
#include <QElapsedTimer>
#include <QDebug>

// ── Interfaces ────────────────────────────────────────────────────────────────

class IQLogger {
public:
    virtual ~IQLogger() = default;
    virtual void log(const QString& message) = 0;
};

class IQClock {
public:
    virtual ~IQClock() = default;
    virtual qint64 elapsedMs() = 0;
};

// ── Business Logic ────────────────────────────────────────────────────────────
// QtDeviceMonitor has Q_OBJECT — defined here in the header so both
// good_di_qt.cpp (which provides the .moc include) and di_tests.cpp
// (which uses QSignalSpy on its signals) see the same definition.

class QtDeviceMonitor : public QObject {
    Q_OBJECT
public:
    explicit QtDeviceMonitor(IQLogger& logger, IQClock& clock,
                             QObject* parent = nullptr)
        : QObject(parent), logger_(logger), clock_(clock) {}

    void reportStatus() {
        qint64 elapsed = clock_.elapsedMs();
        logger_.log(QString("Elapsed: %1 ms").arg(elapsed));
        emit statusReported(elapsed);
    }

    bool isOverdue(qint64 thresholdMs) {
        return clock_.elapsedMs() > thresholdMs;
    }

signals:
    void statusReported(qint64 elapsedMs);

private:
    IQLogger& logger_;
    IQClock&  clock_;
};

// ── Concrete Implementations ──────────────────────────────────────────────────
// Production implementations — defined in the header so main.cpp can use them
// without needing to compile good_di_qt.cpp separately.

class QtDebugLogger : public QObject, public IQLogger {
    Q_OBJECT
public:
    explicit QtDebugLogger(QObject* parent = nullptr) : QObject(parent) {}
    void log(const QString& message) override { qDebug() << message; }
};

class QtElapsedClock : public QObject, public IQClock {
    Q_OBJECT
public:
    explicit QtElapsedClock(QObject* parent = nullptr) : QObject(parent) {
        timer_.start();
    }
    qint64 elapsedMs() override { return timer_.elapsed(); }
private:
    QElapsedTimer timer_;
};
