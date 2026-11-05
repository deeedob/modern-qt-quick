#pragma once

#include "SketchStorage.hpp"

#include <QtCore/QAbstractListModel>
#include <QtQmlIntegration/qqmlintegration.h>

namespace app::sketchbook {

// The rows the sketchbook shows. SketchbookViewModel decides what they contain.
class SketchListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by SketchbookViewModel.model")

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        ModifiedRole,
        StrokeCountRole,
    };

    using QAbstractListModel::QAbstractListModel;

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] const SketchEntry &entry(int row) const;
    [[nodiscard]] int rowOf(const QString &id) const;
    void setEntries(std::vector<SketchEntry> entries);
    void prepend(SketchEntry entry);
    void update(int row, SketchEntry entry);
    void removeAt(int row);

private:
    std::vector<SketchEntry> m_entries;
};

} // namespace app::sketchbook
