#pragma once

#include <QtCore/QObject>
#include <QtCore/QTranslator>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

class QSettings;

namespace app {

struct BuildDetails
{
    Q_GADGET
    QML_VALUE_TYPE(buildDetails)
    Q_PROPERTY(QString gitSha MEMBER gitSha CONSTANT FINAL)
    Q_PROPERTY(QString buildType MEMBER buildType CONSTANT FINAL)
    Q_PROPERTY(QString timestamp MEMBER timestamp CONSTANT FINAL)
    Q_PROPERTY(QString compiler MEMBER compiler CONSTANT FINAL)
    Q_PROPERTY(QString system MEMBER system CONSTANT FINAL)
    Q_PROPERTY(QString qtVersion MEMBER qtVersion CONSTANT FINAL)

public:
    QString gitSha;
    QString buildType;
    QString timestamp;
    QString compiler;
    QString system;
    QString qtVersion;
};

// App-wide state for every view. A QML singleton, but main() owns the instance (no C++ global).
class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    QML_UNCREATABLE("Created in main() and registered with setExternalSingletonInstance()")
    Q_PROPERTY(app::BuildDetails buildDetails READ buildDetails CONSTANT FINAL)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged FINAL)
    Q_PROPERTY(bool darkTheme READ darkTheme WRITE setDarkTheme NOTIFY darkThemeChanged FINAL)

public:
    explicit AppController(QString logFilePath, QString licensesDirectory, QSettings &settings,
        QObject *parent = nullptr);

    [[nodiscard]] BuildDetails buildDetails() const;
    [[nodiscard]] QString language() const;
    [[nodiscard]] bool darkTheme() const;
    // Loads the embedded translation for an ISO 639 language code, e.g. "en" or "de".
    void setLanguage(const QString &language);
    void setDarkTheme(bool darkTheme);

    Q_INVOKABLE void openLogDirectory() const;
    Q_INVOKABLE void openLicensesDirectory() const;

Q_SIGNALS:
    void languageChanged();
    void darkThemeChanged();

private:
    void openDirectory(const QString &directory) const;

    QString m_logFilePath;
    QString m_licensesDirectory;
    QString m_language;
    bool m_darkTheme = false;
    QSettings &m_settings;
    std::unique_ptr<QTranslator> m_translator;
};

} // namespace app
