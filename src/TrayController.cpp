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

    m_showAction = m_trayMenu->addAction(QString());
    connect(m_showAction, &QAction::triggered, this, &TrayController::showRequested);

    m_hideAction = m_trayMenu->addAction(QString());
    connect(m_hideAction, &QAction::triggered, this, &TrayController::hideRequested);

    m_alwaysOnTopAction = m_trayMenu->addAction(QString());
    m_alwaysOnTopAction->setCheckable(true);
    connect(m_alwaysOnTopAction, &QAction::toggled, this, &TrayController::alwaysOnTopToggled);

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
    m_showAction->setText(tr("Show Pet"));
    m_hideAction->setText(tr("Hide Pet"));
    m_alwaysOnTopAction->setText(tr("Always on Top"));
    m_languageMenu->setTitle(tr("Language"));
    m_systemLanguageAction->setText(tr("System"));
    m_englishLanguageAction->setText(tr("English"));
    m_chineseLanguageAction->setText(tr("Simplified Chinese"));
    m_resetAction->setText(tr("Reset Position"));
    m_quitAction->setText(tr("Quit"));
    m_trayIcon->setToolTip(tr("DeskPal"));
}
