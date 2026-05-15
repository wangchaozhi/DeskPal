#include "ActionController.h"

#include <QRandomGenerator>
#include <QTime>

namespace {
constexpr auto kIdleAction = "idle";
constexpr auto kHappyAction = "happy";
constexpr auto kSleepyAction = "sleepy";
constexpr auto kDraggingAction = "dragging";
constexpr auto kWalkingAction = "walking";
constexpr int kDefaultActionDurationMs = 1800;
} // namespace

ActionController::ActionController(QObject *parent)
    : QObject(parent)
{
    m_restoreTimer.setSingleShot(true);
    connect(&m_restoreTimer, &QTimer::timeout, this, &ActionController::restoreIdle);

    m_idleTimer.setSingleShot(true);
    connect(&m_idleTimer, &QTimer::timeout, this, &ActionController::triggerRandomIdleAction);

    scheduleIdleAction();
}

QString ActionController::currentAction() const
{
    return m_currentAction;
}

void ActionController::triggerAction(const QString &action, int durationMs)
{
    if (action.isEmpty()) {
        return;
    }

    setCurrentAction(action);

    if (durationMs > 0) {
        m_restoreTimer.start(durationMs);
    } else if (action != QLatin1String(kIdleAction) && action != QLatin1String(kDraggingAction)) {
        m_restoreTimer.start(kDefaultActionDurationMs);
    }
}

void ActionController::setIdleActions(const QStringList &actions)
{
    QStringList filtered;
    for (const QString &action : actions) {
        if (action == QLatin1String(kIdleAction) || action == QLatin1String(kDraggingAction)) {
            continue;
        }
        if (!action.isEmpty() && !filtered.contains(action)) {
            filtered.append(action);
        }
    }

    m_idleActions = filtered.isEmpty()
            ? QStringList{QString::fromLatin1(kHappyAction), QString::fromLatin1(kSleepyAction)}
            : filtered;
}

void ActionController::setDragging(bool dragging)
{
    if (m_dragging == dragging) {
        return;
    }

    m_dragging = dragging;

    if (m_dragging) {
        m_restoreTimer.stop();
        setCurrentAction(QString::fromLatin1(kDraggingAction));
    } else {
        restoreIdle();
    }
}

void ActionController::setWalking(bool walking)
{
    if (m_walking == walking) {
        return;
    }

    m_walking = walking;

    if (m_dragging) {
        return;
    }

    if (m_walking) {
        m_restoreTimer.stop();
        setCurrentAction(QString::fromLatin1(kWalkingAction));
    } else {
        restoreIdle();
    }
}

void ActionController::setCurrentAction(const QString &action)
{
    if (m_currentAction == action) {
        return;
    }

    m_currentAction = action;
    emit currentActionChanged();
}

void ActionController::scheduleIdleAction()
{
    m_idleTimer.start(QRandomGenerator::global()->bounded(5000, 11000));
}

void ActionController::triggerRandomIdleAction()
{
    if (!m_dragging && m_currentAction == QLatin1String(kIdleAction) && !m_idleActions.isEmpty()) {
        QString choice;
        if (m_nightSleepyEnabled
            && m_idleActions.contains(QString::fromLatin1(kSleepyAction))) {
            const int hour = QTime::currentTime().hour();
            const bool isNight = hour >= 22 || hour < 7;
            if (isNight) {
                choice = QString::fromLatin1(kSleepyAction);
            }
        }
        if (choice.isEmpty()) {
            const int index = QRandomGenerator::global()->bounded(m_idleActions.size());
            choice = m_idleActions.at(index);
        }
        triggerAction(choice, 2200);
    }

    scheduleIdleAction();
}

void ActionController::setNightSleepyEnabled(bool enabled)
{
    m_nightSleepyEnabled = enabled;
}

void ActionController::restoreIdle()
{
    if (m_dragging) {
        return;
    }

    setCurrentAction(QString::fromLatin1(m_walking ? kWalkingAction : kIdleAction));
}
