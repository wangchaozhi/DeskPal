#include "ActionController.h"

#include <QRandomGenerator>

namespace {
constexpr auto kIdleAction = "idle";
constexpr auto kHappyAction = "happy";
constexpr auto kSleepyAction = "sleepy";
constexpr auto kDraggingAction = "dragging";
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
    if (!m_dragging && m_currentAction == QLatin1String(kIdleAction)) {
        const bool becomeSleepy = QRandomGenerator::global()->bounded(100) < 35;
        const QString action = QString::fromLatin1(becomeSleepy ? kSleepyAction : kHappyAction);
        triggerAction(action, 2200);
    }

    scheduleIdleAction();
}

void ActionController::restoreIdle()
{
    if (!m_dragging) {
        setCurrentAction(QString::fromLatin1(kIdleAction));
    }
}
