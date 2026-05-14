#include "TrayController.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QIcon>
#include <QMenu>
#include <QSignalBlocker>
#include <QStyle>

TrayController::TrayController(QObject *parent)
    : QObject(parent)
{
    createMenu();
}

TrayController::~TrayController()
{
    delete m_trayMenu;
}

void TrayController::setAlwaysOnTop(bool enabled)
{
    if (!m_alwaysOnTopAction) {
        return;
    }

    const QSignalBlocker blocker(m_alwaysOnTopAction);
    m_alwaysOnTopAction->setChecked(enabled);
}

void TrayController::showContextMenu()
{
    if (m_trayMenu) {
        m_trayMenu->popup(QCursor::pos());
    }
}

void TrayController::createMenu()
{
    const QIcon icon = QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);

    m_trayMenu = new QMenu();

    m_showAction = m_trayMenu->addAction(QStringLiteral("Show Pet"));
    connect(m_showAction, &QAction::triggered, this, &TrayController::showRequested);

    m_hideAction = m_trayMenu->addAction(QStringLiteral("Hide Pet"));
    connect(m_hideAction, &QAction::triggered, this, &TrayController::hideRequested);

    m_alwaysOnTopAction = m_trayMenu->addAction(QStringLiteral("Always on Top"));
    m_alwaysOnTopAction->setCheckable(true);
    connect(m_alwaysOnTopAction, &QAction::toggled, this, &TrayController::alwaysOnTopToggled);

    m_trayMenu->addSeparator();

    auto *resetAction = m_trayMenu->addAction(QStringLiteral("Reset Position"));
    connect(resetAction, &QAction::triggered, this, &TrayController::resetPositionRequested);

    auto *quitAction = m_trayMenu->addAction(QStringLiteral("Quit"));
    connect(quitAction, &QAction::triggered, this, &TrayController::quitRequested);

    m_trayIcon.reset(new QSystemTrayIcon(icon));
    m_trayIcon->setToolTip(QStringLiteral("DeskPal"));
    m_trayIcon->setContextMenu(m_trayMenu);
    connect(m_trayIcon.get(), &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            emit showRequested();
        }
    });

    m_trayIcon->show();
}
