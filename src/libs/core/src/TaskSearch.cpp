// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include <app/core/TaskSearch.hpp>

#include <rapidfuzz/fuzz.hpp>

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>

namespace app::core {

namespace {

    constexpr double MinScore = 70.0;

    std::string toLower(std::string_view text)
    {
        std::string lower(text);
        std::ranges::transform(lower, lower.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return lower;
    }

} // namespace

std::vector<SearchHit> rankTasks(std::span<const Task> tasks, std::string_view query)
{
    std::vector<SearchHit> hits;
    hits.reserve(tasks.size());
    if (query.empty()) {
        for (std::size_t i = 0; i < tasks.size(); ++i)
            hits.push_back({ .index = i, .score = 100.0 });
        return hits;
    }

    // partial_ratio scores the best-matching substring, so short queries match long titles.
    const rapidfuzz::fuzz::CachedPartialRatio<char> scorer(toLower(query));
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        const double score = scorer.similarity(toLower(tasks[i].title), MinScore);
        if (score >= MinScore)
            hits.push_back({ .index = i, .score = score });
    }
    std::ranges::stable_sort(hits, std::ranges::greater{}, &SearchHit::score);
    return hits;
}

} // namespace app::core
