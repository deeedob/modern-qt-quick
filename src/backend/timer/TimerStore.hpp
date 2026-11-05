// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <app/core/FocusTimer.hpp>

#include <QtCore/QChronoTimer>
#include <QtCore/QObject>
#include <QtQmlIntegration/qqmlintegration.h>

#include <chrono>

namespace app::backend {

// Flux store: views dispatch actions, core::reduce() computes the state, stateChanged() announces it.
class TimerStore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created in main() and passed to Main.qml as a required property")
    Q_PROPERTY(int durationMinutes READ durationMinutes NOTIFY stateChanged FINAL)
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool running READ running NOTIFY stateChanged FINAL)
    Q_PROPERTY(double progress READ progress NOTIFY stateChanged FINAL)

public:
    enum class Action { Start, Pause, Reset, SetDuration };
    Q_ENUM(Action)

    explicit TimerStore(QObject *parent = nullptr);

    // `minutes` is the payload of SetDuration and ignored otherwise.
    Q_INVOKABLE void dispatch(Action action, int minutes = 0);

    [[nodiscard]] int durationMinutes() const;
    [[nodiscard]] int remainingSeconds() const;
    [[nodiscard]] bool running() const;
    [[nodiscard]] double progress() const;

signals:
    void stateChanged();
    void finished();

private:
    void apply(const core::TimerAction &action);
    void tick();

    core::TimerState m_state;
    QChronoTimer m_ticker;
    std::chrono::steady_clock::time_point m_lastTick;
};

} // namespace app::backend
