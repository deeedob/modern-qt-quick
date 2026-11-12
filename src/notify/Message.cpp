#include <Message.hpp>

#include <NotificationCenter.hpp>

#include <QtCore/QTimer>
#include <QtQml/QJSEngine>

#include <QtCore/QSet>
#include <algorithm>

namespace app::notify {

Message::Message(NotificationCenter *center, quint64 id, const Options &options, QObject *context)
    : QObject(center)
    , m_center(center)
    , m_id(id)
    , m_title(options.title)
    , m_message(options.message)
    , m_severity(options.severity)
    , m_presentation(options.presentation)
    , m_actions(options.actions)
    , m_context(context)
    , m_finishedCallback(options.finished)
    , m_timer(new QTimer(this))
    , m_remainingMs(options.timeoutMs)
    , m_timeoutMs(options.timeoutMs)
    , m_timeoutEnabled(options.timeoutMs > 0)
{
    assignActionIds(m_actions);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this] {
        m_center->finish(this, FinishReason::Expired);
    });
    connect(context, &QObject::destroyed, m_center,
        [center = m_center, notification = QPointer<Message>(this)] {
            QTimer::singleShot(0, center, [notification] {
                if (notification)
                    notification->cancel();
            });
        });
}

void Message::setTitle(const QString &title)
{
    if (m_title == title || m_finished)
        return;
    m_title = title;
    Q_EMIT titleChanged();
}

void Message::setMessage(const QString &message)
{
    if (m_message == message || m_finished)
        return;
    m_message = message;
    Q_EMIT messageChanged();
}

void Message::setActions(const QList<Action> &actions)
{
    if (m_finished)
        return;
    m_actions = actions;
    assignActionIds(m_actions);
    Q_EMIT actionsChanged();
}

void Message::setPresentation(Presentation presentation)
{
    if (m_presentation == presentation || m_finished)
        return;
    const auto previous = m_presentation;
    m_presentation = presentation;
    QPointer<Message> guardedNotification(this);
    m_center->presentationChanged(this, previous);
    if (!guardedNotification)
        return;
    updateTimerEligibility();
    Q_EMIT presentationChanged();
}

double Message::timeRemaining() const
{
    if (!m_timeoutEnabled || m_presentation != Presentation::Passive)
        return -1.0;
    const auto remaining = m_remainingMs - (m_timer->isActive() ? m_elapsed.elapsed() : 0);
    return qBound(0.0, static_cast<double>(remaining) / m_timeoutMs, 1.0);
}

void Message::choose(const QString &actionId)
{
    if (m_finished || !m_context)
        return;
    const auto action = std::find_if(m_actions.cbegin(), m_actions.cend(),
        [&actionId](const Action &item) { return item.id == actionId && item.enabled; });
    if (action == m_actions.cend())
        return;

    const auto triggered = action->triggered;
    const auto selectedId = action->id;
    const QPointer<QObject> context = m_context;
    m_center->finish(this, FinishReason::Action, selectedId);
    if (triggered && context)
        triggered();
}

void Message::dismiss()
{
    if (!m_finished && m_context)
        m_center->finish(this, FinishReason::Dismissed);
}

void Message::cancel()
{
    if (!m_finished)
        m_center->finish(this, FinishReason::Cancelled);
}

void Message::setPresentationState(bool presented, bool paused)
{
    if (m_presented == presented && m_paused == paused)
        return;
    m_presented = presented;
    m_paused = paused;
    updateTimerEligibility();
}

void Message::setCenterActive(bool active)
{
    if (m_centerActive == active)
        return;
    m_centerActive = active;
    updateTimerEligibility();
}

void Message::updateTimerEligibility()
{
    const bool shouldRun = !m_finished && m_presentation == Presentation::Passive && m_presented
        && !m_paused && m_centerActive && m_timeoutEnabled;
    if (shouldRun == m_timer->isActive())
        return;
    if (shouldRun) {
        m_elapsed.start();
        m_timer->start(m_remainingMs);
    } else if (m_timer->isActive()) {
        m_timer->stop();
        m_remainingMs = qMax(0, m_remainingMs - static_cast<int>(m_elapsed.elapsed()));
    }
}

void Message::assignActionIds(QList<Action> &actions) const
{
    QSet<QString> used;
    for (const auto &action : actions)
        if (!action.id.isEmpty())
            used.insert(action.id);

    for (qsizetype i = 0; i < actions.size(); ++i) {
        auto &action = actions[i];
        if (!action.id.isEmpty())
            continue;
        const QString baseId = QStringLiteral("notification-%1-action-%2")
                                   .arg(m_id)
                                   .arg(static_cast<qulonglong>(i));
        QString id = baseId;
        quint64 suffix = 1;
        while (used.contains(id))
            id = baseId + QLatin1Char('-') + QString::number(suffix++);
        action.id = id;
        used.insert(id);
    }
}

} // namespace app::notify
