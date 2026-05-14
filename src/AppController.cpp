#include "AppController.h"

#include "ActionController.h"
#include "PetCatalog.h"
#include "SettingsStore.h"
#include "TrayController.h"
#include "TranslationManager.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QUrl>
#include <QScreen>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_settings(new SettingsStore(this))
    , m_actions(new ActionController(this))
    , m_petCatalog(new PetCatalog(this))
    , m_translations(new TranslationManager(this))
    , m_tray(new TrayController(this))
{
    m_translations->installLanguage(m_settings->language());
    m_alwaysOnTop = m_settings->alwaysOnTop();
    m_currentPetId = m_petCatalog->contains(m_settings->currentPetId())
            ? m_settings->currentPetId()
            : m_petCatalog->defaultPetId();
    m_renderMode = m_petCatalog->petById(m_currentPetId).type;
    m_tray->setAlwaysOnTop(m_alwaysOnTop);
    m_tray->setLanguage(m_translations->language());
    m_tray->setPetVisible(m_petVisible);
    m_tray->setRenderMode(m_renderMode);
    m_tray->setPets(m_petCatalog->pets());
    m_tray->setCurrentPet(m_currentPetId);

    connect(m_tray, &TrayController::showRequested, this, &AppController::showWindow);
    connect(m_tray, &TrayController::toggleVisibilityRequested, this, [this]() {
        if (m_petVisible) {
            hideWindow();
        } else {
            showWindow();
        }
    });
    connect(m_tray, &TrayController::resetPositionRequested, this, &AppController::resetWindowPosition);
    connect(m_tray, &TrayController::alwaysOnTopToggled, this, &AppController::setAlwaysOnTop);
    connect(m_tray, &TrayController::languageChanged, this, &AppController::setLanguage);
    connect(m_tray, &TrayController::renderModeChanged, this, &AppController::setRenderMode);
    connect(m_tray, &TrayController::petChanged, this, &AppController::setCurrentPetId);
    connect(m_tray, &TrayController::quitRequested, this, &AppController::quit);
    connect(m_actions, &ActionController::currentActionChanged, this, &AppController::petActionChanged);
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

QString AppController::petAction() const
{
    return m_actions->currentAction();
}

QString AppController::renderMode() const
{
    return m_renderMode;
}

void AppController::setRenderMode(const QString &renderMode)
{
    const QString normalizedMode = renderMode == QStringLiteral("3d") ? QStringLiteral("3d") : QStringLiteral("2d");
    if (m_renderMode == normalizedMode) {
        return;
    }

    m_renderMode = normalizedMode;
    m_settings->setRenderMode(m_renderMode);
    m_tray->setRenderMode(m_renderMode);
    emit renderModeChanged();
}

QString AppController::currentPetId() const
{
    return m_currentPetId;
}

void AppController::setCurrentPetId(const QString &petId)
{
    if (!m_petCatalog->contains(petId)) {
        return;
    }

    if (m_currentPetId == petId) {
        return;
    }

    m_currentPetId = petId;
    const PetProfile pet = m_petCatalog->petById(m_currentPetId);
    m_settings->setCurrentPetId(m_currentPetId);
    m_tray->setCurrentPet(m_currentPetId);
    emit currentPetChanged();
    setRenderMode(pet.type);
}

QString AppController::currentPetName() const
{
    return m_petCatalog->petById(m_currentPetId).name;
}

QString AppController::currentPetType() const
{
    return m_petCatalog->petById(m_currentPetId).type;
}

QString AppController::currentPetRenderer() const
{
    return m_petCatalog->petById(m_currentPetId).renderer;
}

QString AppController::currentPetSource() const
{
    return m_petCatalog->petById(m_currentPetId).source;
}

QString AppController::currentPetBasePath() const
{
    return m_petCatalog->petById(m_currentPetId).basePath;
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

void AppController::setPetVisible(bool visible)
{
    if (m_petVisible == visible) {
        return;
    }

    m_petVisible = visible;
    m_tray->setPetVisible(m_petVisible);
}

void AppController::triggerPetAction(const QString &action, int durationMs)
{
    m_actions->triggerAction(action, durationMs);
}

void AppController::setPetDragging(bool dragging)
{
    m_actions->setDragging(dragging);
}

QString AppController::currentPetActionSource(const QString &action) const
{
    const PetProfile pet = m_petCatalog->petById(m_currentPetId);
    const QString actionSource = pet.actions.value(action);
    if (!actionSource.isEmpty()) {
        return actionSource;
    }

    return pet.source;
}

QString AppController::resolvePetResource(const QString &relativePath) const
{
    if (relativePath.isEmpty()) {
        return {};
    }

    if (relativePath.startsWith(QStringLiteral("qrc:/")) || relativePath.startsWith(QStringLiteral(":/"))
        || relativePath.startsWith(QStringLiteral("file:/"))) {
        return relativePath;
    }

    const PetProfile pet = m_petCatalog->petById(m_currentPetId);
    if (pet.basePath.isEmpty()) {
        return relativePath;
    }

    return QUrl::fromLocalFile(QDir(pet.basePath).filePath(relativePath)).toString();
}

QStringList AppController::currentPetFrameUrls(const QString &action) const
{
    const PetProfile pet = m_petCatalog->petById(m_currentPetId);
    if (pet.basePath.isEmpty()) {
        return {};
    }

    const QString actionSource = currentPetActionSource(action);
    const QFileInfo sourceInfo(QDir(pet.basePath).filePath(actionSource));
    const QDir frameDir(sourceInfo.isDir() ? sourceInfo.absoluteFilePath() : sourceInfo.absolutePath());
    if (!frameDir.exists()) {
        return {};
    }

    const QStringList filters = {
        QStringLiteral("*.png"),
        QStringLiteral("*.apng"),
        QStringLiteral("*.jpg"),
        QStringLiteral("*.jpeg"),
        QStringLiteral("*.webp"),
    };

    QStringList frameUrls;
    const QFileInfoList frames = frameDir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo &frame : frames) {
        frameUrls.append(QUrl::fromLocalFile(frame.absoluteFilePath()).toString());
    }

    return frameUrls;
}

void AppController::quit()
{
    QApplication::quit();
}
