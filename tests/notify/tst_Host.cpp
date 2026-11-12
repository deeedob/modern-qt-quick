#include <NotificationCenter.hpp>

#include <QtCore/QDir>
#include <QtCore/QMetaObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtGui/QFontDatabase>
#include <QtQml/QQmlComponent>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

class tst_Host : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        const QDir source(QString::fromUtf8(QT_TESTCASE_SOURCEDIR));
        const int fontId = QFontDatabase::addApplicationFont(
            source.filePath(QStringLiteral("../../src/controls/fonts/InterVariable.ttf")));
        QVERIFY(fontId >= 0);
        QGuiApplication::setFont(
            QFont(QFontDatabase::applicationFontFamilies(fontId).constFirst()));
    }

    void init() { QTest::failOnWarning(); }

    void cardsRemainIndependentUntilDismissed()
    {
        app::notify::NotificationCenter center;
        app::notify::Options options;
        options.presentation = app::notify::Message::Presentation::Persistent;
        options.message = u"First message"_s;
        app::notify::Message *first = center.post(options, this);
        options.message = u"Second message"_s;
        app::notify::Message *second = center.post(options, this);

        QQmlEngine engine;
        QQuickWindow window;
        window.resize(640, 480);
        QScopedPointer<QObject> root(createHost(engine, center, window));
        QVERIFY(root);
        window.show();

        auto *cardList = root->findChild<QObject *>(u"cardList"_s);
        QVERIFY(cardList);
        QTRY_COMPARE(cardList->property("count").toInt(), 2);

        first->dismiss();
        QTRY_COMPARE(center.count(), 1);
        QCOMPARE(cardList->property("count").toInt(), 1);
        QVERIFY(!second->isFinished());
    }

    void overflowShowsWaitingRequests()
    {
        app::notify::NotificationCenter center;
        app::notify::Options options;
        options.presentation = app::notify::Message::Presentation::Persistent;
        for (int index = 0; index < 4; ++index) {
            options.message = u"Message %1"_s.arg(index);
            QVERIFY(center.post(options, this));
        }

        QQmlEngine engine;
        QQuickWindow window;
        window.resize(640, 480);
        QScopedPointer<QObject> root(createHost(engine, center, window));
        QVERIFY(root);
        window.show();

        auto *cardList = root->findChild<QObject *>(u"cardList"_s);
        QVERIFY(cardList);
        QTRY_COMPARE(cardList->property("count").toInt(), 4);
        QVERIFY(!root->findChild<QObject *>(u"overflowDialog"_s));
    }

    void scrollingUpdatesCardPresentation()
    {
        app::notify::NotificationCenter center;
        app::notify::Options options;
        options.presentation = app::notify::Message::Presentation::Persistent;
        for (int i = 0; i < 20; ++i)
            QVERIFY(center.post(options, this));
        QQmlEngine engine;
        QQuickWindow window;
        window.resize(640, 480);
        QScopedPointer<QObject> root(createHost(engine, center, window));
        QVERIFY(root);
        window.show();
        auto *list = root->findChild<QObject *>(u"cardList"_s);
        QVERIFY(list);
        QVERIFY(list->setProperty("cacheBuffer", 2000));
        QQuickItem *delegate = nullptr;
        QTRY_VERIFY(QMetaObject::invokeMethod(list, "itemAtIndex",
                        Q_RETURN_ARG(QQuickItem *, delegate), Q_ARG(int, 0))
            && delegate);
        auto *card = delegate->findChild<QQuickItem *>(u"notificationCard"_s);
        QVERIFY(card);
        QTRY_VERIFY(card->property("presented").toBool());
        QVERIFY(list->setProperty("contentY", 250));
        QTRY_VERIFY(!card->property("presented").toBool());
        QVERIFY(list->setProperty("contentY", 0));
        QTRY_VERIFY(card->property("presented").toBool());
    }

    void persistentCardsHaveEmphasizedBorder()
    {
        app::notify::NotificationCenter center;
        app::notify::Options options;
        options.presentation = app::notify::Message::Presentation::Persistent;
        QVERIFY(center.post(options, this));
        QQmlEngine engine;
        QQuickWindow window;
        window.resize(640, 480);
        QScopedPointer<QObject> root(createHost(engine, center, window));
        QVERIFY(root);
        window.show();
        auto *list = root->findChild<QObject *>(u"cardList"_s);
        QVERIFY(list);
        QQuickItem *delegate = nullptr;
        QTRY_VERIFY(QMetaObject::invokeMethod(list, "itemAtIndex",
                        Q_RETURN_ARG(QQuickItem *, delegate), Q_ARG(int, 0))
            && delegate);
        auto *surface = delegate->findChild<QObject *>(u"notificationSurface"_s);
        QVERIFY(surface);
        auto *border = qvariant_cast<QObject *>(surface->property("border"));
        QVERIFY(border);
        QCOMPARE(border->property("width").toInt(), 2);
    }

    void cardsWrapAndActionsRemainVisibleAtNarrowWidth()
    {
        app::notify::NotificationCenter center;
        app::notify::Options options;
        options.presentation = app::notify::Message::Presentation::Persistent;
        options.title = u"A long notification title that needs two lines"_s;
        options.message = QStringLiteral("This notification message wraps across the card. ")
                              .repeated(5);
        options.actions = {
            { u"retry"_s, u"Try again from here"_s, true, {} },
            { u"keep"_s,  u"Save this draft now"_s, true, {} },
        };
        QVERIFY(center.post(options, this));

        QQmlEngine engine;
        QQuickWindow window;
        window.resize(360, 360);
        QScopedPointer<QObject> root(createHost(engine, center, window));
        QVERIFY(root);
        auto *rootItem = qobject_cast<QQuickItem *>(root.data());
        QVERIFY(rootItem);
        window.show();
        QTRY_COMPARE(rootItem->width(), 360);
        QTRY_COMPARE(rootItem->height(), 360);

        auto *cardList = root->findChild<QObject *>(u"cardList"_s);
        QVERIFY(cardList);
        QTRY_COMPARE(cardList->property("count").toInt(), 1);
        QQuickItem *delegate = nullptr;
        QTRY_VERIFY(cardList->property("height").toReal() > 0);
        QTRY_VERIFY(QMetaObject::invokeMethod(cardList, "itemAtIndex",
                        Q_RETURN_ARG(QQuickItem *, delegate), Q_ARG(int, 0))
            && delegate);
        auto *card = delegate->findChild<QQuickItem *>(u"notificationCard"_s);
        auto *message = card ? card->findChild<QObject *>(u"notificationMessage"_s) : nullptr;
        auto *actions = card ? card->findChild<QQuickItem *>(u"notificationActions"_s) : nullptr;
        auto *repeater = card
            ? card->findChild<QObject *>(u"notificationActionRepeater"_s)
            : nullptr;
        QQuickItem *button = nullptr;
        QVERIFY(card);
        QVERIFY(message);
        QVERIFY(actions);
        QVERIFY(repeater);
        QVERIFY(QMetaObject::invokeMethod(repeater, "itemAt", Q_RETURN_ARG(QQuickItem *, button),
            Q_ARG(int, 0)));
        QVERIFY(button);

        const qreal narrowMessageHeight = message->property("implicitHeight").toReal();
        const qreal narrowActionsHeight = actions->height();
        QVERIFY(card->width() <= 336);
        QVERIFY(narrowMessageHeight > 28);
        QVERIFY(narrowActionsHeight > 36);
        QVERIFY(button->isVisible());
        QVERIFY(button->width() <= card->width());

        window.resize(640, 480);
        QTRY_VERIFY(cardList->property("width").toReal() >= 320);
        QTRY_VERIFY(message->property("implicitHeight").toReal() <= narrowMessageHeight);
        QTRY_VERIFY(actions->width() <= card->width());
        QTRY_VERIFY(button->isVisible());
    }

    void decisionsSerializeEscapeAndRestoreFocus()
    {
        app::notify::NotificationCenter center;
        app::notify::Options options;
        options.presentation = app::notify::Message::Presentation::Decision;
        options.actions = {
            { u"accept"_s, u"Accept"_s, true, {} }
        };
        options.message = u"First decision"_s;

        QQmlEngine engine;
        QQuickWindow window;
        window.resize(360, 360);
        QScopedPointer<QObject> root(createHost(engine, center, window));
        QVERIFY(root);
        auto *rootItem = qobject_cast<QQuickItem *>(root.data());
        QVERIFY(rootItem);
        window.show();
        rootItem->forceActiveFocus();
        QTRY_COMPARE(window.activeFocusItem(), rootItem);
        QObject *originalFocus = window.activeFocusItem();

        app::notify::Message *first = center.post(options, this);
        options.message = u"Second decision"_s;
        app::notify::Message *second = center.post(options, this);

        auto *dialog = root->findChild<QObject *>(u"decisionDialog"_s);
        QVERIFY(dialog);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(dialog->property("width").toReal() <= 336);
        QVERIFY(dialog->property("height").toReal() <= 336);
        QCOMPARE(center.currentDecision(), first);
        QTRY_VERIFY(window.activeFocusItem() != originalFocus);

        QTest::keyClick(&window, Qt::Key_Escape);
        QTRY_COMPARE(center.currentDecision(), second);
        QTRY_VERIFY(dialog->property("visible").toBool());

        QTest::keyClick(&window, Qt::Key_Escape);
        QTRY_VERIFY(!center.currentDecision());
        QTRY_COMPARE(window.activeFocusItem(), originalFocus);
    }

private:
    QObject *createHost(QQmlEngine &engine, app::notify::NotificationCenter &center,
        QQuickWindow &window)
    {
        QQmlComponent component(&engine);
        component.setData(R"(
            import QtQuick
            import App.Notify
            import App.Controls

            Item {
                required property NotificationCenter center
                width: parent ? parent.width : 0
                height: parent ? parent.height : 0
                NotificationHost {
                    anchors.fill: parent
                    center: parent.center
                }
            }
        )",
            QUrl());
        QObject *root = component.createWithInitialProperties({
            { u"center"_s, QVariant::fromValue(&center) }
        });
        if (auto *rootItem = qobject_cast<QQuickItem *>(root))
            rootItem->setParentItem(window.contentItem());
        return root;
    }
};

QTEST_MAIN(tst_Host)
#include "tst_Host.moc"
