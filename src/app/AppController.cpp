#include "AppController.hpp"

#include <BuildInfo.hpp>

#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSettings>
#include <QtCore/QTranslator>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>

#include <utility>

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

AppController::AppController(Directories directories, QSettings &settings, QObject *parent)
    : QObject(parent)
    , m_directories(std::move(directories))
    , m_darkTheme(settings.contains("theme/dark"_L1)
              ? settings.value("theme/dark"_L1).toBool()
              : systemUsesDarkTheme())
    , m_settings(settings)
{
    const auto systemLanguage = QLocale::system().language();
    setLanguage(systemLanguage == QLocale::German ? "de"_L1
            : systemLanguage == QLocale::Spanish
            ? "es"_L1
            : "en"_L1);
}

AppController::~AppController() = default;

BuildInfo AppController::buildDetails() const
{
    return buildInfo;
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
    qCDebug(lcApp) << "Theme set to" << (darkTheme ? "dark" : "light");
    Q_EMIT darkThemeChanged();
}

void AppController::openLogDirectory() const
{
    openDirectory(m_directories.logs);
}

void AppController::openLicensesDirectory() const
{
    openDirectory(m_directories.licenses);
}

void AppController::openDirectory(const QDir &directory) const
{
    if (!directory.exists()) {
        qCWarning(lcApp) << "Directory does not exist" << directory.absolutePath();
        return;
    }
    const auto path = directory.absolutePath();
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
        qCWarning(lcApp) << "Could not open directory" << path;
}

} // namespace app
