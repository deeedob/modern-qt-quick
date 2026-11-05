// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0
//
// The reducer is constexpr, so these tests run inside the compiler: a failing STATIC_REQUIRE
// is a build error. The "relaxed" build of this file defines CATCH_CONFIG_RUNTIME_STATIC_REQUIRE
// and runs the same checks at runtime, where they can be debugged.

#include <app/core/FocusTimer.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace std::chrono_literals;
using namespace app::core;

namespace {

constexpr TimerState fiveMinutes{ .duration = 5min, .remaining = 5min };

constexpr TimerState apply(TimerState state, std::initializer_list<TimerAction> actions)
{
    for (const auto &action : actions)
        state = reduce(state, action);
    return state;
}

} // namespace

TEST_CASE("Start runs the timer and Pause stops it")
{
    STATIC_REQUIRE(apply(fiveMinutes, { timer::Start{} }).running);
    STATIC_REQUIRE_FALSE(apply(fiveMinutes, { timer::Start{}, timer::Pause{} }).running);
}

TEST_CASE("Ticks only count down while running")
{
    STATIC_REQUIRE(apply(fiveMinutes, { timer::Tick{ 1min } }).remaining == 5min);
    STATIC_REQUIRE(apply(fiveMinutes, { timer::Start{}, timer::Tick{ 1min } }).remaining == 4min);
}

TEST_CASE("The timer stops at zero")
{
    constexpr auto finished = apply(fiveMinutes, { timer::Start{}, timer::Tick{ 7min } });
    STATIC_REQUIRE(finished.remaining == 0ms);
    STATIC_REQUIRE_FALSE(finished.running);
    STATIC_REQUIRE_FALSE(apply(finished, { timer::Start{} }).running);
}

TEST_CASE("Reset restores the full duration")
{
    constexpr auto reset = apply(fiveMinutes,
        { timer::Start{}, timer::Tick{ 2min }, timer::Reset{} });
    STATIC_REQUIRE(reset == fiveMinutes);
}

TEST_CASE("SetDuration restarts from the new duration and rejects non-positive values")
{
    constexpr auto changed = apply(fiveMinutes, { timer::Start{}, timer::SetDuration{ 10min } });
    STATIC_REQUIRE(changed == TimerState{ .duration = 10min, .remaining = 10min });
    STATIC_REQUIRE(apply(fiveMinutes, { timer::SetDuration{ 0ms } }) == fiveMinutes);
}

TEST_CASE("Progress goes from 0 to 1")
{
    STATIC_REQUIRE(progress(fiveMinutes) == 0.0);
    STATIC_REQUIRE(progress(apply(fiveMinutes, { timer::Start{}, timer::Tick{ 150s } })) == 0.5);
    STATIC_REQUIRE(progress(apply(fiveMinutes, { timer::Start{}, timer::Tick{ 5min } })) == 1.0);
}
