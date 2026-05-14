#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QSystemTrayIcon>

class QAction;
class QMenu;

class TrayController : public QObject
{
    Q_OBJECT

public:
    explicit TrayController(QObject *parent = nullptr);
    ~TrayController() override;

    void setAlwaysOnTop(bool enabled);
    void showContextMenu();

signals:
    void showRequested();
    void hideRequested();
    void resetPositionRequested();
    void alwaysOnTopToggled(bool enabled);
    void quitRequested();

private:
    void createMenu();

    QScopedPointer<QSystemTrayIcon> m_trayIcon;
    QMenu *m_trayMenu = nullptr;
    QAction *m_showAction = nullptr;
    QAction *m_hideAction = nullptr;
    QAction *m_alwaysOnTopAction = nullptr;
};
