#pragma once

#include <QtCore/QElapsedTimer>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtQmlIntegration/qqmlintegration.h>

#include <functional>

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace app::notify {

class NotificationCenter;
class Action;
struct Options;

class Message final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Message)
    QML_UNCREATABLE("Created and owned by NotificationCenter")
    Q_PROPERTY(quint64 id READ id CONSTANT FINAL)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged FINAL)
    Q_PROPERTY(app::notify::Message::Severity severity READ severity NOTIFY severityChanged FINAL)
    Q_PROPERTY(app::notify::Message::Presentation presentation READ presentation NOTIFY
            presentationChanged FINAL)
    Q_PROPERTY(QList<app::notify::Action> actions READ actions NOTIFY actionsChanged FINAL)
    Q_PROPERTY(bool isFinished READ isFinished NOTIFY isFinishedChanged FINAL)

public:
    enum class Severity { Information, Success, Warning, Error };
    Q_ENUM(Severity)

    enum class Presentation { Passive, Persistent, Decision };
    Q_ENUM(Presentation)

    enum class FinishReason { Action, Dismissed, Expired, Cancelled };
    Q_ENUM(FinishReason)

    [[nodiscard]] quint64 id() const { return m_id; }
    [[nodiscard]] QString title() const { return m_title; }
    [[nodiscard]] QString message() const { return m_message; }
    [[nodiscard]] Severity severity() const { return m_severity; }
    [[nodiscard]] Presentation presentation() const { return m_presentation; }
    [[nodiscard]] QList<Action> actions() const { return m_actions; }
    [[nodiscard]] bool isFinished() const { return m_finished; }

    void setTitle(const QString &title);
    void setMessage(const QString &message);
    void setActions(const QList<Action> &actions);
    void setPresentation(Presentation presentation);

    Q_INVOKABLE double timeRemaining() const;
    Q_INVOKABLE void choose(const QString &actionId);
    Q_INVOKABLE void dismiss();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void setPresentationState(bool presented, bool paused);

Q_SIGNALS:
    void titleChanged();
    void messageChanged();
    void severityChanged();
    void presentationChanged();
    void actionsChanged();
    void isFinishedChanged();
    void finished(app::notify::Message::FinishReason reason, const QString &actionId);

private:
    friend class NotificationCenter;

    Message(NotificationCenter *center, quint64 id, const Options &options, QObject *context);
    void updateTimerEligibility();
    void setCenterActive(bool active);
    void assignActionIds(QList<Action> &actions) const;

    NotificationCenter *m_center;
    quint64 m_id;
    QString m_title;
    QString m_message;
    Severity m_severity;
    Presentation m_presentation;
    QList<Action> m_actions;
    QPointer<QObject> m_context;
    std::function<void(FinishReason)> m_finishedCallback;
    QTimer *m_timer;
    QElapsedTimer m_elapsed;
    int m_remainingMs;
    int m_timeoutMs;
    bool m_timeoutEnabled;
    bool m_presented = false;
    bool m_paused = false;
    bool m_centerActive = false;
    bool m_finished = false;
};

class Action
{
    Q_GADGET
    QML_VALUE_TYPE(notificationAction)
    Q_PROPERTY(QString id MEMBER id FINAL)
    Q_PROPERTY(QString text MEMBER text FINAL)
    Q_PROPERTY(bool enabled MEMBER enabled FINAL)

public:
    QString id;
    QString text;
    bool enabled = true;
    std::function<void()> triggered;
    friend bool operator==(const Action &left, const Action &right)
    {
        return left.id == right.id && left.text == right.text && left.enabled == right.enabled;
    }
};

struct Options
{
    QString title;
    QString message;
    Message::Severity severity = Message::Severity::Information;
    Message::Presentation presentation = Message::Presentation::Passive;
    QList<Action> actions;
    int timeoutMs = 5000;
    std::function<void(Message::FinishReason)> finished;
};

} // namespace app::notify

Q_DECLARE_METATYPE(app::notify::Message *)
