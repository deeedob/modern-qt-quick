#include "SketchStorage.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSaveFile>
#include <QtCore/QSet>
#include <QtCore/QThread>

#include <glaze/json.hpp>

#include <algorithm>
#include <filesystem>
#include <optional>

template <>
struct glz::meta<app::canvas::PaperBackground>
{
    using enum app::canvas::PaperBackground;
    static constexpr auto value = enumerate("plain", Plain, "dots", Dots, "ruled", Ruled);
};

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcStorage, "app.storage")

namespace app::sketchbook {

struct ParsedSketchDocument
{
    std::string name;
    std::vector<std::shared_ptr<canvas::Stroke>> strokes;
    canvas::Page page;
};

class SketchStorageWorker : public QObject
{
public:
    using Mutation = SketchStorage::Mutation;

    SketchStorageWorker(SketchStorage *storage, QString directory)
        : m_storage(storage), m_directory(std::move(directory))
    {
    }

    void list()
    {
        std::vector<SketchEntry> entries;
        QSet<QString> ids;
        const QDir dir(m_directory);
        for (const auto &info : dir.entryInfoList({ "*.json"_L1 }, QDir::Files, QDir::Time)) {
            const auto id = info.completeBaseName();
            const auto [document, error] = read(id);
            if (!error.isEmpty()) {
                qCWarning(lcStorage) << "Skipping" << info.fileName() << error;
                continue;
            }
            ids.insert(id);
            entries.push_back({
                .id = id,
                .name = QString::fromStdString(document.name),
                .modified = info.lastModified(),
                .strokeCount = static_cast<qsizetype>(document.strokes.size()),
            });
        }
        for (auto it = m_states.cbegin(); it != m_states.cend(); ++it) {
            if (!it->removed && it->failed && !ids.contains(it.key())) {
                const auto &document = *it->failed;
                entries.push_back({
                    .id = it.key(),
                    .name = QString::fromStdString(document.name),
                    .modified = QDateTime::currentDateTime(),
                    .strokeCount = static_cast<qsizetype>(document.strokes.size()),
                });
            }
        }
        post([storage = m_storage, entries = std::move(entries)]() mutable {
            Q_EMIT storage->listed(std::move(entries));
        });
    }

    void load(const QString &id, quint64 requestId)
    {
        auto [document, error] = read(id);
        post([storage = m_storage, id, requestId, result = std::move(document),
                 failure = std::move(error)]() mutable {
            Q_EMIT storage->loaded(id, requestId, std::move(result), std::move(failure));
        });
    }

    void save(const QString &id, quint64 revision, SketchDocument document)
    {
        mutate(id, id, revision, Mutation::Save, std::move(document));
    }

    void rename(const QString &id, quint64 revision, const QString &name)
    {
        auto &state = m_states[id];
        state.revision = revision;
        if (state.removed) {
            reportMutation(id, id, Mutation::Rename, revision, {}, false);
            return;
        }
        auto [document, error] = read(id);
        if (!error.isEmpty()) {
            reportMutation(id, id, Mutation::Rename, revision, error, false);
            return;
        }
        document.name = name.toStdString();
        finishMutation(id, id, revision, Mutation::Rename, std::move(document));
    }

    void duplicate(const QString &sourceId, const QString &id, quint64 revision,
        const QString &name)
    {
        auto &state = m_states[id];
        state.revision = revision;
        auto [document, error] = read(sourceId);
        if (!error.isEmpty()) {
            reportMutation(id, sourceId, Mutation::Duplicate, revision, error, false);
            return;
        }
        document.name = name.toStdString();
        finishMutation(id, sourceId, revision, Mutation::Duplicate, std::move(document));
    }

    void remove(const QString &id)
    {
        auto &state = m_states[id];
        QFile file(filePath(id));
        QString error;
        if (!file.remove()) {
            std::error_code statusError;
            const auto status = std::filesystem::symlink_status(file.filesystemFileName(),
                statusError);
            if (status.type() != std::filesystem::file_type::not_found) {
                error = file.errorString();
                qCWarning(lcStorage) << "Cannot remove" << file.fileName() << error;
            }
        }
        if (error.isEmpty()) {
            state.removed = true;
            state.failed.reset();
        }
        post([storage = m_storage, id, error] { storage->onRemoved(id, error); });
    }

    void retryFailedSaves()
    {
        for (auto it = m_states.begin(); it != m_states.end(); ++it) {
            if (!it->removed && it->failed) {
                const auto error = write(it.key(), *it->failed);
                if (error.isEmpty())
                    it->failed.reset();
                reportMutation(it.key(), it->sourceId, it->mutation, it->revision, error,
                    !error.isEmpty());
            }
        }
    }

    void stop() { QThread::currentThread()->quit(); }

private:
    template <typename Function>
    void post(Function function)
    {
        QMetaObject::invokeMethod(m_storage, std::move(function), Qt::QueuedConnection);
    }

    void reportMutation(const QString &id, const QString &sourceId, Mutation mutation,
        quint64 revision, const QString &error, bool retryable)
    {
        post([storage = m_storage, id, sourceId, mutation, revision, error, retryable] {
            storage->onMutationFinished(id, sourceId, mutation, revision, error, retryable);
        });
    }

    struct State
    {
        quint64 revision = 0;
        std::optional<SketchDocument> failed;
        QString sourceId;
        Mutation mutation = Mutation::Save;
        bool removed = false;
    };

    void mutate(const QString &id, const QString &sourceId, quint64 revision, Mutation mutation,
        SketchDocument document)
    {
        auto &state = m_states[id];
        state.revision = revision;
        state.sourceId = sourceId;
        state.mutation = mutation;
        if (state.removed) {
            reportMutation(id, sourceId, mutation, revision, {}, false);
            return;
        }
        finishMutation(id, sourceId, revision, mutation, std::move(document));
    }

    void finishMutation(const QString &id, const QString &sourceId, quint64 revision,
        Mutation mutation, SketchDocument document)
    {
        auto &state = m_states[id];
        state.sourceId = sourceId;
        state.mutation = mutation;
        const auto error = write(id, document);
        if (error.isEmpty())
            state.failed.reset();
        else
            state.failed = std::move(document);
        reportMutation(id, sourceId, mutation, revision, error, !error.isEmpty());
    }

    [[nodiscard]] std::pair<SketchDocument, QString> read(const QString &id) const
    {
        const auto state = m_states.constFind(id);
        if (state != m_states.cend() && state->removed)
            return { {}, QCoreApplication::translate("SketchStorage", "Sketch was deleted") };
        if (state != m_states.cend() && state->failed)
            return { *state->failed, {} };
        QFile file(filePath(id));
        if (!file.open(QIODevice::ReadOnly))
            return { {}, file.errorString() };
        const auto json = file.readAll().toStdString();
        ParsedSketchDocument parsed;
        if (const auto error = glz::read<glz::opts{ .error_on_missing_keys = true }>(parsed, json))
            return { {}, QString::fromStdString(glz::format_error(error, json)) };
        if (std::ranges::any_of(parsed.strokes, [](const auto &stroke) { return !stroke; }))
            return { {},
                QCoreApplication::translate("SketchStorage", "Sketch contains an invalid stroke") };
        parsed.page.spacing = std::clamp(parsed.page.spacing, 8, 128);
        parsed.page.thickness = std::clamp(parsed.page.thickness, 1, 8);
        return {
            {
             .name = std::move(parsed.name),
             .strokes = { parsed.strokes.begin(), parsed.strokes.end() },
             .page = parsed.page,
             },
            {},
        };
    }

    [[nodiscard]] QString write(const QString &id, const SketchDocument &document) const
    {
        const auto path = filePath(id);
        const auto failed = [&path](const QString &reason) {
            qCWarning(lcStorage) << "Cannot save" << path << reason;
            return reason;
        };
        const auto json = glz::write_json(document);
        if (!json)
            return failed(QCoreApplication::translate("SketchStorage", "Cannot serialize sketch"));
        if (!QDir().mkpath(QFileInfo(path).absolutePath()))
            return failed(QCoreApplication::translate("SketchStorage",
                "Cannot create the sketch directory"));
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly))
            return failed(file.errorString());
        const auto size = static_cast<qint64>(json->size());
        if (file.write(json->data(), size) != size || !file.commit())
            return failed(file.errorString());
        return {};
    }

    [[nodiscard]] QString filePath(const QString &id) const
    {
        return QDir(m_directory).filePath(id + ".json"_L1);
    }

    SketchStorage *m_storage;
    QString m_directory;
    QHash<QString, State> m_states;
};

SketchStorage::SketchStorage(QString directory, QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<SketchDocument>();
    qRegisterMetaType<SketchEntry>();
    qRegisterMetaType<std::vector<SketchEntry>>();
    auto *worker = new SketchStorageWorker(this, std::move(directory));
    m_worker = worker;
    worker->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, worker, &QObject::deleteLater);
    m_thread.start();
}

SketchStorage::~SketchStorage()
{
    auto *worker = m_worker;
    QMetaObject::invokeMethod(worker, [worker] { worker->stop(); }, Qt::QueuedConnection);
    m_thread.wait();
}

void SketchStorage::list()
{
    QMetaObject::invokeMethod(
        m_worker, [worker = m_worker] { worker->list(); }, Qt::QueuedConnection);
}

void SketchStorage::load(const QString &id, quint64 requestId)
{
    QMetaObject::invokeMethod(
        m_worker, [worker = m_worker, id, requestId] { worker->load(id, requestId); },
        Qt::QueuedConnection);
}

quint64 SketchStorage::markDirty(const QString &id)
{
    const auto revision = ++m_nextRevision;
    const bool wasDirty = hasUnsavedChanges();
    m_dirty.insert(id, revision);
    if (!wasDirty)
        Q_EMIT unsavedChangesChanged();
    return revision;
}

void SketchStorage::save(const QString &id, SketchDocument document)
{
    const auto revision = markDirty(id);
    QMetaObject::invokeMethod(
        m_worker,
        [worker = m_worker, id, revision, document = std::move(document)]() mutable {
            worker->save(id, revision, std::move(document));
        },
        Qt::QueuedConnection);
}

void SketchStorage::rename(const QString &id, QString name)
{
    const auto revision = markDirty(id);
    QMetaObject::invokeMethod(
        m_worker,
        [worker = m_worker, id, revision, name = std::move(name)] {
            worker->rename(id, revision, name);
        },
        Qt::QueuedConnection);
}

void SketchStorage::duplicate(const QString &sourceId, QString id, QString name)
{
    const auto revision = markDirty(id);
    QMetaObject::invokeMethod(
        m_worker,
        [worker = m_worker, sourceId, id = std::move(id), revision,
            name = std::move(name)]() mutable { worker->duplicate(sourceId, id, revision, name); },
        Qt::QueuedConnection);
}

void SketchStorage::remove(const QString &id)
{
    QMetaObject::invokeMethod(
        m_worker, [worker = m_worker, id] { worker->remove(id); }, Qt::QueuedConnection);
}

void SketchStorage::retryFailedSaves()
{
    QMetaObject::invokeMethod(
        m_worker, [worker = m_worker] { worker->retryFailedSaves(); }, Qt::QueuedConnection);
}

void SketchStorage::onMutationFinished(const QString &id, const QString &sourceId,
    Mutation mutation, quint64 revision, const QString &error, bool retryable)
{
    auto it = m_dirty.find(id);
    if (it == m_dirty.end() || it.value() != revision)
        return;
    if (error.isEmpty() || !retryable) {
        m_dirty.erase(it);
        if (m_dirty.isEmpty())
            Q_EMIT unsavedChangesChanged();
    }
    Q_EMIT mutationFinished(id, sourceId, mutation, error, retryable);
}

void SketchStorage::onRemoved(const QString &id, const QString &error)
{
    if (error.isEmpty() && m_dirty.remove(id) != 0 && m_dirty.isEmpty())
        Q_EMIT unsavedChangesChanged();
    Q_EMIT removed(id, error);
}

} // namespace app::sketchbook
