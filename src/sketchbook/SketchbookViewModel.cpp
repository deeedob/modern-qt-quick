#include "SketchbookViewModel.hpp"

#include <QtCore/QLoggingCategory>
#include <QtCore/QUuid>
#include <Message.hpp>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcSketchbook, "app.sketchbook")

namespace app::sketchbook {

SketchbookViewModel::SketchbookViewModel(SketchStorage &storage, canvas::CanvasStore &canvas,
    notify::NotificationCenter &notifications, QObject *parent)
    : QObject(parent), m_storage(storage), m_canvas(canvas), m_notifications(notifications)
{
    connect(&m_storage, &SketchStorage::listed, this, &SketchbookViewModel::onListed);
    connect(&m_storage, &SketchStorage::loaded, this, &SketchbookViewModel::onLoaded);
    connect(&m_storage, &SketchStorage::mutationFinished, this,
        &SketchbookViewModel::onMutationFinished);
    connect(&m_storage, &SketchStorage::removed, this, &SketchbookViewModel::onRemoved);
    connect(&m_storage, &SketchStorage::unsavedChangesChanged, this,
        &SketchbookViewModel::onUnsavedChangesChanged);
    connect(&m_canvas, &canvas::CanvasStore::edited, this, &SketchbookViewModel::saveOpenSketch);
    connect(&m_canvas, &canvas::CanvasStore::historyMoved, this, [this](bool redo) {
        m_notifications.postMessage(redo ? tr("Redone") : tr("Undone"), this);
    });
    m_storage.list();
}

SketchListModel *SketchbookViewModel::model()
{
    return &m_model;
}

QString SketchbookViewModel::openName() const
{
    const int row = m_model.rowOf(m_openId);
    return row < 0 ? QString() : m_model.entry(row).name;
}

bool SketchbookViewModel::hasUnsavedChanges() const
{
    return m_storage.hasUnsavedChanges();
}

void SketchbookViewModel::create()
{
    ++m_openRequestId;
    SketchEntry entry{
        .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
        .name = tr("Sketch %1").arg(m_model.rowCount() + 1),
        .modified = QDateTime::currentDateTime(),
    };
    const auto id = entry.id;
    m_storage.save(id, { .name = entry.name.toStdString(), .strokes = {}, .page = {} });
    qCDebug(lcSketchbook) << "Created sketch" << id;
    m_model.prepend(std::move(entry));
    activate(id, {});
}

void SketchbookViewModel::open(int row)
{
    if (row < 0 || row >= m_model.rowCount())
        return;
    const auto id = m_model.entry(row).id;
    ++m_openRequestId;
    m_storage.load(id, m_openRequestId);
}

void SketchbookViewModel::rename(int row, const QString &name)
{
    const auto trimmed = name.trimmed();
    if (row < 0 || row >= m_model.rowCount() || trimmed.isEmpty())
        return;
    auto entry = m_model.entry(row);
    const auto id = entry.id;
    if (!m_pendingRenames.contains(id))
        m_pendingRenames.insert(id, entry.name);
    entry.name = trimmed;
    entry.modified = QDateTime::currentDateTime();
    m_model.update(row, std::move(entry));
    if (id == m_openId)
        Q_EMIT openNameChanged();
    m_storage.rename(id, trimmed);
}

void SketchbookViewModel::renameOpen(const QString &name)
{
    rename(m_model.rowOf(m_openId), name);
}

void SketchbookViewModel::duplicate(int row)
{
    if (row < 0 || row >= m_model.rowCount())
        return;
    const auto &entry = m_model.entry(row);
    const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const auto name = copyName(entry.name);
    SketchEntry copy{
        .id = id,
        .name = name,
        .modified = QDateTime::currentDateTime(),
        .strokeCount = entry.strokeCount,
    };
    m_storage.duplicate(entry.id, id, name);
    qCDebug(lcSketchbook) << "Duplicating sketch" << entry.id << "as" << id;
    m_model.prepend(std::move(copy));
}

void SketchbookViewModel::remove(int row)
{
    if (row >= 0 && row < m_model.rowCount())
        m_storage.remove(m_model.entry(row).id);
}

bool SketchbookViewModel::requestClose()
{
    if (m_discardApproved) {
        m_discardApproved = false;
        return true;
    }
    if (!m_storage.hasUnsavedChanges())
        return true;
    m_closePending = true;
    if (!m_recoveryMessage.isEmpty())
        showSaveError();
    return false;
}

void SketchbookViewModel::onListed(std::vector<SketchEntry> entries)
{
    std::vector<SketchEntry> localEntries;
    for (int row = 0; row < m_model.rowCount(); ++row) {
        const auto &local = m_model.entry(row);
        const auto it = std::ranges::find(entries, local.id, &SketchEntry::id);
        if (it == entries.end())
            localEntries.push_back(local);
        else
            *it = local;
    }
    entries.insert(entries.begin(), localEntries.begin(), localEntries.end());
    m_model.setEntries(std::move(entries));
    m_initialized = true;
    Q_EMIT initializedChanged();
}

void SketchbookViewModel::onLoaded(const QString &id, quint64 requestId,
    const SketchDocument &document, const QString &error)
{
    if (requestId != m_openRequestId)
        return;
    const int row = m_model.rowOf(id);
    if (row < 0)
        return;
    if (!error.isEmpty()) {
        reportError(Operation::Open, m_model.entry(row).name,
            tr("Could not open sketch: %1").arg(error), false);
        return;
    }
    activate(id, document);
}

void SketchbookViewModel::onMutationFinished(const QString &id, const QString &sourceId,
    SketchStorage::Mutation mutation, const QString &error, bool retryable)
{
    if (mutation == SketchStorage::Mutation::Rename) {
        const auto pending = m_pendingRenames.find(id);
        if (pending != m_pendingRenames.end()) {
            if (error.isEmpty() || retryable) {
                if (error.isEmpty())
                    m_pendingRenames.erase(pending);
            } else {
                auto rollbackName = std::move(pending.value());
                m_pendingRenames.erase(pending);
                const int row = m_model.rowOf(id);
                if (row >= 0) {
                    auto entry = m_model.entry(row);
                    entry.name = std::move(rollbackName);
                    m_model.update(row, std::move(entry));
                    if (id == m_openId)
                        Q_EMIT openNameChanged();
                }
            }
        }
    } else if (mutation == SketchStorage::Mutation::Save && error.isEmpty()) {
        m_pendingRenames.remove(id);
    }
    if (error.isEmpty()) {
        if (mutation == SketchStorage::Mutation::Duplicate) {
            const int row = m_model.rowOf(id);
            if (row >= 0) {
                notify::Options options;
                options.message = tr("Duplicated %1").arg(m_model.entry(row).name);
                options.severity = notify::Message::Severity::Success;
                m_notifications.post(options, this);
            }
        }
        if (mutation == SketchStorage::Mutation::Save && !m_storage.hasUnsavedChanges())
            clearRecovery();
        return;
    }
    const int sourceRow = m_model.rowOf(sourceId);
    const auto source = sourceRow < 0 ? sourceId : m_model.entry(sourceRow).name;
    switch (mutation) {
    case SketchStorage::Mutation::Save:
        reportError(Operation::Save, source, tr("Could not save sketch: %1").arg(error), retryable);
        break;
    case SketchStorage::Mutation::Rename:
        reportError(Operation::Rename, source, tr("Could not rename sketch: %1").arg(error),
            retryable);
        break;
    case SketchStorage::Mutation::Duplicate:
        if (!retryable) {
            const int row = m_model.rowOf(id);
            if (row >= 0)
                m_model.removeAt(row);
        }
        reportError(Operation::Duplicate, source, tr("Could not duplicate sketch: %1").arg(error),
            retryable);
        break;
    }
}

void SketchbookViewModel::onRemoved(const QString &id, const QString &error)
{
    if (!error.isEmpty()) {
        const int row = m_model.rowOf(id);
        const auto source = row < 0 ? id : m_model.entry(row).name;
        reportError(Operation::Remove, source, tr("Could not delete sketch: %1").arg(error), false);
        return;
    }
    const int row = m_model.rowOf(id);
    if (row >= 0) {
        notify::Options options;
        options.message = tr("Deleted %1").arg(m_model.entry(row).name);
        options.severity = notify::Message::Severity::Success;
        m_notifications.post(options, this);
        m_model.removeAt(row);
    }
    if (id == m_openId) {
        m_openId.clear();
        m_canvas.load({});
        Q_EMIT openNameChanged();
    }
}

void SketchbookViewModel::saveOpenSketch()
{
    const int row = m_model.rowOf(m_openId);
    if (row < 0)
        return;
    auto entry = m_model.entry(row);
    SketchDocument document{
        .name = entry.name.toStdString(),
        .strokes = m_canvas.sketch().strokes,
        .page = m_canvas.sketch().page,
    };
    m_storage.save(entry.id, std::move(document));
    entry.modified = QDateTime::currentDateTime();
    entry.strokeCount = static_cast<qsizetype>(m_canvas.sketch().strokes.size());
    m_model.update(row, std::move(entry));
}

void SketchbookViewModel::activate(const QString &id, SketchDocument document)
{
    m_openId = id;
    m_canvas.load({ .strokes = std::move(document.strokes), .page = document.page });
    qCDebug(lcSketchbook) << "Opened sketch" << m_openId;
    Q_EMIT openNameChanged();
    Q_EMIT opened();
}

void SketchbookViewModel::reportError(Operation operation, const QString &source,
    const QString &message, bool retryable)
{
    QString title;
    switch (operation) {
    case Operation::Open:
        title = tr("Open sketch");
        break;
    case Operation::Rename:
        title = tr("Rename sketch");
        break;
    case Operation::Duplicate:
        title = tr("Duplicate sketch");
        break;
    case Operation::Save:
        title = tr("Save sketch");
        break;
    case Operation::Remove:
        title = tr("Delete sketch");
        break;
    }
    qCWarning(lcSketchbook).noquote() << title << "failed for" << source << ':' << message;
    if (retryable) {
        m_recoveryMessage = tr("%1: %2").arg(source, message);
        showSaveError();
        return;
    }
    notify::Options options;
    options.title = std::move(title);
    options.message = tr("%1: %2").arg(source, message);
    options.severity = notify::Message::Severity::Error;
    m_notifications.post(options, this);
}

void SketchbookViewModel::showSaveError()
{
    QList<notify::Action> actions{
        { .id = {}, .text = tr("Retry"), .triggered = [this] { m_storage.retryFailedSaves(); } },
    };
    if (m_closePending) {
        actions.append({ .id = {}, .text = tr("Discard"), .triggered = [this] {
                            m_closePending = false;
                            m_discardApproved = true;
                            Q_EMIT closeReady();
                        } });
        actions.append(
            { .id = {}, .text = tr("Keep editing"), .triggered = [this] { keepEditing(); } });
    }
    notify::Options options;
    options.title = tr("Could not save sketch");
    options.message = m_recoveryMessage;
    options.severity = notify::Message::Severity::Error;
    options.presentation = m_closePending
        ? notify::Message::Presentation::Decision
        : notify::Message::Presentation::Persistent;
    options.actions = actions;
    options.finished = [this](notify::Message::FinishReason reason) {
        m_recovery = nullptr;
        if (reason == notify::Message::FinishReason::Dismissed)
            keepEditing();
    };
    if (m_recovery) {
        m_recovery->setMessage(options.message);
        m_recovery->setActions(options.actions);
        m_recovery->setPresentation(options.presentation);
        return;
    }
    m_recovery = m_notifications.post(options, this);
}

void SketchbookViewModel::keepEditing()
{
    m_closePending = false;
    if (!m_recoveryMessage.isEmpty())
        showSaveError();
}

void SketchbookViewModel::onUnsavedChangesChanged()
{
    Q_EMIT unsavedChangesChanged();
    if (m_storage.hasUnsavedChanges())
        return;
    clearRecovery();
}

void SketchbookViewModel::clearRecovery()
{
    m_recoveryMessage.clear();
    if (m_recovery)
        m_recovery->cancel();
    if (m_closePending) {
        m_closePending = false;
        Q_EMIT closeReady();
    }
}

QString SketchbookViewModel::copyName(const QString &name) const
{
    const auto stem = name + tr(" copy");
    QString candidate = stem;
    for (int number = 2;; ++number) {
        bool exists = false;
        for (int row = 0; row < m_model.rowCount(); ++row) {
            if (m_model.entry(row).name == candidate) {
                exists = true;
                break;
            }
        }
        if (!exists)
            break;
        candidate = stem + u" %1"_s.arg(number);
    }
    return candidate;
}

} // namespace app::sketchbook
