// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <app/core/TaskRepository.hpp>

// Test double for the storage port: view models are tested without touching the file system.
class InMemoryTaskRepository final : public app::core::TaskRepository
{
public:
    explicit InMemoryTaskRepository(std::vector<app::core::Task> tasks = {})
        : stored(std::move(tasks))
    {
    }

    std::expected<std::vector<app::core::Task>, std::string> load() override { return stored; }

    std::expected<void, std::string> save(std::span<const app::core::Task> tasks) override
    {
        stored.assign(tasks.begin(), tasks.end());
        return {};
    }

    std::vector<app::core::Task> stored;
};
