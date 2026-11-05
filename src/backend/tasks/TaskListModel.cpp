// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "TaskListModel.hpp"

using namespace Qt::StringLiterals;

namespace app::backend {

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

QVariant TaskListModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid))
        return {};
    const auto &row = m_rows.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return row.title;
    case DoneRole:
        return row.done;
    default:
        return {};
    }
}

QHash<int, QByteArray> TaskListModel::roleNames() const
{
    return {
        { TitleRole, "title"_ba },
        { DoneRole,  "done"_ba  }
    };
}

void TaskListModel::setRows(QList<Row> rows)
{
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
}

// Inserting and removing single rows, instead of resetting, keeps the view's scroll position.
void TaskListModel::appendRow(Row row)
{
    const auto position = static_cast<int>(m_rows.size());
    beginInsertRows({}, position, position);
    m_rows.append(std::move(row));
    endInsertRows();
}

void TaskListModel::removeRowAt(int row)
{
    Q_ASSERT(row >= 0 && row < m_rows.size());
    beginRemoveRows({}, row, row);
    m_rows.removeAt(row);
    endRemoveRows();
}

void TaskListModel::setDone(int row, bool done)
{
    Q_ASSERT(row >= 0 && row < m_rows.size());
    m_rows[row].done = done;
    const auto changed = index(row);
    emit dataChanged(changed, changed, { DoneRole });
}

} // namespace app::backend
