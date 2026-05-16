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
    } else if (language == QStringLiteral("ja_JP")) {
        targetAction = m_japaneseLanguageAction;
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

void TrayController::setActions(const QStringList &actions)
{
    if (m_currentActions == actions || !m_actionMenu) {
        if (m_currentActions == actions) {
            return;
        }
    }

    for (QAction *action : m_actionEntries) {
        m_actionMenu->removeAction(action);
        action->deleteLater();
    }
    m_actionEntries.clear();
    m_currentActions = actions;

    for (const QString &name : m_currentActions) {
        QAction *action = m_actionMenu->addAction(name);
        action->setData(name);
        connect(action, &QAction::triggered, this, [this, action]() {
            emit actionTriggered(action->data().toString());
        });
        m_actionEntries.append(action);
    }

    retranslate();
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

    m_settingsAction = m_trayMenu->addAction(QString());
    connect(m_settingsAction, &QAction::triggered, this, &TrayController::settingsRequested);

    m_actionMenu = m_trayMenu->addMenu(QString());
    setActions({QStringLiteral("idle"), QStringLiteral("happy"),
                QStringLiteral("sleepy"), QStringLiteral("dragging")});

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

    m_japaneseLanguageAction = m_languageMenu->addAction(QString());
    m_japaneseLanguageAction->setCheckable(true);
    m_japaneseLanguageAction->setData(QStringLiteral("ja_JP"));
    m_languageActionGroup->addAction(m_japaneseLanguageAction);

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
        if (reason == QSystemTrayIcon::DoubleClick) {
            emit toggleVisibilityRequested();
        } else if (reason == QSystemTrayIcon::Trigger) {
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
    m_settingsAction->setText(tr("Pet Settings"));
    m_actionMenu->setTitle(tr("Action"));
    for (int i = 0; i < m_actionEntries.size() && i < m_currentActions.size(); ++i) {
        const QString &name = m_currentActions.at(i);
        // Translate well-known names; fall back to the raw name for custom actions.
        QString label = name;
        if (name == QStringLiteral("idle")) label = tr("Idle");
        else if (name == QStringLiteral("happy")) label = tr("Happy");
        else if (name == QStringLiteral("sleepy")) label = tr("Sleepy");
        else if (name == QStringLiteral("dragging")) label = tr("Dragging");
        m_actionEntries.at(i)->setText(label);
    }
    m_languageMenu->setTitle(tr("Language"));
    m_systemLanguageAction->setText(tr("System"));
    m_englishLanguageAction->setText(tr("English"));
    m_chineseLanguageAction->setText(tr("Simplified Chinese"));
    m_japaneseLanguageAction->setText(tr("Japanese"));
    m_resetAction->setText(tr("Reset Position"));
    m_quitAction->setText(tr("Quit"));
    m_trayIcon->setToolTip(tr("DeskPal"));
}
