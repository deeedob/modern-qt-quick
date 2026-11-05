// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "TimerStore.hpp"

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

using app::backend::TimerStore;

class tst_TimerStore : public QObject
{
    Q_OBJECT

private slots:
    void startsWithTheDefaultDuration()
    {
        const TimerStore store;
        QCOMPARE(store.durationMinutes(), 25);
        QCOMPARE(store.remainingSeconds(), 25 * 60);
        QVERIFY(!store.running());
    }

    void dispatchedActionsChangeTheState()
    {
        TimerStore store;
        QSignalSpy stateChanged(&store, &TimerStore::stateChanged);

        store.dispatch(TimerStore::Action::SetDuration, 5);
        QCOMPARE(store.remainingSeconds(), 5 * 60);
        store.dispatch(TimerStore::Action::Start);
        QVERIFY(store.running());
        store.dispatch(TimerStore::Action::Pause);
        QVERIFY(!store.running());
        QCOMPARE(stateChanged.count(), 3);
    }

    void runningTimerTicks()
    {
        TimerStore store;
        store.dispatch(TimerStore::Action::Start);
        QTRY_VERIFY(store.progress() > 0.0);
        store.dispatch(TimerStore::Action::Reset);
        QCOMPARE(store.progress(), 0.0);
        QVERIFY(!store.running());
    }
};

QTEST_GUILESS_MAIN(tst_TimerStore)
#include "tst_TimerStore.moc"
