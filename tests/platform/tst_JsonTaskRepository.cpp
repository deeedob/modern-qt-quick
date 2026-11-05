// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "JsonTaskRepository.hpp"

#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

using app::core::Task;
using app::platform::JsonTaskRepository;

class tst_JsonTaskRepository : public QObject
{
    Q_OBJECT

private slots:
    void missingFileLoadsAsEmpty()
    {
        JsonTaskRepository repository(m_dir.filePath("missing.json"_L1));
        const auto tasks = repository.load();
        QVERIFY(tasks.has_value());
        QVERIFY(tasks->empty());
    }

    void savedTasksLoadBack()
    {
        const std::vector<Task> saved{
            { .title = "Write tests", .done = true },
            { .title = "Äpfel kaufen" }
        };
        JsonTaskRepository repository(m_dir.filePath("tasks.json"_L1));
        QVERIFY(repository.save(saved).has_value());

        const auto loaded = repository.load();
        QVERIFY(loaded.has_value());
        QCOMPARE(*loaded, saved);
    }

    void corruptFileReportsAnError()
    {
        const auto path = m_dir.filePath("corrupt.json"_L1);
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{ not json");
        file.close();

        const auto tasks = JsonTaskRepository(path).load();
        QVERIFY(!tasks.has_value());
        QVERIFY(!tasks.error().empty());
    }

    void nonArrayDocumentReportsAnError()
    {
        const auto path = m_dir.filePath("object.json"_L1);
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(R"({ "title": "not a list" })");
        file.close();

        QVERIFY(!JsonTaskRepository(path).load().has_value());
    }

private:
    QTemporaryDir m_dir;
};

QTEST_GUILESS_MAIN(tst_JsonTaskRepository)
#include "tst_JsonTaskRepository.moc"
