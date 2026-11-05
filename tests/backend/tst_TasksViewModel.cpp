// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "InMemoryTaskRepository.hpp"
#include "TaskListModel.hpp"
#include "TasksViewModel.hpp"

#include <QtTest/QAbstractItemModelTester>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;
using app::backend::TaskListModel;
using app::backend::TasksViewModel;
using app::core::Task;

class tst_TasksViewModel : public QObject
{
    Q_OBJECT

private:
    static QString title(const TasksViewModel &viewModel, int row)
    {
        return viewModel.model()->index(row).data(TaskListModel::TitleRole).toString();
    }

private slots:
    void loadsTasksFromTheRepository()
    {
        InMemoryTaskRepository repository({
            { .title = "One" },
            { .title = "Two", .done = true }
        });
        TasksViewModel viewModel(repository);
        QAbstractItemModelTester tester(viewModel.model(),
            QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(viewModel.model()->rowCount(), 2);
        QCOMPARE(viewModel.openCount(), 1);
        QCOMPARE(title(viewModel, 1), "Two"_L1);
    }

    void addSavesAndTrimsTitles()
    {
        InMemoryTaskRepository repository;
        TasksViewModel viewModel(repository);
        QSignalSpy openCountChanged(&viewModel, &TasksViewModel::openCountChanged);

        viewModel.add("  Write docs "_L1);
        viewModel.add("   "_L1);

        QCOMPARE(repository.stored, (std::vector<Task>{ { .title = "Write docs" } }));
        QCOMPARE(viewModel.model()->rowCount(), 1);
        QCOMPARE(openCountChanged.count(), 1);
    }

    void searchFiltersAndRanks()
    {
        InMemoryTaskRepository repository({ { .title = "Review slides" },
            { .title = "Buy groceries" } });
        TasksViewModel viewModel(repository);

        viewModel.setSearchText("grocries"_L1);
        QCOMPARE(viewModel.model()->rowCount(), 1);
        QCOMPARE(title(viewModel, 0), "Buy groceries"_L1);

        viewModel.setSearchText({});
        QCOMPARE(viewModel.model()->rowCount(), 2);
    }

    void addAndRemoveUpdateRowsInPlace()
    {
        InMemoryTaskRepository repository({ { .title = "One" }, { .title = "Two" } });
        TasksViewModel viewModel(repository);
        QAbstractItemModelTester tester(viewModel.model(),
            QAbstractItemModelTester::FailureReportingMode::QtTest);
        QSignalSpy reset(viewModel.model(), &QAbstractItemModel::modelReset);

        viewModel.add("Three"_L1);
        viewModel.remove(0);
        viewModel.toggle(0);

        QCOMPARE(reset.count(), 0);
        QCOMPARE(repository.stored,
            (std::vector<Task>{
                { .title = "Two", .done = true },
                { .title = "Three" }
        }));
        QCOMPARE(title(viewModel, 1), "Three"_L1);
    }

    void toggleAndRemoveAffectTheFilteredRow()
    {
        InMemoryTaskRepository repository({ { .title = "Review slides" },
            { .title = "Buy groceries" } });
        TasksViewModel viewModel(repository);
        QAbstractItemModelTester tester(viewModel.model(),
            QAbstractItemModelTester::FailureReportingMode::QtTest);
        viewModel.setSearchText("groceries"_L1);
        QSignalSpy dataChanged(viewModel.model(), &QAbstractItemModel::dataChanged);

        viewModel.toggle(0);
        QVERIFY(repository.stored.at(1).done);
        QCOMPARE(dataChanged.count(), 1);
        QCOMPARE(viewModel.openCount(), 1);

        viewModel.remove(0);
        QCOMPARE(repository.stored, (std::vector<Task>{ { .title = "Review slides" } }));
        QCOMPARE(viewModel.model()->rowCount(), 0);
    }
};

QTEST_GUILESS_MAIN(tst_TasksViewModel)
#include "tst_TasksViewModel.moc"
