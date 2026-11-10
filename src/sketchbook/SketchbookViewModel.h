#pragma once

#include "SketchListModel.hpp"
#include "SketchStorage.h"

#include <CanvasStore.h>

#include <QtCore/QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace app::sketchbook {

// MVVM: the view binds to the model and calls these commands; canvas edits are saved from here.
class SketchbookViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created in main() and passed to Main.qml as a required property")
    Q_PROPERTY(app::sketchbook::SketchListModel *model READ model CONSTANT FINAL)
    Q_PROPERTY(QString openName READ openName NOTIFY openNameChanged FINAL)

public:
    SketchbookViewModel(SketchStorage &storage, canvas::CanvasStore &canvas,
        QObject *parent = nullptr);

    [[nodiscard]] SketchListModel *model();
    [[nodiscard]] QString openName() const;

    // Creates an empty sketch and opens it.
    Q_INVOKABLE void create();
    Q_INVOKABLE void open(int row);
    Q_INVOKABLE void rename(int row, const QString &name);
    Q_INVOKABLE void renameOpen(const QString &name);
    Q_INVOKABLE void duplicate(int row);
    Q_INVOKABLE void remove(int row);

Q_SIGNALS:
    void openNameChanged();

private:
    void saveOpenSketch();
    [[nodiscard]] QString copyName(const QString &name) const;

    SketchStorage &m_storage;
    canvas::CanvasStore &m_canvas;
    SketchListModel m_model{ this };
    QString m_openId;
};

} // namespace app::sketchbook
