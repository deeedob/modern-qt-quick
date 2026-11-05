// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include "TaskListModel.hpp"

#include <app/core/Task.hpp>
#include <app/core/TaskRepository.hpp>

#include <QtCore/QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace app::backend {

// MVVM: the view binds to properties and calls commands; it knows nothing about storage.
class TasksViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created in main() and passed to Main.qml as a required property")
    // Fully qualified: moc records the type as written, and QML tooling looks up the full C++ name.
    Q_PROPERTY(app::backend::TaskListModel *model READ model CONSTANT FINAL)
    Q_PROPERTY(
        QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged FINAL)
    Q_PROPERTY(int openCount READ openCount NOTIFY openCountChanged FINAL)

public:
    explicit TasksViewModel(core::TaskRepository &repository, QObject *parent = nullptr);

    [[nodiscard]] TaskListModel *model();
    [[nodiscard]] const TaskListModel *model() const;
    [[nodiscard]] QString searchText() const;
    void setSearchText(const QString &text);
    [[nodiscard]] int openCount() const;

    Q_INVOKABLE void add(const QString &title);
    Q_INVOKABLE void toggle(int row);
    Q_INVOKABLE void remove(int row);

signals:
    void searchTextChanged();
    void openCountChanged();

private:
    void refreshRows();
    void commit(int previousOpenCount);

    core::TaskRepository &m_repository;
    std::vector<core::Task> m_tasks;
    std::vector<std::size_t> m_visible; // model row -> index into m_tasks
    TaskListModel m_model{ this };
    QString m_searchText;
};

} // namespace app::backend
