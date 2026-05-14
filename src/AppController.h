#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>

class SettingsStore;
class TrayController;
class TranslationManager;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool alwaysOnTop READ alwaysOnTop WRITE setAlwaysOnTop NOTIFY alwaysOnTopChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool enabled);
    QString language() const;
    void setLanguage(const QString &language);

    Q_INVOKABLE QPoint windowPosition() const;
    Q_INVOKABLE QRect availableGeometry(int x, int y) const;
    Q_INVOKABLE void saveWindowPosition(int x, int y);
    Q_INVOKABLE void resetWindowPosition();
    Q_INVOKABLE void showContextMenu();
    Q_INVOKABLE void showWindow();
    Q_INVOKABLE void hideWindow();
    Q_INVOKABLE void setPetVisible(bool visible);
    Q_INVOKABLE void quit();

signals:
    void alwaysOnTopChanged();
    void languageChanged();
    void showRequested();
    void hideRequested();
    void resetPositionRequested();

private:
    bool m_alwaysOnTop = true;
    SettingsStore *m_settings = nullptr;
    TranslationManager *m_translations = nullptr;
    TrayController *m_tray = nullptr;
    bool m_petVisible = true;
};
