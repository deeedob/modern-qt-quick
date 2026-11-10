#include "AppController.hpp"

#include <BuildInfo.hpp>

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcApp, "app.controller")

namespace app {

namespace {

    bool systemUsesDarkTheme()
    {
        if (!qobject_cast<QGuiApplication *>(QCoreApplication::instance()))
            return false;
        return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    }

} // namespace

AppController::AppController(QString logFilePath, QString licensesDirectory, QSettings &settings,
    QObject *parent)
    : QObject(parent)
    , m_logFilePath(std::move(logFilePath))
    , m_licensesDirectory(std::move(licensesDirectory))
    , m_darkTheme(settings.contains("theme/dark"_L1)
              ? settings.value("theme/dark"_L1).toBool()
              : systemUsesDarkTheme())
    , m_settings(settings)
{
    setLanguage(QLocale::system().language() == QLocale::German ? "de"_L1 : "en"_L1);
}

QString AppController::buildDetails() const
{
    // u""_s, not _L1: the middle dot is two bytes in this UTF-8 file, but one character.
    return u"%1 · %2 · %3 · Qt %4"_s.arg(buildInfo.gitSha, buildInfo.buildType, buildInfo.compiler,
        QLatin1StringView(qVersion()));
}

QString AppController::language() const
{
    return m_language;
}

bool AppController::darkTheme() const
{
    return m_darkTheme;
}

void AppController::setLanguage(const QString &language)
{
    if (language == m_language)
        return;
    // Load first, swap second: a missing translation leaves the current language in place.
    // English needs its file too, for the plural forms of the source strings ("%n stroke(s)").
    auto translator = std::make_unique<QTranslator>();
    if (!translator->load(":/i18n/app_%1.qm"_L1.arg(language))) {
        qCWarning(lcApp) << "No translation for language" << language;
        return;
    }
    if (m_translator)
        QCoreApplication::removeTranslator(m_translator.get());
    m_translator = std::move(translator);
    QCoreApplication::installTranslator(m_translator.get());
    m_language = language;
    qCInfo(lcApp) << "Language set to" << language;
    Q_EMIT languageChanged();
}

void AppController::setDarkTheme(bool darkTheme)
{
    if (darkTheme == m_darkTheme)
        return;
    m_darkTheme = darkTheme;
    m_settings.setValue("theme/dark"_L1, darkTheme);
    Q_EMIT darkThemeChanged();
}

void AppController::openLogDirectory() const
{
    openDirectory(QFileInfo(m_logFilePath).absolutePath());
}

void AppController::openLicensesDirectory() const
{
    openDirectory(m_licensesDirectory);
}

void AppController::openDirectory(const QString &directory) const
{
    if (!QFileInfo(directory).isDir()) {
        qCWarning(lcApp) << "Directory does not exist" << directory;
        return;
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(directory)))
        qCWarning(lcApp) << "Could not open directory" << directory;
}

} // namespace app
