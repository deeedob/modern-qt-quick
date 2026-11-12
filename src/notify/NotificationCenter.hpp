#pragma once

#include "Message.hpp"

#include <QtCore/QAbstractListModel>
#include <QtCore/QPointer>
#include <QtQmlIntegration/qqmlintegration.h>

#include <vector>

namespace app::notify {

class NotificationCenter final : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(NotificationCenter)
    QML_UNCREATABLE("Created in main() and passed to the application window")
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(app::notify::Message *currentDecision READ currentDecision NOTIFY
            currentDecisionChanged FINAL)
    Q_PROPERTY(bool hostActive READ hostActive WRITE setHostActive NOTIFY hostActiveChanged FINAL)

public:
    enum Role { NotificationRole = Qt::UserRole + 1 };
    Q_ENUM(Role)

    explicit NotificationCenter(QObject *parent = nullptr);
    ~NotificationCenter() override;

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int count() const;
    [[nodiscard]] Message *currentDecision() const;
    [[nodiscard]] bool hostActive() const { return m_hostActive; }

    Message *post(const Options &options, QObject *context);
    Q_INVOKABLE app::notify::Message *postMessage(const QString &message, QObject *context);
    void setHostActive(bool active);
    Q_INVOKABLE void dismissPassive();
    Q_INVOKABLE void shutdown();

Q_SIGNALS:
    void countChanged();
    void currentDecisionChanged();
    void hostActiveChanged();

private:
    friend class Message;

    void finish(Message *notification, Message::FinishReason reason, const QString &actionId = {});
    void presentationChanged(Message *notification, Message::Presentation previous);
    void updateTimerEligibility();
    void promoteDecision();
    void insertCard(Message *notification);
    void removeNotification(Message *notification);
    [[nodiscard]] int rowOf(const Message *notification) const;

    std::vector<Message *> m_cards;
    std::vector<Message *> m_decisions;
    QPointer<Message> m_currentDecision;
    quint64 m_nextId = 0;
    bool m_hostActive = true;
    bool m_shutdown = false;
};

} // namespace app::notify
