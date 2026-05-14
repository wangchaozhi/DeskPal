#include "TrayController.h"

#include <QAction>
#include <QActionGroup>
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

void TrayController::setLanguage(const QString &language)
{
    QAction *targetAction = m_systemLanguageAction;
    if (language == QStringLiteral("en")) {
        targetAction = m_englishLanguageAction;
    } else if (language == QStringLiteral("zh_CN")) {
        targetAction = m_chineseLanguageAction;
    }

    if (targetAction) {
        const QSignalBlocker blocker(m_languageActionGroup);
        targetAction->setChecked(true);
    }

    retranslate();
}

void TrayController::setPetVisible(bool visible)
{
    if (m_petVisible == visible) {
        return;
    }

    m_petVisible = visible;
    retranslate();
}

void TrayController::setRenderMode(const QString &renderMode)
{
    QAction *targetAction = renderMode == QStringLiteral("3d") ? m_render3DAction : m_render2DAction;
    if (targetAction) {
        const QSignalBlocker blocker(m_renderModeActionGroup);
        targetAction->setChecked(true);
    }
}

void TrayController::setPets(const QVector<PetProfile> &pets)
{
    if (!m_petMenu || !m_petActionGroup) {
        return;
    }

    for (QAction *action : m_petActions) {
        m_petMenu->removeAction(action);
        action->deleteLater();
    }
    m_petActions.clear();

    for (const PetProfile &pet : pets) {
        QAction *action = m_petMenu->addAction(pet.name);
        action->setCheckable(true);
        action->setData(pet.id);
        m_petActionGroup->addAction(action);
        m_petActions.append(action);
    }

    setCurrentPet(m_currentPetId);
}

void TrayController::setCurrentPet(const QString &petId)
{
    m_currentPetId = petId;
    for (QAction *action : m_petActions) {
        const QSignalBlocker blocker(action);
        action->setChecked(action->data().toString() == m_currentPetId);
    }
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

    m_toggleVisibilityAction = m_trayMenu->addAction(QString());
    connect(m_toggleVisibilityAction, &QAction::triggered, this, &TrayController::toggleVisibilityRequested);

    m_petMenu = m_trayMenu->addMenu(QString());
    m_petActionGroup = new QActionGroup(this);
    m_petActionGroup->setExclusive(true);
    connect(m_petActionGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        emit petChanged(action->data().toString());
    });

    m_alwaysOnTopAction = m_trayMenu->addAction(QString());
    m_alwaysOnTopAction->setCheckable(true);
    connect(m_alwaysOnTopAction, &QAction::toggled, this, &TrayController::alwaysOnTopToggled);

    m_renderModeMenu = m_trayMenu->addMenu(QString());
    m_renderModeActionGroup = new QActionGroup(this);
    m_renderModeActionGroup->setExclusive(true);

    m_render2DAction = m_renderModeMenu->addAction(QString());
    m_render2DAction->setCheckable(true);
    m_render2DAction->setData(QStringLiteral("2d"));
    m_renderModeActionGroup->addAction(m_render2DAction);

    m_render3DAction = m_renderModeMenu->addAction(QString());
    m_render3DAction->setCheckable(true);
    m_render3DAction->setData(QStringLiteral("3d"));
    m_renderModeActionGroup->addAction(m_render3DAction);

    connect(m_renderModeActionGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        emit renderModeChanged(action->data().toString());
    });

    m_settingsAction = m_trayMenu->addAction(QString());
    connect(m_settingsAction, &QAction::triggered, this, &TrayController::settingsRequested);

    m_languageMenu = m_trayMenu->addMenu(QString());
    m_languageActionGroup = new QActionGroup(this);
    m_languageActionGroup->setExclusive(true);

    m_systemLanguageAction = m_languageMenu->addAction(QString());
    m_systemLanguageAction->setCheckable(true);
    m_systemLanguageAction->setData(QStringLiteral("system"));
    m_languageActionGroup->addAction(m_systemLanguageAction);

    m_englishLanguageAction = m_languageMenu->addAction(QString());
    m_englishLanguageAction->setCheckable(true);
    m_englishLanguageAction->setData(QStringLiteral("en"));
    m_languageActionGroup->addAction(m_englishLanguageAction);

    m_chineseLanguageAction = m_languageMenu->addAction(QString());
    m_chineseLanguageAction->setCheckable(true);
    m_chineseLanguageAction->setData(QStringLiteral("zh_CN"));
    m_languageActionGroup->addAction(m_chineseLanguageAction);

    connect(m_languageActionGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        emit languageChanged(action->data().toString());
    });

    m_trayMenu->addSeparator();

    m_resetAction = m_trayMenu->addAction(QString());
    connect(m_resetAction, &QAction::triggered, this, &TrayController::resetPositionRequested);

    m_quitAction = m_trayMenu->addAction(QString());
    connect(m_quitAction, &QAction::triggered, this, &TrayController::quitRequested);

    m_trayIcon.reset(new QSystemTrayIcon(icon));
    m_trayIcon->setContextMenu(m_trayMenu);
    connect(m_trayIcon.get(), &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            emit showRequested();
        }
    });

    retranslate();
    m_trayIcon->show();
}

void TrayController::retranslate()
{
    m_toggleVisibilityAction->setText(m_petVisible ? tr("Hide Pet") : tr("Show Pet"));
    m_petMenu->setTitle(tr("Pet"));
    m_alwaysOnTopAction->setText(tr("Always on Top"));
    m_renderModeMenu->setTitle(tr("Render Mode"));
    m_render2DAction->setText(tr("2D Pet"));
    m_render3DAction->setText(tr("3D Pet"));
    m_settingsAction->setText(tr("Pet Settings"));
    m_languageMenu->setTitle(tr("Language"));
    m_systemLanguageAction->setText(tr("System"));
    m_englishLanguageAction->setText(tr("English"));
    m_chineseLanguageAction->setText(tr("Simplified Chinese"));
    m_resetAction->setText(tr("Reset Position"));
    m_quitAction->setText(tr("Quit"));
    m_trayIcon->setToolTip(tr("DeskPal"));
}
