#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QVector>
#include <QSystemTrayIcon>

#include "PetCatalog.h"

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
    void setPetVisible(bool visible);
    void setRenderMode(const QString &renderMode);
    void setPets(const QVector<PetProfile> &pets);
    void setCurrentPet(const QString &petId);
    void showContextMenu();

signals:
    void showRequested();
    void toggleVisibilityRequested();
    void resetPositionRequested();
    void alwaysOnTopToggled(bool enabled);
    void languageChanged(const QString &language);
    void renderModeChanged(const QString &renderMode);
    void petChanged(const QString &petId);
    void settingsRequested();
    void quitRequested();

private:
    void createMenu();
    void retranslate();

    QScopedPointer<QSystemTrayIcon> m_trayIcon;
    QMenu *m_trayMenu = nullptr;
    QMenu *m_petMenu = nullptr;
    QMenu *m_languageMenu = nullptr;
    QMenu *m_renderModeMenu = nullptr;
    QAction *m_toggleVisibilityAction = nullptr;
    QAction *m_alwaysOnTopAction = nullptr;
    QAction *m_settingsAction = nullptr;
    QAction *m_resetAction = nullptr;
    QAction *m_quitAction = nullptr;
    QActionGroup *m_languageActionGroup = nullptr;
    QActionGroup *m_renderModeActionGroup = nullptr;
    QActionGroup *m_petActionGroup = nullptr;
    QAction *m_systemLanguageAction = nullptr;
    QAction *m_englishLanguageAction = nullptr;
    QAction *m_chineseLanguageAction = nullptr;
    QAction *m_render2DAction = nullptr;
    QAction *m_render3DAction = nullptr;
    QVector<QAction *> m_petActions;
    QString m_currentPetId = QStringLiteral("classic_2d");
    bool m_petVisible = true;
};
