// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include <app/core/TaskSearch.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <ranges>
#include <vector>

using app::core::rankTasks;
using app::core::Task;

namespace {

std::vector<Task> sampleTasks()
{
    return { { .title = "Review slides" }, { .title = "Buy groceries" },
        { .title = "Prepare the Qt lecture" } };
}

std::vector<std::size_t> indices(std::span<const Task> all, std::string_view query)
{
    return rankTasks(all, query) | std::views::transform(&app::core::SearchHit::index)
        | std::ranges::to<std::vector>();
}

} // namespace

TEST_CASE("An empty query keeps every task in its original order")
{
    CHECK(indices(sampleTasks(), "") == std::vector<std::size_t>{ 0, 1, 2 });
}

TEST_CASE("Matching ignores case")
{
    CHECK(indices(sampleTasks(), "GROCERIES") == std::vector<std::size_t>{ 1 });
}

TEST_CASE("Typos still find the task")
{
    CHECK(indices(sampleTasks(), "reveiw") == std::vector<std::size_t>{ 0 });
    CHECK(indices(sampleTasks(), "lectrue") == std::vector<std::size_t>{ 2 });
}

TEST_CASE("Unrelated queries match nothing")
{
    CHECK(indices(sampleTasks(), "zzz").empty());
}

TEST_CASE("Better matches come first")
{
    const std::array similar{
        Task{ .title = "Plan the week" },
        Task{ .title = "Plant tomatoes" },
    };
    const auto hits = rankTasks(similar, "plant");
    REQUIRE(hits.size() == 2);
    CHECK(hits[0].index == 1);
    CHECK(hits[0].score > hits[1].score);
}
