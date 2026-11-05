// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <app/core/Task.hpp>

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace app::core {

struct SearchHit
{
    std::size_t index = 0;
    double score = 0.0;
};

// Fuzzy ranking by title, best first, ignoring ASCII case. An empty query keeps every task in order.
[[nodiscard]] std::vector<SearchHit> rankTasks(std::span<const Task> tasks, std::string_view query);

} // namespace app::core
