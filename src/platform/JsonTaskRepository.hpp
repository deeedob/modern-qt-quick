// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <app/core/TaskRepository.hpp>

#include <QtCore/QString>

namespace app::platform {

// Stores tasks as a JSON array. Writes are atomic: a crash never leaves a half-written file.
class JsonTaskRepository final : public core::TaskRepository
{
public:
    explicit JsonTaskRepository(QString filePath);

    [[nodiscard]] std::expected<std::vector<core::Task>, std::string> load() override;
    [[nodiscard]] std::expected<void, std::string> save(std::span<const core::Task> tasks) override;

private:
    QString m_filePath;
};

} // namespace app::platform
