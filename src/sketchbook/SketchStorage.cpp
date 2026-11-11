#include "SketchStorage.hpp"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSaveFile>

#include <glaze/json.hpp>

#include <algorithm>

template <>
struct glz::meta<app::canvas::PaperBackground>
{
    using enum app::canvas::PaperBackground;
    static constexpr auto value = enumerate("plain", Plain, "dots", Dots, "ruled", Ruled);
};

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcStorage, "app.storage")

namespace app::sketchbook {

struct ReadableDocument
{
    std::string name;
    std::vector<std::shared_ptr<canvas::Stroke>> strokes;
    canvas::Page page;
};

SketchStorage::SketchStorage(QString directory)
    : m_directory(std::move(directory))
{
    // One thread: saves of the same sketch must reach the disk in the order they were made.
    m_writer.setMaxThreadCount(1);
}

std::vector<SketchEntry> SketchStorage::list()
{
    waitForSaves();
    std::vector<SketchEntry> entries;
    const QDir dir(m_directory);
    for (const auto &info : dir.entryInfoList({ "*.json"_L1 }, QDir::Files, QDir::Time)) {
        const auto id = info.completeBaseName();
        if (const auto document = load(id)) {
            entries.push_back({
                .id = id,
                .name = QString::fromStdString(document->name),
                .modified = info.lastModified(),
                .strokeCount = static_cast<qsizetype>(document->strokes.size()),
            });
        } else {
            qCWarning(lcStorage) << "Skipping" << info.fileName() << document.error();
        }
    }
    return entries;
}

std::expected<SketchDocument, QString> SketchStorage::load(const QString &id)
{
    waitForSaves();
    QFile file(filePath(id));
    if (!file.open(QIODevice::ReadOnly))
        return std::unexpected(file.errorString());
    const auto json = file.readAll().toStdString();

    ReadableDocument parsed;
    if (const auto error = glz::read<glz::opts{ .error_on_missing_keys = true }>(parsed, json))
        return std::unexpected(QString::fromStdString(glz::format_error(error, json)));
    parsed.page.spacing = std::clamp(parsed.page.spacing, 8, 128);
    parsed.page.thickness = std::clamp(parsed.page.thickness, 1, 8);
    return SketchDocument{
        .name = std::move(parsed.name),
        .strokes = { parsed.strokes.begin(), parsed.strokes.end() },
        .page = parsed.page,
    };
}

void SketchStorage::save(const QString &id, SketchDocument document)
{
    // The strokes are immutable and shared with the canvas: the worker reads them without a lock or a copy.
    std::ignore = QtConcurrent::run(&m_writer,
        [path = filePath(id), document = std::move(document)]() mutable {
            const auto json = glz::write_json(document);
            if (!json) {
                qCWarning(lcStorage) << "Cannot serialize" << path;
                return;
            }
            QDir().mkpath(QFileInfo(path).absolutePath());
            QSaveFile file(path);
            if (!file.open(QIODevice::WriteOnly)
                || file.write(json->data(), static_cast<qint64>(json->size())) < 0
                || !file.commit())
                qCWarning(lcStorage) << "Cannot save" << path << file.errorString();
        });
}

void SketchStorage::remove(const QString &id)
{
    waitForSaves();
    if (!QFile::remove(filePath(id)))
        qCWarning(lcStorage) << "Cannot remove" << filePath(id);
}

void SketchStorage::waitForSaves()
{
    m_writer.waitForDone();
}

QString SketchStorage::filePath(const QString &id) const
{
    return QDir(m_directory).filePath(id + ".json"_L1);
}

} // namespace app::sketchbook
