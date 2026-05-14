#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QSystemTrayIcon>

class QAction;
class QActionGroup;
class QMenu;

class TrayController : public QObject
{
    Q_OBJECT

public:
    explicit TrayController(QObject *parent = nullptr);
    ~TrayController() override;

    void setAlwaysOnTop(bool enabled);
    void setLanguage(const QString &language);
    void showContextMenu();

signals:
    void showRequested();
    void hideRequested();
    void resetPositionRequested();
    void alwaysOnTopToggled(bool enabled);
    void languageChanged(const QString &language);
    void quitRequested();

private:
    void createMenu();
    void retranslate();

    QScopedPointer<QSystemTrayIcon> m_trayIcon;
    QMenu *m_trayMenu = nullptr;
    QMenu *m_languageMenu = nullptr;
    QAction *m_showAction = nullptr;
    QAction *m_hideAction = nullptr;
    QAction *m_alwaysOnTopAction = nullptr;
    QAction *m_resetAction = nullptr;
    QAction *m_quitAction = nullptr;
    QActionGroup *m_languageActionGroup = nullptr;
    QAction *m_systemLanguageAction = nullptr;
    QAction *m_englishLanguageAction = nullptr;
    QAction *m_chineseLanguageAction = nullptr;
};
