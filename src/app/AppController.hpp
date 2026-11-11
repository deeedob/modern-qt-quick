#pragma once

#include <BuildInfo.hpp>

#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QTranslator)

namespace app {

struct RuntimeInfo
{
    Q_GADGET
    QML_VALUE_TYPE(runtimeInfo)
    Q_PROPERTY(QString operatingSystem READ operatingSystem CONSTANT FINAL)
    Q_PROPERTY(QString kernelVersion READ kernelVersion CONSTANT FINAL)
    Q_PROPERTY(QString processArchitecture READ processArchitecture CONSTANT FINAL)
    Q_PROPERTY(QString windowSystem READ windowSystem CONSTANT FINAL)

public:
    RuntimeInfo();

    [[nodiscard]] const QString &operatingSystem() const { return m_operatingSystem; }
    [[nodiscard]] const QString &kernelVersion() const { return m_kernelVersion; }
    [[nodiscard]] const QString &processArchitecture() const { return m_processArchitecture; }
    [[nodiscard]] const QString &windowSystem() const { return m_windowSystem; }

private:
    QString m_operatingSystem;
    QString m_kernelVersion;
    QString m_processArchitecture;
    QString m_windowSystem;
};

// App-wide state for every view. A QML singleton, but main() owns the instance (no C++ global).
class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    QML_UNCREATABLE("Created in main() and registered with setExternalSingletonInstance()")
    Q_PROPERTY(app::BuildInfo buildDetails READ buildDetails CONSTANT FINAL)
    Q_PROPERTY(app::RuntimeInfo runtimeInfo READ runtimeInfo CONSTANT FINAL)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged FINAL)
    Q_PROPERTY(bool darkTheme READ darkTheme WRITE setDarkTheme NOTIFY darkThemeChanged FINAL)

public:
    struct Directories
    {
        QDir logs;
        QDir licenses;
    };

    explicit AppController(Directories directories, QSettings &settings, QObject *parent = nullptr);
    ~AppController() override;

    [[nodiscard]] BuildInfo buildDetails() const;
    [[nodiscard]] RuntimeInfo runtimeInfo() const;
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
    void openDirectory(const QDir &directory) const;

    Directories m_directories;
    RuntimeInfo m_runtimeInfo;
    QString m_language;
    bool m_darkTheme = false;
    QSettings &m_settings;
    std::unique_ptr<QTranslator> m_translator;
};

} // namespace app
