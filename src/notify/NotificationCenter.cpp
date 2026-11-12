#include <NotificationCenter.hpp>

#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtQml/QJSEngine>

#include <algorithm>
#include <utility>

namespace app::notify {

NotificationCenter::NotificationCenter(QObject *parent)
    : QAbstractListModel(parent)
{
}

NotificationCenter::~NotificationCenter()
{
    shutdown();
}

int NotificationCenter::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_cards.size());
}

QVariant NotificationCenter::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return {};
    if (role != NotificationRole)
        return {};
    return QVariant::fromValue(m_cards[static_cast<std::size_t>(index.row())]);
}

QHash<int, QByteArray> NotificationCenter::roleNames() const
{
    return {
        { NotificationRole, QByteArrayLiteral("notification") }
    };
}

int NotificationCenter::count() const
{
    return rowCount();
}

Message *NotificationCenter::currentDecision() const
{
    return m_currentDecision;
}

Message *NotificationCenter::post(const Options &options, QObject *context)
{
    if (m_shutdown || !context || QThread::currentThread() != thread()
        || context->thread() != thread())
        return nullptr;

    auto *notification = new Message(this, ++m_nextId, options, context);
    QJSEngine::setObjectOwnership(notification, QJSEngine::CppOwnership);
    if (options.presentation == Message::Presentation::Decision) {
        if (!m_currentDecision) {
            m_currentDecision = notification;
            Q_EMIT currentDecisionChanged();
        } else {
            m_decisions.push_back(notification);
        }
        updateTimerEligibility();
    } else {
        insertCard(notification);
    }
    return notification;
}

Message *NotificationCenter::postMessage(const QString &message, QObject *context)
{
    Options options;
    options.message = message;
    return post(options, context);
}

void NotificationCenter::setHostActive(bool active)
{
    if (m_hostActive == active)
        return;
    m_hostActive = active;
    Q_EMIT hostActiveChanged();
    updateTimerEligibility();
}

void NotificationCenter::dismissPassive()
{
    std::vector<QPointer<Message>> passive;
    for (auto *message : m_cards)
        if (message->presentation() == Message::Presentation::Passive)
            passive.emplace_back(message);
    for (const auto &message : passive) {
        auto *current = message.data();
        if (current && current->presentation() == Message::Presentation::Passive)
            current->dismiss();
    }
}

void NotificationCenter::shutdown()
{
    if (m_shutdown)
        return;
    m_shutdown = true;
    std::vector<QPointer<Message>> requests;
    requests.reserve(m_cards.size() + m_decisions.size() + (m_currentDecision ? 1U : 0U));
    for (auto *notification : m_cards)
        requests.emplace_back(notification);
    for (auto *notification : m_decisions)
        requests.emplace_back(notification);
    if (m_currentDecision)
        requests.emplace_back(m_currentDecision);
    for (const auto &notification : requests)
        if (notification)
            finish(notification, Message::FinishReason::Cancelled);
}

void NotificationCenter::finish(Message *notification, Message::FinishReason reason,
    const QString &actionId)
{
    if (!notification || notification->m_finished)
        return;

    const auto callback = std::move(notification->m_finishedCallback);
    const QPointer<QObject> context = notification->m_context;
    notification->m_finished = true;
    notification->m_actions.clear();
    notification->m_timer->stop();
    QPointer<Message> guardedNotification(notification);
    removeNotification(notification);
    if (!guardedNotification)
        return;
    Q_EMIT guardedNotification->actionsChanged();
    if (!guardedNotification)
        return;
    Q_EMIT guardedNotification->isFinishedChanged();
    if (!guardedNotification)
        return;
    Q_EMIT guardedNotification->finished(reason, actionId);
    if (callback && context)
        callback(reason);
    if (guardedNotification)
        guardedNotification->deleteLater();
}

void NotificationCenter::presentationChanged(Message *notification, Message::Presentation previous)
{
    const auto current = notification->presentation();
    if (previous != Message::Presentation::Decision && current != Message::Presentation::Decision) {
        updateTimerEligibility();
        return;
    }

    if (previous == Message::Presentation::Decision) {
        if (m_currentDecision == notification) {
            m_currentDecision = nullptr;
            promoteDecision();
        } else {
            std::erase(m_decisions, notification);
        }
        insertCard(notification);
    } else if (notification->presentation() == Message::Presentation::Decision) {
        const int row = rowOf(notification);
        if (row >= 0) {
            beginRemoveRows({}, row, row);
            m_cards.erase(m_cards.begin() + row);
            endRemoveRows();
            Q_EMIT countChanged();
        }
        if (!m_currentDecision) {
            m_currentDecision = notification;
            Q_EMIT currentDecisionChanged();
        } else {
            m_decisions.push_back(notification);
        }
        updateTimerEligibility();
    }
}

void NotificationCenter::updateTimerEligibility()
{
    const bool active = m_hostActive && !m_currentDecision;
    for (auto *notification : m_cards)
        notification->setCenterActive(active);
}

void NotificationCenter::promoteDecision()
{
    if (m_currentDecision)
        return;
    if (!m_shutdown && !m_decisions.empty()) {
        m_currentDecision = m_decisions.front();
        m_decisions.erase(m_decisions.begin());
    }
    Q_EMIT currentDecisionChanged();
    updateTimerEligibility();
}

void NotificationCenter::insertCard(Message *notification)
{
    const auto severity = notification->severity();
    const auto row = std::ranges::find_if(m_cards.begin(), m_cards.end(),
        [severity](const Message *existing) {
            return static_cast<int>(existing->severity()) <= static_cast<int>(severity);
        });
    const int position = static_cast<int>(std::distance(m_cards.begin(), row));
    beginInsertRows({}, position, position);
    m_cards.insert(row, notification);
    endInsertRows();
    Q_EMIT countChanged();
    updateTimerEligibility();
}

void NotificationCenter::removeNotification(Message *notification)
{
    const int row = rowOf(notification);
    if (row >= 0) {
        beginRemoveRows({}, row, row);
        m_cards.erase(m_cards.begin() + row);
        endRemoveRows();
        Q_EMIT countChanged();
        updateTimerEligibility();
        return;
    }

    if (m_currentDecision == notification) {
        m_currentDecision = nullptr;
        promoteDecision();
        return;
    }
    std::erase(m_decisions, notification);
}

int NotificationCenter::rowOf(const Message *notification) const
{
    const auto found = std::ranges::find(m_cards, notification);
    return found == m_cards.end() ? -1 : static_cast<int>(std::distance(m_cards.begin(), found));
}

} // namespace app::notify
