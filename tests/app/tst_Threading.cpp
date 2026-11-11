#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFuture>
#include <QtCore/QObject>
#include <QtCore/QTemporaryDir>
#include <QtCore/QThread>
#include <QtTest/QTest>

#include <functional>
#include <utility>

using namespace Qt::StringLiterals;

namespace {

QFuture<void> removeFile(QObject *context, const QString &path,
    std::function<void(QString)> completion)
{
    return QtConcurrent::run([path] {
        QFile file(path);
        if (file.remove())
            return QString{};
        return file.errorString();
    }).then(context, std::move(completion));
}

} // namespace

class tst_Threading : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init() { QTest::failOnWarning(); }

    void oneShotDeliversToContextThread()
    {
        QTemporaryDir directory;
        const auto path = directory.filePath(u"example.txt"_s);
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        bool delivered = false;
        auto result = removeFile(this, path, [this, &delivered](const QString &error) {
            QCOMPARE(QThread::currentThread(), thread());
            QVERIFY(error.isEmpty());
            delivered = true;
        });
        QTRY_VERIFY(result.isFinished());
        QVERIFY(delivered);
        QVERIFY(!QFileInfo::exists(path));
    }

    void oneShotReturnsFileErrors()
    {
        QTemporaryDir directory;
        QString failure;
        auto result = removeFile(this, directory.filePath(u"missing"_s), [&failure](QString error) {
            failure = std::move(error);
        });
        QTRY_VERIFY(result.isFinished());
        QVERIFY(!failure.isEmpty());
    }
};

QTEST_GUILESS_MAIN(tst_Threading)
#include "tst_Threading.moc"
