// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <QtCore/QObject>
#include <QtCore/QTranslator>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

namespace app {

// App-wide state for every view. A QML singleton, but main() owns the instance (no C++ global).
class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    QML_UNCREATABLE("Created in main() and registered with setExternalSingletonInstance()")
    Q_PROPERTY(QString displayName READ displayName CONSTANT FINAL)
    Q_PROPERTY(QString version READ version CONSTANT FINAL)
    Q_PROPERTY(QString buildDetails READ buildDetails CONSTANT FINAL)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged FINAL)

public:
    explicit AppController(QString logFilePath, QObject *parent = nullptr);

    [[nodiscard]] QString displayName() const;
    [[nodiscard]] QString version() const;
    [[nodiscard]] QString buildDetails() const;
    [[nodiscard]] QString language() const;
    // Loads the embedded translation for an ISO 639 language code, e.g. "en" or "de".
    void setLanguage(const QString &language);

    Q_INVOKABLE void openLogDirectory() const;

signals:
    void languageChanged();

private:
    QString m_logFilePath;
    QString m_language;
    std::unique_ptr<QTranslator> m_translator;
};

} // namespace app
