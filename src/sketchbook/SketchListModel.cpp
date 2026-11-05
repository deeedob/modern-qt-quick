#include "SketchListModel.hpp"

using namespace Qt::StringLiterals;

namespace app::sketchbook {

int SketchListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

QVariant SketchListModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid))
        return {};
    const auto &e = entry(index.row());
    switch (role) {
    case NameRole:
        return e.name;
    case ModifiedRole:
        return e.modified;
    case StrokeCountRole:
        return e.strokeCount;
    default:
        return {};
    }
}

QHash<int, QByteArray> SketchListModel::roleNames() const
{
    return {
        { NameRole,        "name"_ba        },
        { ModifiedRole,    "modified"_ba    },
        { StrokeCountRole, "strokeCount"_ba },
    };
}

const SketchEntry &SketchListModel::entry(int row) const
{
    return m_entries.at(static_cast<std::size_t>(row));
}

int SketchListModel::rowOf(const QString &id) const
{
    const auto it = std::ranges::find(m_entries, id, &SketchEntry::id);
    return it == m_entries.end() ? -1 : static_cast<int>(it - m_entries.begin());
}

void SketchListModel::setEntries(std::vector<SketchEntry> entries)
{
    beginResetModel();
    m_entries = std::move(entries);
    endResetModel();
}

// Single-row changes, unlike a model reset, keep the view's scroll position.
void SketchListModel::prepend(SketchEntry entry)
{
    beginInsertRows({}, 0, 0);
    m_entries.insert(m_entries.begin(), std::move(entry));
    endInsertRows();
}

void SketchListModel::update(int row, SketchEntry entry)
{
    m_entries.at(static_cast<std::size_t>(row)) = std::move(entry);
    const auto changed = index(row);
    Q_EMIT dataChanged(changed, changed);
}

void SketchListModel::removeAt(int row)
{
    Q_ASSERT(row >= 0 && row < rowCount());
    beginRemoveRows({}, row, row);
    m_entries.erase(m_entries.begin() + row);
    endRemoveRows();
}

} // namespace app::sketchbook
