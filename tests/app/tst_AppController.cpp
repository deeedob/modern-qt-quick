#include "AppController.hpp"

#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QDesktopServices>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

class tst_AppController : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init() { QTest::failOnWarning(); }

    void cleanup() { QDesktopServices::unsetUrlHandler("file"_L1); }

    void opensTheLicensesDirectory()
    {
        QTemporaryDir dir;
        QSettings settings(dir.filePath("settings.ini"_L1), QSettings::IniFormat);
        settings.setValue("theme/dark"_L1, false);
        // This test links the controller without the application's translation resource.
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"No translation for language.*"_s));
        app::AppController controller(dir.filePath("session.log"_L1), dir.path(), settings);
        QDesktopServices::setUrlHandler("file"_L1, this, "recordOpenedUrl");

        controller.openLicensesDirectory();

        QCOMPARE(m_openedUrl, QUrl::fromLocalFile(dir.path()));
    }

    void persistsTheThemeChoice()
    {
        QTemporaryDir dir;
        QSettings settings(dir.filePath("settings.ini"_L1), QSettings::IniFormat);
        settings.setValue("theme/dark"_L1, false);
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"No translation for language.*"_s));
        app::AppController controller(dir.filePath("session.log"_L1), dir.path(), settings);

        controller.setDarkTheme(true);

        QVERIFY(controller.darkTheme());
        QCOMPARE(settings.value("theme/dark"_L1).toBool(), true);
    }

    void exposesStructuredBuildDetails()
    {
        QTemporaryDir dir;
        QSettings settings(dir.filePath("settings.ini"_L1), QSettings::IniFormat);
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"No translation for language.*"_s));
        app::AppController controller(dir.filePath("session.log"_L1), dir.path(), settings);

        const auto details = controller.buildDetails();
        QVERIFY(!details.gitSha.isEmpty());
        QVERIFY(!details.buildType.isEmpty());
        QVERIFY(!details.timestamp.isEmpty());
        QVERIFY(!details.compiler.isEmpty());
        QVERIFY(!details.system.isEmpty());
        QVERIFY(!details.qtVersion.isEmpty());
    }

    void recordOpenedUrl(const QUrl &url) { m_openedUrl = url; }

private:
    QUrl m_openedUrl;
};

QTEST_GUILESS_MAIN(tst_AppController)
#include "tst_AppController.moc"
