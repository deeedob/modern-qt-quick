// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "TimerStore.hpp"

#include <QtCore/QLoggingCategory>

using namespace std::chrono_literals;

Q_STATIC_LOGGING_CATEGORY(lcTimer, "app.timer")

namespace app::backend {

TimerStore::TimerStore(QObject *parent)
    : QObject(parent), m_ticker(100ms)
{
    connect(&m_ticker, &QChronoTimer::timeout, this, &TimerStore::tick);
}

void TimerStore::dispatch(Action action, int minutes)
{
    qCDebug(lcTimer) << "dispatch" << action << minutes;
    switch (action) {
    case Action::Start:
        apply(core::timer::Start{});
        break;
    case Action::Pause:
        apply(core::timer::Pause{});
        break;
    case Action::Reset:
        apply(core::timer::Reset{});
        break;
    case Action::SetDuration:
        apply(core::timer::SetDuration{ std::chrono::minutes{ minutes } });
        break;
    }
}

int TimerStore::durationMinutes() const
{
    return static_cast<int>(
        std::chrono::duration_cast<std::chrono::minutes>(m_state.duration).count());
}

int TimerStore::remainingSeconds() const
{
    return static_cast<int>(std::chrono::ceil<std::chrono::seconds>(m_state.remaining).count());
}

bool TimerStore::running() const
{
    return m_state.running;
}

double TimerStore::progress() const
{
    return core::progress(m_state);
}

void TimerStore::apply(const core::TimerAction &action)
{
    const auto previous = m_state;
    m_state = core::reduce(m_state, action);
    if (m_state == previous)
        return;

    // Side effects live here, outside the pure reducer.
    if (m_state.running && !previous.running) {
        m_lastTick = std::chrono::steady_clock::now();
        m_ticker.start();
    } else if (!m_state.running) {
        m_ticker.stop();
    }
    emit stateChanged();
    if (previous.running && m_state.remaining == core::Duration::zero())
        emit finished();
}

void TimerStore::tick()
{
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<core::Duration>(now - m_lastTick);
    // Advance by what was counted, so the truncated remainder carries over to the next tick.
    m_lastTick += elapsed;
    apply(core::timer::Tick{ elapsed });
}

} // namespace app::backend
