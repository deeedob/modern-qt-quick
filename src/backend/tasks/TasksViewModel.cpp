// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "TasksViewModel.hpp"

#include <app/core/TaskSearch.hpp>

#include <QtCore/QLoggingCategory>

#include <algorithm>
#include <utility>

Q_STATIC_LOGGING_CATEGORY(lcTasks, "app.tasks")

namespace app::backend {

TasksViewModel::TasksViewModel(core::TaskRepository &repository, QObject *parent)
    : QObject(parent), m_repository(repository)
{
    if (auto tasks = m_repository.load())
        m_tasks = std::move(*tasks);
    else
        qCWarning(lcTasks) << "Could not load tasks:" << tasks.error();
    refreshRows();
}

TaskListModel *TasksViewModel::model()
{
    return &m_model;
}

const TaskListModel *TasksViewModel::model() const
{
    return &m_model;
}

QString TasksViewModel::searchText() const
{
    return m_searchText;
}

void TasksViewModel::setSearchText(const QString &text)
{
    if (text == m_searchText)
        return;
    m_searchText = text;
    emit searchTextChanged();
    refreshRows();
}

int TasksViewModel::openCount() const
{
    return static_cast<int>(std::ranges::count(m_tasks, false, &core::Task::done));
}

void TasksViewModel::add(const QString &title)
{
    const auto trimmed = title.trimmed();
    if (trimmed.isEmpty())
        return;
    const auto previousOpenCount = openCount();
    m_tasks.push_back({ .title = trimmed.toStdString() });
    // Without a search, a new task is simply the last row; otherwise the ranking decides.
    if (m_searchText.isEmpty()) {
        m_visible.push_back(m_tasks.size() - 1);
        m_model.appendRow({ .title = trimmed });
    } else {
        refreshRows();
    }
    commit(previousOpenCount);
}

void TasksViewModel::toggle(int row)
{
    if (row < 0 || std::cmp_greater_equal(row, m_visible.size()))
        return;
    const auto previousOpenCount = openCount();
    auto &task = m_tasks.at(m_visible.at(static_cast<std::size_t>(row)));
    task.done = !task.done;
    m_model.setDone(row, task.done);
    commit(previousOpenCount);
}

void TasksViewModel::remove(int row)
{
    if (row < 0 || std::cmp_greater_equal(row, m_visible.size()))
        return;
    const auto previousOpenCount = openCount();
    const auto visibleRow = m_visible.begin() + row;
    const auto taskIndex = *visibleRow;
    m_tasks.erase(m_tasks.begin() + static_cast<std::ptrdiff_t>(taskIndex));
    // Removing a task keeps the ranking of the others, so only the indices behind it shift.
    m_visible.erase(visibleRow);
    for (auto &index : m_visible)
        if (index > taskIndex)
            --index;
    m_model.removeRowAt(row);
    commit(previousOpenCount);
}

void TasksViewModel::refreshRows()
{
    m_visible.clear();
    QList<TaskListModel::Row> rows;
    for (const auto &hit : core::rankTasks(m_tasks, m_searchText.toStdString())) {
        const auto &task = m_tasks.at(hit.index);
        m_visible.push_back(hit.index);
        rows.append({ .title = QString::fromStdString(task.title), .done = task.done });
    }
    m_model.setRows(std::move(rows));
}

void TasksViewModel::commit(int previousOpenCount)
{
    if (openCount() != previousOpenCount)
        emit openCountChanged();
    if (const auto saved = m_repository.save(m_tasks); !saved)
        qCWarning(lcTasks) << "Could not save tasks:" << saved.error();
}

} // namespace app::backend
