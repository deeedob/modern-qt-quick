// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "Logging.hpp"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTemporaryDir>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcTest, "app.test")

class tst_Logging : public QObject
{
    Q_OBJECT

private slots:
    void messagesReachTheSessionLog()
    {
        QTemporaryDir dir;
        const auto logFile = app::platform::installLogging(dir.path(), 3);
        qCInfo(lcTest) << "hello log file";

        QFile file(logFile);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto contents = file.readAll();
        QVERIFY2(contents.contains("app.test: hello log file"), contents.constData());
    }

    void oldSessionsAreRemoved()
    {
        QTemporaryDir dir;
        for (const char *name :
            { "20260101-000000.log", "20260102-000000.log", "20260103-000000.log" }) {
            QFile file(QDir(dir.path()).filePath(QLatin1StringView(name)));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }

        app::platform::installLogging(dir.path(), 3);

        const auto logs = QDir(dir.path()).entryList({ "*.log"_L1 }, QDir::Files, QDir::Name);
        QCOMPARE(logs.size(), 3);
        QVERIFY(!logs.contains("20260101-000000.log"_L1));
    }
};

QTEST_GUILESS_MAIN(tst_Logging)
#include "tst_Logging.moc"
