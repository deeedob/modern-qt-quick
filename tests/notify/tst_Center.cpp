#include <NotificationCenter.hpp>

#include <QtCore/QElapsedTimer>
#include <QtCore/QPointer>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtTest/QAbstractItemModelTester>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

namespace {

class DestructionContext : public QObject
{
public:
    DestructionContext() = default;
    Q_DISABLE_COPY_MOVE(DestructionContext)
    ~DestructionContext() override { destroying = true; }
    bool destroying = false;
};

} // namespace

class tst_Center : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void keepsOverlappingMessages();
    void dismissesOnlyPassiveMessages();
    void dismissPassiveHandlesDeletedMessages();
    void newestMessagesFollowPriority();
    void finishesOnceBeforeNotifying();
    void cancelsWhenContextIsDestroyed();
    void defersCancellationUntilContextDestructionCompletes();
    void cancelsAllRequestsOnShutdown();
    void passiveTimeoutStartsWhenPresented();
    void reportsIndependentTimerProgress();
    void prioritizesWaitingCardsWithoutMovingVisibleCards();
    void serializesDecisionRequests();
    void modalCompletionResumesCardTimeout();
    void presentedOverflowCardsCanExpire();
    void ignoresUnknownAndDisabledActions();
    void resumesOverdueCardsWithoutSynchronousCompletion();
    void autoIdsActionsAndRunsCallbacksOnce();
    void suppressesActionCallbackWhenContextDiesDuringFinish();
    void callsCompletionWithLiveContext();
};

void tst_Center::init()
{
    QTest::failOnWarning();
}

void tst_Center::keepsOverlappingMessages()
{
    app::notify::NotificationCenter center;
    QObject context;

    auto *first = center.postMessage(u"first"_s, &context);
    auto *second = center.postMessage(u"second"_s, &context);

    QVERIFY(first);
    QVERIFY(second);
    QVERIFY(first->id() != second->id());
    QCOMPARE(center.rowCount(), 2);
    QCOMPARE(center.data(center.index(0), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        second);
    QCOMPARE(center.data(center.index(1), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        first);
}

void tst_Center::dismissesOnlyPassiveMessages()
{
    app::notify::NotificationCenter center;
    QObject context;
    app::notify::Options options;
    auto *passive = center.post(options, &context);
    options.presentation = app::notify::Message::Presentation::Persistent;
    auto *persistent = center.post(options, &context);
    options.presentation = app::notify::Message::Presentation::Decision;
    auto *decision = center.post(options, &context);
    center.dismissPassive();
    QVERIFY(passive->isFinished());
    QVERIFY(!persistent->isFinished());
    QVERIFY(!decision->isFinished());
    QCOMPARE(center.count(), 1);
}

void tst_Center::dismissPassiveHandlesDeletedMessages()
{
    app::notify::NotificationCenter center;
    QObject context;
    auto *later = center.postMessage(u"later"_s, &context);
    auto *first = center.postMessage(u"first"_s, &context);
    QPointer<app::notify::Message> guardedLater(later);
    connect(first, &app::notify::Message::finished, &center,
        [later](app::notify::Message::FinishReason, const QString &) {
            later->dismiss();
            delete later;
        });

    center.dismissPassive();

    QVERIFY(guardedLater.isNull());
    QCOMPARE(center.count(), 0);
}

void tst_Center::newestMessagesFollowPriority()
{
    app::notify::NotificationCenter center;
    QObject context;
    app::notify::Options options;
    auto *first = center.post(options, &context);
    auto *second = center.post(options, &context);
    options.severity = app::notify::Message::Severity::Error;
    auto *urgent = center.post(options, &context);
    QCOMPARE(center.data(center.index(0), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        urgent);
    QCOMPARE(center.data(center.index(1), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        second);
    QCOMPARE(center.data(center.index(2), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        first);
}

void tst_Center::finishesOnceBeforeNotifying()
{
    app::notify::NotificationCenter center;
    QObject context;
    auto *notification = center.postMessage(u"finish"_s, &context);
    QVERIFY(notification);
    QPointer<app::notify::Message> guardedNotification(notification);

    QSignalSpy finished(notification, &app::notify::Message::finished);
    connect(notification, &app::notify::Message::finished, &center,
        [&center](app::notify::Message::FinishReason, const QString &) {
            QCOMPARE(center.rowCount(), 0);
        });

    notification->dismiss();
    notification->dismiss();

    QTRY_COMPARE(finished.size(), 1);
    QVERIFY(notification->isFinished());
    QCOMPARE(center.rowCount(), 0);
    QTRY_VERIFY(guardedNotification.isNull());
}

void tst_Center::cancelsWhenContextIsDestroyed()
{
    app::notify::NotificationCenter center;
    auto *context = new QObject;
    auto *notification = center.postMessage(u"context"_s, context);
    QVERIFY(notification);
    QSignalSpy finished(notification, &app::notify::Message::finished);

    delete context;

    QTRY_COMPARE(finished.size(), 1);
    QCOMPARE(finished.first().at(0).value<app::notify::Message::FinishReason>(),
        app::notify::Message::FinishReason::Cancelled);
    QTRY_COMPARE(center.rowCount(), 0);
}

void tst_Center::defersCancellationUntilContextDestructionCompletes()
{
    app::notify::NotificationCenter center;
    auto *context = new DestructionContext;
    auto *notification = center.postMessage(u"context"_s, context);
    QVERIFY(notification);
    QSignalSpy finished(notification, &app::notify::Message::finished);
    bool calledDuringDestruction = false;
    connect(notification, &app::notify::Message::finished, context,
        [context, &calledDuringDestruction] { calledDuringDestruction = context->destroying; });

    delete context;

    QVERIFY(!calledDuringDestruction);
    QTRY_COMPARE(finished.size(), 1);
    QCOMPARE(center.count(), 0);
}

void tst_Center::cancelsAllRequestsOnShutdown()
{
    app::notify::NotificationCenter center;
    QObject context;
    auto *first = center.postMessage(u"first"_s, &context);
    auto *second = center.postMessage(u"second"_s, &context);
    QSignalSpy firstFinished(first, &app::notify::Message::finished);
    QSignalSpy secondFinished(second, &app::notify::Message::finished);

    center.shutdown();
    center.shutdown();

    QCOMPARE(firstFinished.size(), 1);
    QCOMPARE(secondFinished.size(), 1);
    QCOMPARE(center.rowCount(), 0);
    QVERIFY(center.postMessage(u"late"_s, &context) == nullptr);
}

void tst_Center::passiveTimeoutStartsWhenPresented()
{
    app::notify::NotificationCenter center;
    QObject context;
    app::notify::Options options;
    options.message = u"timed"_s;
    options.timeoutMs = 40;
    auto *notification = center.post(options, &context);
    QVERIFY(notification);
    QSignalSpy finished(notification, &app::notify::Message::finished);

    bool elapsed = false;
    QTimer::singleShot(70, &center, [&elapsed] { elapsed = true; });
    QTRY_VERIFY_WITH_TIMEOUT(elapsed, 1000);
    QCOMPARE(finished.size(), 0);

    notification->setPresentationState(true, false);
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 1000);
    QCOMPARE(finished.first().at(0).value<app::notify::Message::FinishReason>(),
        app::notify::Message::FinishReason::Expired);
}

void tst_Center::reportsIndependentTimerProgress()
{
    app::notify::NotificationCenter center;
    QObject context;
    app::notify::Options options;
    options.timeoutMs = 1000;
    auto *first = center.post(options, &context);
    auto *second = center.post(options, &context);
    first->setPresentationState(true, false);
    QTRY_VERIFY_WITH_TIMEOUT(first->timeRemaining() < 0.95, 500);
    QCOMPARE(second->timeRemaining(), 1.0);
    first->setPresentationState(true, true);
    const auto paused = first->timeRemaining();
    QTRY_VERIFY_WITH_TIMEOUT(first->timeRemaining() == paused, 500);
    second->setPresentationState(true, false);
    QTRY_VERIFY_WITH_TIMEOUT(second->timeRemaining() < 0.95, 500);
    QCOMPARE(first->timeRemaining(), paused);
}

void tst_Center::prioritizesWaitingCardsWithoutMovingVisibleCards()
{
    app::notify::NotificationCenter center;
    QAbstractItemModelTester modelTester(&center,
        QAbstractItemModelTester::FailureReportingMode::QtTest);
    QObject context;
    app::notify::Options options;
    options.presentation = app::notify::Message::Presentation::Persistent;
    auto *visible = center.post(options, &context);
    auto *waiting = center.post(options, &context);
    options.severity = app::notify::Message::Severity::Error;
    auto *urgent = center.post(options, &context);

    QCOMPARE(center.data(center.index(0), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        urgent);
    QCOMPARE(center.data(center.index(1), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        waiting);
    QCOMPARE(center.data(center.index(2), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        visible);
}

void tst_Center::serializesDecisionRequests()
{
    app::notify::NotificationCenter center;
    QAbstractItemModelTester modelTester(&center,
        QAbstractItemModelTester::FailureReportingMode::QtTest);
    QObject context;
    app::notify::Options options;
    options.presentation = app::notify::Message::Presentation::Decision;
    auto *first = center.post(options, &context);
    auto *second = center.post(options, &context);
    auto *card = center.postMessage(u"card"_s, &context);

    QCOMPARE(center.currentDecision(), first);
    QCOMPARE(center.rowCount(), 1);
    QSignalSpy decisionChanges(&center, &app::notify::NotificationCenter::currentDecisionChanged);

    first->dismiss();

    QCOMPARE(decisionChanges.size(), 1);
    QCOMPARE(center.currentDecision(), second);
    QCOMPARE(center.rowCount(), 1);
    QCOMPARE(center.data(center.index(0), app::notify::NotificationCenter::NotificationRole)
                 .value<app::notify::Message *>(),
        card);
}

void tst_Center::modalCompletionResumesCardTimeout()
{
    app::notify::NotificationCenter center;
    QObject context;
    app::notify::Options options;
    options.timeoutMs = 100;
    auto *card = center.post(options, &context);
    card->setPresentationState(true, false);
    QSignalSpy cardFinished(card, &app::notify::Message::finished);

    options.presentation = app::notify::Message::Presentation::Decision;
    auto *decision = center.post(options, &context);
    bool pausedIntervalElapsed = false;
    QTimer::singleShot(150, &center, [&pausedIntervalElapsed] { pausedIntervalElapsed = true; });
    QTRY_VERIFY_WITH_TIMEOUT(pausedIntervalElapsed, 1000);
    QCOMPARE(cardFinished.size(), 0);

    decision->dismiss();

    QTRY_COMPARE_WITH_TIMEOUT(cardFinished.size(), 1, 1000);
    QCOMPARE(cardFinished.first().at(0).value<app::notify::Message::FinishReason>(),
        app::notify::Message::FinishReason::Expired);
}

void tst_Center::presentedOverflowCardsCanExpire()
{
    app::notify::NotificationCenter center;
    QObject context;
    app::notify::Options options;
    options.timeoutMs = 30;
    auto *first = center.post(options, &context);
    auto *overflow = center.post(options, &context);
    overflow->setPresentationState(true, false);
    QSignalSpy overflowFinished(overflow, &app::notify::Message::finished);

    QTRY_COMPARE_WITH_TIMEOUT(overflowFinished.size(), 1, 1000);
    QVERIFY(first->isFinished() == false);
}

void tst_Center::ignoresUnknownAndDisabledActions()
{
    app::notify::NotificationCenter center;
    QObject context;
    app::notify::Options options;
    options.presentation = app::notify::Message::Presentation::Persistent;
    options.actions = {
        { u"disabled"_s, u"Disabled"_s, false, {} },
        { u"save"_s,     u"Save"_s,     true,  {} },
    };
    auto *notification = center.post(options, &context);
    QVERIFY(notification);
    QSignalSpy finished(notification, &app::notify::Message::finished);

    notification->choose(u"missing"_s);
    notification->choose(u"disabled"_s);
    QCOMPARE(finished.size(), 0);

    notification->choose(u"save"_s);

    QCOMPARE(finished.size(), 1);
    QCOMPARE(finished.first().at(0).value<app::notify::Message::FinishReason>(),
        app::notify::Message::FinishReason::Action);
    QCOMPARE(finished.first().at(1).toString(), u"save"_s);
}

void tst_Center::resumesOverdueCardsWithoutSynchronousCompletion()
{
    app::notify::NotificationCenter center;
    QObject context;
    app::notify::Options options;
    options.timeoutMs = 1;
    auto *first = center.post(options, &context);
    auto *second = center.post(options, &context);
    first->setPresentationState(true, false);
    second->setPresentationState(true, false);
    QElapsedTimer undeliveredTimeout;
    undeliveredTimeout.start();
    while (undeliveredTimeout.elapsed() < 5)
        QThread::yieldCurrentThread();
    first->setPresentationState(true, true);
    second->setPresentationState(true, true);
    center.setHostActive(false);
    first->setPresentationState(true, false);
    second->setPresentationState(true, false);
    QSignalSpy firstFinished(first, &app::notify::Message::finished);
    QSignalSpy secondFinished(second, &app::notify::Message::finished);

    center.setHostActive(true);

    QCOMPARE(firstFinished.size(), 0);
    QCOMPARE(secondFinished.size(), 0);
    QTRY_COMPARE(firstFinished.size(), 1);
    QTRY_COMPARE(secondFinished.size(), 1);
    QCOMPARE(center.count(), 0);
}

void tst_Center::autoIdsActionsAndRunsCallbacksOnce()
{
    app::notify::NotificationCenter center;
    QObject context;
    int triggeredCount = 0;
    app::notify::Options options;
    options.presentation = app::notify::Message::Presentation::Persistent;
    options.actions = {
        { .text = u"Retry"_s, .triggered = [&triggeredCount] { ++triggeredCount; } },
        { .text = u"Again"_s, .triggered = [&triggeredCount] { ++triggeredCount; } },
    };
    auto *notification = center.post(options, &context);
    QVERIFY(notification);
    const auto actions = notification->actions();
    QVERIFY(!actions.at(0).id.isEmpty());
    QVERIFY(!actions.at(1).id.isEmpty());
    QVERIFY(actions.at(0).id != actions.at(1).id);

    notification->choose(actions.at(0).id);
    notification->choose(actions.at(0).id);

    QCOMPARE(triggeredCount, 1);

    auto *updated = center.post({}, &context);
    QVERIFY(updated);
    updated->setActions(options.actions);
    const auto updatedActions = updated->actions();
    QVERIFY(!updatedActions.at(0).id.isEmpty());
    QVERIFY(!updatedActions.at(1).id.isEmpty());
    QVERIFY(updatedActions.at(0).id != updatedActions.at(1).id);
}

void tst_Center::suppressesActionCallbackWhenContextDiesDuringFinish()
{
    app::notify::NotificationCenter center;
    auto *context = new QObject;
    int triggeredCount = 0;
    app::notify::Options options;
    options.presentation = app::notify::Message::Presentation::Persistent;
    options.actions = {
        { .text = u"Retry"_s, .triggered = [&triggeredCount] { ++triggeredCount; } },
    };
    auto *notification = center.post(options, context);
    QVERIFY(notification);
    const QString actionId = notification->actions().first().id;
    QObject observer;
    connect(notification, &app::notify::Message::finished, &observer, [&context] {
        delete context;
        context = nullptr;
    });

    notification->choose(actionId);

    QCOMPARE(triggeredCount, 0);
    QVERIFY(context == nullptr);
}

void tst_Center::callsCompletionWithLiveContext()
{
    app::notify::NotificationCenter center;
    QObject context;
    int calls = 0;
    app::notify::Options options;
    options.finished = [&calls](app::notify::Message::FinishReason reason) {
        QCOMPARE(reason, app::notify::Message::FinishReason::Dismissed);
        ++calls;
    };
    auto *message = center.post(options, &context);
    QVERIFY(message);
    message->dismiss();
    message->dismiss();
    QCOMPARE(calls, 1);

    auto *dyingContext = new QObject;
    message = center.post(options, dyingContext);
    QVERIFY(message);
    QObject observer;
    connect(message, &app::notify::Message::finished, &observer, [&dyingContext] {
        delete dyingContext;
        dyingContext = nullptr;
    });
    message->dismiss();
    QCOMPARE(calls, 1);
}

QTEST_GUILESS_MAIN(tst_Center)

#include "tst_Center.moc"
