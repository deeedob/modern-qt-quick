// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <QtCore/QAbstractListModel>
#include <QtQmlIntegration/qqmlintegration.h>

namespace app::backend {

// The rows the view shows, already filtered and ranked: TasksViewModel owns the data.
class TaskListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by TasksViewModel.model")

public:
    // Qt's model API takes roles as int, so this stays an unscoped enum.
    enum Role { // NOLINT(cppcoreguidelines-use-enum-class)
        TitleRole = Qt::UserRole + 1,
        DoneRole,
    };
    Q_ENUM(Role)

    struct Row
    {
        QString title;
        bool done = false;
    };

    using QAbstractListModel::QAbstractListModel;

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void setRows(QList<Row> rows);
    void appendRow(Row row);
    void removeRowAt(int row);
    void setDone(int row, bool done);

private:
    QList<Row> m_rows;
};

} // namespace app::backend
