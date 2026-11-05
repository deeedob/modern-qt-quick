// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0
//
// The focus timer as Flux state with a pure reducer: no clock, no Qt, everything constexpr.

#pragma once

#include <algorithm>
#include <chrono>
#include <variant>

namespace app::core {

using Duration = std::chrono::milliseconds;

struct TimerState
{
    Duration duration = std::chrono::minutes{ 25 };
    Duration remaining = std::chrono::minutes{ 25 };
    bool running = false;

    friend constexpr bool operator==(const TimerState &, const TimerState &) = default;
};

namespace timer {
struct Start { };
struct Pause { };
struct Reset { };
struct Tick { Duration elapsed; };
struct SetDuration { Duration duration; };
} // namespace timer

using TimerAction = std::variant<timer::Start, timer::Pause, timer::Reset, timer::Tick, timer::SetDuration>;

namespace detail {
template<typename... Fs>
struct Overloaded : Fs...
{
    using Fs::operator()...;
};
} // namespace detail

[[nodiscard]] constexpr TimerState reduce(TimerState state, const TimerAction &action) noexcept
{
    return std::visit(
        detail::Overloaded{
            [&](timer::Start) {
                state.running = state.remaining > Duration::zero();
                return state;
            },
            [&](timer::Pause) {
                state.running = false;
                return state;
            },
            [&](timer::Reset) {
                return TimerState{ .duration = state.duration, .remaining = state.duration };
            },
            [&](timer::Tick tick) {
                if (state.running) {
                    state.remaining = std::max(state.remaining - tick.elapsed, Duration::zero());
                    state.running = state.remaining > Duration::zero();
                }
                return state;
            },
            [&](timer::SetDuration set) {
                if (set.duration <= Duration::zero())
                    return state;
                return TimerState{ .duration = set.duration, .remaining = set.duration };
            },
        },
        action);
}

// Elapsed fraction of the current run, from 0.0 (untouched) to 1.0 (finished).
[[nodiscard]] constexpr double progress(const TimerState &state) noexcept
{
    if (state.duration <= Duration::zero())
        return 0.0;
    const auto elapsed = state.duration - state.remaining;
    return static_cast<double>(elapsed.count()) / static_cast<double>(state.duration.count());
}

} // namespace app::core
