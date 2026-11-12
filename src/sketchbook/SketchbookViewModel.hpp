#pragma once

#include "SketchListModel.hpp"
#include "SketchStorage.hpp"

#include <CanvasStore.hpp>
#include <NotificationCenter.hpp>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace app::sketchbook {

class SketchbookViewModel : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SketchbookViewModel)
    QML_UNCREATABLE("Created in main() and passed to Main.qml as a required property")
    Q_PROPERTY(app::sketchbook::SketchListModel *model READ model CONSTANT FINAL)
    Q_PROPERTY(QString openName READ openName NOTIFY openNameChanged FINAL)
    Q_PROPERTY(bool hasUnsavedChanges READ hasUnsavedChanges NOTIFY unsavedChangesChanged FINAL)
    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged FINAL)

public:
    SketchbookViewModel(SketchStorage &storage, canvas::CanvasStore &canvas,
        notify::NotificationCenter &notifications, QObject *parent = nullptr);

    [[nodiscard]] SketchListModel *model();
    [[nodiscard]] QString openName() const;
    [[nodiscard]] bool hasUnsavedChanges() const;
    [[nodiscard]] bool initialized() const { return m_initialized; }

    Q_INVOKABLE void create();
    Q_INVOKABLE void open(int row);
    Q_INVOKABLE void rename(int row, const QString &name);
    Q_INVOKABLE void renameOpen(const QString &name);
    Q_INVOKABLE void duplicate(int row);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void retryFailedSaves();
    Q_INVOKABLE bool requestClose();

Q_SIGNALS:
    void openNameChanged();
    void unsavedChangesChanged();
    void initializedChanged();
    void opened();
    void closeReady();

private Q_SLOTS:
    void onListed(std::vector<app::sketchbook::SketchEntry> entries);
    void onLoaded(const QString &id, quint64 requestId,
        const app::sketchbook::SketchDocument &document, const QString &error);
    void onMutationFinished(const QString &id, const QString &sourceId,
        app::sketchbook::SketchStorage::Mutation mutation, const QString &error, bool retryable);
    void onRemoved(const QString &id, const QString &error);
    void onUnsavedChangesChanged();

private:
    enum class Operation { Open, Rename, Duplicate, Save, Remove };

    struct PendingRename
    {
        QString originalName;
        QString currentName;
    };

    void save(const QString &id, SketchDocument document);
    void saveOpenSketch();
    void activate(const QString &id, SketchDocument document);
    void reportError(Operation operation, const QString &source, const QString &message,
        bool retryable);
    void clearRecovery();
    void showSaveError();
    void keepEditing();
    [[nodiscard]] QString copyName(const QString &name) const;

    SketchStorage &m_storage;
    canvas::CanvasStore &m_canvas;
    notify::NotificationCenter &m_notifications;
    QPointer<notify::Message> m_recovery;
    SketchListModel m_model{ this };
    QString m_openId;
    QHash<QString, PendingRename> m_pendingRenames;
    quint64 m_nextRequestId = 0;
    quint64 m_openRequestId = 0;
    bool m_initialized = false;
    bool m_closePending = false;
    bool m_discardApproved = false;
    QString m_recoveryMessage;
};

} // namespace app::sketchbook
