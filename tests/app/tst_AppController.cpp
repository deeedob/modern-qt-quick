#include "AppController.hpp"
#include <BuildInfo.hpp>

#include <QtCore/QMetaProperty>

#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QDesktopServices>
#include <QtQml/QQmlComponent>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
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
        QVERIFY(!details.gitShaText().isEmpty());
        QVERIFY(!details.buildTypeText().isEmpty());
        QVERIFY(!details.timestampText().isEmpty());
        QVERIFY(!details.compilerText().isEmpty());
        QVERIFY(!details.systemText().isEmpty());
        QVERIFY(!details.qtVersionText().isEmpty());
    }

    void exposesGeneratedBuildInfo()
    {
        const auto &meta = app::BuildInfo::staticMetaObject;
        QVERIFY(meta.indexOfProperty("gitSha") >= 0);
        QVERIFY(meta.indexOfProperty("qtVersion") >= 0);
    }

    void qmlReadsGeneratedBuildInfo()
    {
        QTemporaryDir dir;
        QSettings settings(dir.filePath("settings.ini"_L1), QSettings::IniFormat);
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"No translation for language.*"_s));
        app::AppController controller(dir.filePath("session.log"_L1), dir.path(), settings);
        QQmlEngine engine;
        engine.rootContext()->setContextProperty("controller"_L1, &controller);
        QQmlComponent component(&engine);
        component.setData(
            "import QtQml; QtObject { property string sha: controller.buildDetails.gitSha }",
            QUrl());
        const std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        QCOMPARE(object->property("sha").toString(), QString::fromUtf8(app::buildInfo.gitSha));
    }

    void recordOpenedUrl(const QUrl &url) { m_openedUrl = url; }

private:
    QUrl m_openedUrl;
};

QTEST_GUILESS_MAIN(tst_AppController)
#include "tst_AppController.moc"
