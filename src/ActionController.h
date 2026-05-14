#pragma once

#include <QObject>
#include <QTimer>

class ActionController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentAction READ currentAction NOTIFY currentActionChanged)

public:
    explicit ActionController(QObject *parent = nullptr);

    QString currentAction() const;

    Q_INVOKABLE void triggerAction(const QString &action, int durationMs = 0);
    Q_INVOKABLE void setDragging(bool dragging);

signals:
    void currentActionChanged();

private:
    void setCurrentAction(const QString &action);
    void scheduleIdleAction();
    void triggerRandomIdleAction();
    void restoreIdle();

    QString m_currentAction = QStringLiteral("idle");
    QTimer m_restoreTimer;
    QTimer m_idleTimer;
    bool m_dragging = false;
};
