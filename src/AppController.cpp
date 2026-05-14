#include "AppController.h"

#include "SettingsStore.h"
#include "TrayController.h"
#include "TranslationManager.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_settings(new SettingsStore(this))
    , m_translations(new TranslationManager(this))
    , m_tray(new TrayController(this))
{
    m_translations->installLanguage(m_settings->language());
    m_alwaysOnTop = m_settings->alwaysOnTop();
    m_tray->setAlwaysOnTop(m_alwaysOnTop);
    m_tray->setLanguage(m_translations->language());

    connect(m_tray, &TrayController::showRequested, this, &AppController::showWindow);
    connect(m_tray, &TrayController::hideRequested, this, &AppController::hideWindow);
    connect(m_tray, &TrayController::resetPositionRequested, this, &AppController::resetWindowPosition);
    connect(m_tray, &TrayController::alwaysOnTopToggled, this, &AppController::setAlwaysOnTop);
    connect(m_tray, &TrayController::languageChanged, this, &AppController::setLanguage);
    connect(m_tray, &TrayController::quitRequested, this, &AppController::quit);
}

AppController::~AppController() = default;

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
    m_settings->setAlwaysOnTop(m_alwaysOnTop);
    m_tray->setAlwaysOnTop(m_alwaysOnTop);
    emit alwaysOnTopChanged();
}

QString AppController::language() const
{
    return m_translations->language();
}

void AppController::setLanguage(const QString &language)
{
    if (m_translations->language() == language) {
        return;
    }

    m_settings->setLanguage(language);
    m_translations->installLanguage(language);
    m_tray->setLanguage(language);
    emit languageChanged();
}

QPoint AppController::windowPosition() const
{
    return m_settings->windowPosition();
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
    m_settings->setWindowPosition(QPoint(x, y));
}

void AppController::resetWindowPosition()
{
    m_settings->resetWindowPosition();
    emit resetPositionRequested();
}

void AppController::showContextMenu()
{
    m_tray->showContextMenu();
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
