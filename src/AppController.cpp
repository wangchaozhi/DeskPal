#include "AppController.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QGuiApplication>
#include <QIcon>
#include <QMenu>
#include <QScreen>
#include <QSettings>
#include <QStyle>

namespace {
constexpr auto kSettingsOrg = "DeskPal";
constexpr auto kSettingsApp = "DeskPal";
constexpr auto kWindowPosition = "window/position";
constexpr auto kAlwaysOnTop = "window/alwaysOnTop";
} // namespace

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    QCoreApplication::setOrganizationName(kSettingsOrg);
    QCoreApplication::setApplicationName(kSettingsApp);

    QSettings settings;
    m_alwaysOnTop = settings.value(kAlwaysOnTop, true).toBool();

    createTrayIcon();
}

AppController::~AppController()
{
    delete m_trayMenu;
}

bool AppController::alwaysOnTop() const
{
    return m_alwaysOnTop;
}

void AppController::setAlwaysOnTop(bool enabled)
{
    if (m_alwaysOnTop == enabled) {
        return;
    }

    m_alwaysOnTop = enabled;
    QSettings settings;
    settings.setValue(kAlwaysOnTop, m_alwaysOnTop);
    updateTrayActions();
    emit alwaysOnTopChanged();
}

QPoint AppController::windowPosition() const
{
    QSettings settings;
    return settings.value(kWindowPosition, QPoint(120, 120)).toPoint();
}

QRect AppController::availableGeometry(int x, int y) const
{
    QScreen *screen = QGuiApplication::screenAt(QPoint(x, y));
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    return screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);
}

void AppController::saveWindowPosition(int x, int y)
{
    QSettings settings;
    settings.setValue(kWindowPosition, QPoint(x, y));
}

void AppController::resetWindowPosition()
{
    QSettings settings;
    settings.remove(kWindowPosition);
    emit resetPositionRequested();
}

void AppController::showContextMenu()
{
    updateTrayActions();
    m_trayMenu->popup(QCursor::pos());
}

void AppController::showWindow()
{
    emit showRequested();
}

void AppController::hideWindow()
{
    emit hideRequested();
}

void AppController::quit()
{
    QApplication::quit();
}

void AppController::createTrayIcon()
{
    const QIcon icon = QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);

    m_trayMenu = new QMenu();

    m_showAction = m_trayMenu->addAction(QStringLiteral("Show Pet"));
    connect(m_showAction, &QAction::triggered, this, &AppController::showWindow);

    m_hideAction = m_trayMenu->addAction(QStringLiteral("Hide Pet"));
    connect(m_hideAction, &QAction::triggered, this, &AppController::hideWindow);

    m_alwaysOnTopAction = m_trayMenu->addAction(QStringLiteral("Always on Top"));
    m_alwaysOnTopAction->setCheckable(true);
    connect(m_alwaysOnTopAction, &QAction::toggled, this, &AppController::setAlwaysOnTop);

    m_trayMenu->addSeparator();

    auto *resetAction = m_trayMenu->addAction(QStringLiteral("Reset Position"));
    connect(resetAction, &QAction::triggered, this, &AppController::resetWindowPosition);

    auto *quitAction = m_trayMenu->addAction(QStringLiteral("Quit"));
    connect(quitAction, &QAction::triggered, this, &AppController::quit);

    m_trayIcon.reset(new QSystemTrayIcon(icon));
    m_trayIcon->setToolTip(QStringLiteral("DeskPal"));
    m_trayIcon->setContextMenu(m_trayMenu);
    connect(m_trayIcon.get(), &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            emit showRequested();
        }
    });

    updateTrayActions();
    m_trayIcon->show();
}

void AppController::updateTrayActions()
{
    if (m_alwaysOnTopAction) {
        m_alwaysOnTopAction->setChecked(m_alwaysOnTop);
    }
}
