#include "LogSession.hpp"

#include <BuildInfo.hpp>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTemporaryDir>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcTest, "app.test")

namespace {
void previousHandler(QtMsgType, const QMessageLogContext &, const QString &)
{
}
}

class tst_LogSession : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init() { QTest::failOnWarning(); }

    void restoresPreviousMessageHandler()
    {
        QTemporaryDir dir;
        const auto original = qInstallMessageHandler(previousHandler);
        {
            const app::LogSession logging(QDir(dir.path()));
        }
        const auto restored = qInstallMessageHandler(original);
        QVERIFY(restored == previousHandler);
    }

    void messagesReachTheSessionLog()
    {
        QTemporaryDir dir;
        const QDir logDirectory(dir.path());
        {
            const app::LogSession logging(logDirectory, 3);
            qCInfo(lcTest) << "hello log file";

            const auto logs = logDirectory.entryList({ "*.log"_L1 }, QDir::Files, QDir::Name);
            QCOMPARE(logs.size(), 1);
            QFile file(logDirectory.filePath(logs.constFirst()));
            QVERIFY(file.open(QIODevice::ReadOnly));
            const auto contents = file.readAll();
            QVERIFY2(contents.contains("app.test: hello log file"), contents.constData());
            file.close();
        }
        QVERIFY(dir.remove());
    }

    void buildInfoPrintsAsOneLine()
    {
        QString text;
        QDebug(&text) << app::buildInfo;

        QVERIFY2(text.contains(app::buildInfo.version()), qPrintable(text));
        QVERIFY2(text.contains(app::buildInfo.gitSha()), qPrintable(text));
        QVERIFY(!text.contains(u'\n'));
    }

    void oldSessionsAreRemoved()
    {
        QTemporaryDir dir;
        for (const char *name :
            { "20260101-000000.log", "20260102-000000.log", "20260103-000000.log" }) {
            QFile file(QDir(dir.path()).filePath(QLatin1StringView(name)));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }

        {
            const app::LogSession logging(QDir(dir.path()), 3);

            const auto logs = QDir(dir.path()).entryList({ "*.log"_L1 }, QDir::Files, QDir::Name);
            QCOMPARE(logs.size(), 3);
            QVERIFY(!logs.contains("20260101-000000.log"_L1));
        }
        QVERIFY(dir.remove());
    }
};

QTEST_GUILESS_MAIN(tst_LogSession)
#include "tst_LogSession.moc"
