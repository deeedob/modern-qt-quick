#include <QtCore/QDir>
#include <QtGui/QFontDatabase>
#include <QtGui/QGuiApplication>
#include <QtQuickTest/quicktest.h>

class QuickTestSetup : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void applicationAvailable()
    {
        const QDir source(QString::fromUtf8(QUICK_TEST_SOURCE_DIR));
        const int fontId = QFontDatabase::addApplicationFont(
            source.filePath(QStringLiteral("../../src/controls/fonts/InterVariable.ttf")));
        if (fontId < 0)
            qFatal("Cannot load the test font");
        QGuiApplication::setFont(
            QFont(QFontDatabase::applicationFontFamilies(fontId).constFirst()));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(quicktest, QuickTestSetup)
#include "quicktest.moc"
