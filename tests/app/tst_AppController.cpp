#include "AppController.hpp"
#include <BuildInfo.hpp>

#include <QtCore/QMetaProperty>

#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QDesktopServices>
#include <QtQml/QQmlComponent>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

namespace {

app::AppController::Directories directories(const QTemporaryDir &dir)
{
    return {
        .logs = QDir(dir.filePath("logs"_L1)),
        .licenses = QDir(dir.path()),
    };
}

} // namespace

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
        app::AppController controller(directories(dir), settings);
        QDesktopServices::setUrlHandler("file"_L1, this, "recordOpenedUrl");

        controller.openLicensesDirectory();

        QCOMPARE(m_openedUrl, QUrl::fromLocalFile(dir.path()));
    }

    void opensTheLogsDirectory()
    {
        QTemporaryDir dir;
        const auto appDirectories = directories(dir);
        QVERIFY(appDirectories.logs.mkpath("."_L1));
        QSettings settings(dir.filePath("settings.ini"_L1), QSettings::IniFormat);
        settings.setValue("theme/dark"_L1, false);
        app::AppController controller(appDirectories, settings);
        QDesktopServices::setUrlHandler("file"_L1, this, "recordOpenedUrl");

        controller.openLogDirectory();

        QCOMPARE(m_openedUrl, QUrl::fromLocalFile(appDirectories.logs.absolutePath()));
    }

    void persistsTheThemeChoice()
    {
        QTemporaryDir dir;
        QSettings settings(dir.filePath("settings.ini"_L1), QSettings::IniFormat);
        settings.setValue("theme/dark"_L1, false);
        app::AppController controller(directories(dir), settings);

        controller.setDarkTheme(true);

        QVERIFY(controller.darkTheme());
        QCOMPARE(settings.value("theme/dark"_L1).toBool(), true);
    }

    void exposesStructuredBuildDetails()
    {
        QTemporaryDir dir;
        QSettings settings(dir.filePath("settings.ini"_L1), QSettings::IniFormat);
        app::AppController controller(directories(dir), settings);

        const auto details = controller.buildDetails();
        QVERIFY(!details.gitSha().isEmpty());
        QVERIFY(!details.buildType().isEmpty());
        QVERIFY(!details.buildTimestamp().isEmpty());
        QVERIFY(!details.compiler().isEmpty());
        QVERIFY(!details.system().isEmpty());
        QVERIFY(!details.qtVersion().isEmpty());
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
        app::AppController controller(directories(dir), settings);
        QQmlEngine engine;
        engine.rootContext()->setContextProperty("controller"_L1, &controller);
        QQmlComponent component(&engine);
        component.setData(
            "import QtQml; QtObject { property string sha: controller.buildDetails.gitSha }",
            QUrl());
        const std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        QCOMPARE(object->property("sha").toString(), app::buildInfo.gitSha());
    }

    void recordOpenedUrl(const QUrl &url) { m_openedUrl = url; }

private:
    QUrl m_openedUrl;
};

QTEST_GUILESS_MAIN(tst_AppController)
#include "tst_AppController.moc"
