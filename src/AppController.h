#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>

class SettingsStore;
class TrayController;

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
    bool m_alwaysOnTop = true;
    SettingsStore *m_settings = nullptr;
    TrayController *m_tray = nullptr;
};
