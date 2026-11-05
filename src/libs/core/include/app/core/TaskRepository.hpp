// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <app/core/Task.hpp>

#include <expected>
#include <span>
#include <string>
#include <vector>

namespace app::core {

// Storage port: core defines what it needs, the platform layer decides how (dependency inversion).
class TaskRepository
{
public:
    virtual ~TaskRepository() = default;

    [[nodiscard]] virtual std::expected<std::vector<Task>, std::string> load() = 0;
    [[nodiscard]] virtual std::expected<void, std::string> save(std::span<const Task> tasks) = 0;

protected:
    TaskRepository() = default;
    TaskRepository(const TaskRepository &) = default;
    TaskRepository(TaskRepository &&) = default;
    TaskRepository &operator=(const TaskRepository &) = default;
    TaskRepository &operator=(TaskRepository &&) = default;
};

} // namespace app::core
