#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QScopedPointer>
#include <QSystemTrayIcon>

class QAction;
class QMenu;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool alwaysOnTop READ alwaysOnTop WRITE setAlwaysOnTop NOTIFY alwaysOnTopChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool enabled);

    Q_INVOKABLE QPoint windowPosition() const;
    Q_INVOKABLE QRect availableGeometry(int x, int y) const;
    Q_INVOKABLE void saveWindowPosition(int x, int y);
    Q_INVOKABLE void resetWindowPosition();
    Q_INVOKABLE void showContextMenu();
    Q_INVOKABLE void showWindow();
    Q_INVOKABLE void hideWindow();
    Q_INVOKABLE void quit();

signals:
    void alwaysOnTopChanged();
    void showRequested();
    void hideRequested();
    void resetPositionRequested();

private:
    void createTrayIcon();
    void updateTrayActions();

    bool m_alwaysOnTop = true;
    QScopedPointer<QSystemTrayIcon> m_trayIcon;
    QMenu *m_trayMenu = nullptr;
    QAction *m_showAction = nullptr;
    QAction *m_hideAction = nullptr;
    QAction *m_alwaysOnTopAction = nullptr;
};
