// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "JsonTaskRepository.hpp"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSaveFile>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcStorage, "app.storage")

namespace app::platform {

JsonTaskRepository::JsonTaskRepository(QString filePath)
    : m_filePath(std::move(filePath))
{
}

std::expected<std::vector<core::Task>, std::string> JsonTaskRepository::load()
{
    QFile file(m_filePath);
    if (!file.exists())
        return std::vector<core::Task>{};
    if (!file.open(QIODevice::ReadOnly))
        return std::unexpected(file.errorString().toStdString());

    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return std::unexpected(parseError.errorString().toStdString());
    // Anything else would load as "no tasks", and the next save would overwrite the user's file.
    if (!document.isArray())
        return std::unexpected("expected a JSON array of tasks");

    std::vector<core::Task> tasks;
    for (const QJsonValue value : document.array()) {
        const auto object = value.toObject();
        tasks.push_back({
            .title = object["title"_L1].toString().toStdString(),
            .done = object["done"_L1].toBool(),
        });
    }
    qCDebug(lcStorage) << "Loaded" << tasks.size() << "tasks from" << m_filePath;
    return tasks;
}

std::expected<void, std::string> JsonTaskRepository::save(std::span<const core::Task> tasks)
{
    QJsonArray array;
    for (const auto &task : tasks)
        array.append(QJsonObject{
            { "title"_L1, QString::fromStdString(task.title) },
            { "done"_L1,  task.done                          }
        });

    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
        return std::unexpected(file.errorString().toStdString());
    file.write(QJsonDocument(array).toJson());
    if (!file.commit())
        return std::unexpected(file.errorString().toStdString());
    return {};
}

} // namespace app::platform
