// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "AppController.hpp"

#include <BuildInfo.hpp>

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QLoggingCategory>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcApp, "app.controller")

namespace app {

AppController::AppController(QString logFilePath, QObject *parent)
    : QObject(parent), m_logFilePath(std::move(logFilePath))
{
    setLanguage(QLocale::system().language() == QLocale::German ? "de"_L1 : "en"_L1);
}

QString AppController::displayName() const
{
    return platform::buildInfo().displayName.toString();
}

QString AppController::version() const
{
    return platform::buildInfo().version.toString();
}

QString AppController::buildDetails() const
{
    const auto &info = platform::buildInfo();
    // u""_s, not _L1: the middle dot is two bytes in this UTF-8 file, but one character.
    return u"%1 · %2 · %3 · Qt %4"_s.arg(info.gitSha, info.buildType, info.compiler,
        QLatin1StringView(qVersion()));
}

QString AppController::language() const
{
    return m_language;
}

void AppController::setLanguage(const QString &language)
{
    if (language == m_language)
        return;
    // Load first, swap second: a missing translation leaves the current language in place.
    // English needs its file too, for the plural forms of the source strings ("%n task(s)").
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
    emit languageChanged();
}

void AppController::openLogDirectory() const
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_logFilePath).absolutePath()));
}

} // namespace app
