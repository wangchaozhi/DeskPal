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
#include <QRandomGenerator>
#include <QUrl>
#include <QScreen>
#include <QVariantMap>

namespace {
QString petResourcePath(const PetProfile &pet, const QString &relativePath)
{
    if (relativePath.isEmpty() || pet.basePath.isEmpty()) {
        return {};
    }

    return QDir(pet.basePath).filePath(relativePath);
}

bool hasImageFrames(const QString &directoryPath)
{
    const QDir directory(directoryPath);
    if (!directory.exists()) {
        return false;
    }

    const QStringList filters = {
        QStringLiteral("*.png"),
        QStringLiteral("*.apng"),
        QStringLiteral("*.jpg"),
        QStringLiteral("*.jpeg"),
        QStringLiteral("*.webp"),
    };

    return !directory.entryInfoList(filters, QDir::Files, QDir::Name).isEmpty();
}

QString slugifyPetId(const QString &name)
{
    QString slug;
    for (const QChar &ch : name.toLower()) {
        if (ch.isLetterOrNumber()) {
            slug.append(ch);
        } else if (ch.isSpace() || ch == QLatin1Char('-') || ch == QLatin1Char('_')) {
            if (!slug.endsWith(QLatin1Char('_'))) {
                slug.append(QLatin1Char('_'));
            }
        }
    }

    while (slug.startsWith(QLatin1Char('_'))) {
        slug.remove(0, 1);
    }
    while (slug.endsWith(QLatin1Char('_'))) {
        slug.chop(1);
    }

    return slug.isEmpty() ? QStringLiteral("pet") : slug;
}

QStringList validatePet(const PetProfile &pet)
{
    QStringList issues;
    if (pet.id.isEmpty()) {
        issues.append(QObject::tr("Missing pet ID"));
    }
    if (pet.name.isEmpty()) {
        issues.append(QObject::tr("Missing pet name"));
    }
    if (pet.type != QStringLiteral("2d") && pet.type != QStringLiteral("3d")) {
        issues.append(QObject::tr("Invalid pet type"));
    }
    if (pet.renderer.isEmpty()) {
        issues.append(QObject::tr("Missing renderer"));
    }

    if (pet.basePath.isEmpty()) {
        return issues;
    }

    const bool needsSourceFile = pet.renderer != QStringLiteral("png-sequence");
    if (pet.source.isEmpty()) {
        issues.append(QObject::tr("Missing source"));
    } else if (needsSourceFile && !QFileInfo::exists(petResourcePath(pet, pet.source))) {
        issues.append(QObject::tr("Source not found"));
    }

    if (pet.renderer == QStringLiteral("png-sequence")) {
        const QString sourcePath = petResourcePath(pet, pet.source);
        if (!pet.source.isEmpty() && !hasImageFrames(sourcePath)) {
            issues.append(QObject::tr("No frames in source directory"));
        }
    }

    const QStringList expectedActions = {
        QStringLiteral("idle"),
        QStringLiteral("happy"),
        QStringLiteral("sleepy"),
        QStringLiteral("dragging"),
    };
    for (const QString &action : expectedActions) {
        const QString actionSource = pet.actions.value(action);
        if (actionSource.isEmpty()) {
            issues.append(QObject::tr("Missing action: %1").arg(action));
            continue;
        }

        const QString actionPath = petResourcePath(pet, actionSource);
        if (pet.renderer == QStringLiteral("png-sequence")) {
            if (!hasImageFrames(actionPath)) {
                issues.append(QObject::tr("No frames for action: %1").arg(action));
            }
        } else if (!QFileInfo::exists(actionPath)) {
            issues.append(QObject::tr("Action resource not found: %1").arg(action));
        }
    }

    return issues;
}
} // namespace

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
    m_tray->setAlwaysOnTop(m_alwaysOnTop);
    m_tray->setLanguage(m_translations->language());
    m_tray->setPetVisible(m_petVisible);
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
    connect(m_tray, &TrayController::petChanged, this, &AppController::setCurrentPetId);
    connect(m_tray, &TrayController::settingsRequested, this, &AppController::settingsRequested);
    connect(m_tray, &TrayController::quitRequested, this, &AppController::quit);
    connect(m_actions, &ActionController::currentActionChanged, this, &AppController::petActionChanged);

    applyPetToActionController();
}

AppController::~AppController() = default;

void AppController::applyPetToActionController()
{
    m_actions->setIdleActions(m_petCatalog->petById(m_currentPetId).idleActions);
}

void AppController::setLastError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }

    m_lastError = error;
    emit lastErrorChanged();
}

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
    m_settings->setCurrentPetId(m_currentPetId);
    m_tray->setCurrentPet(m_currentPetId);
    applyPetToActionController();
    emit currentPetChanged();
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

int AppController::currentPetWidth() const
{
    return m_petCatalog->petById(m_currentPetId).width;
}

int AppController::currentPetHeight() const
{
    return m_petCatalog->petById(m_currentPetId).height;
}

int AppController::currentPetFps() const
{
    return m_petCatalog->petById(m_currentPetId).fps;
}

qreal AppController::currentPetScale() const
{
    return m_petCatalog->petById(m_currentPetId).scale;
}

qreal AppController::petOpacity() const
{
    return m_settings->petOpacity();
}

void AppController::setPetOpacity(qreal opacity)
{
    if (opacity < 0.2) {
        opacity = 0.2;
    } else if (opacity > 1.0) {
        opacity = 1.0;
    }

    if (qFuzzyCompare(m_settings->petOpacity(), opacity)) {
        return;
    }

    m_settings->setPetOpacity(opacity);
    emit petOpacityChanged();
}

bool AppController::autoStart() const
{
    return m_settings->autoStartEnabled();
}

void AppController::setAutoStart(bool enabled)
{
    if (m_settings->autoStartEnabled() == enabled) {
        return;
    }

    m_settings->setAutoStartEnabled(enabled);
    emit autoStartChanged();
}

bool AppController::wanderEnabled() const
{
    return m_settings->wanderEnabled();
}

void AppController::setWanderEnabled(bool enabled)
{
    if (m_settings->wanderEnabled() == enabled) {
        return;
    }

    m_settings->setWanderEnabled(enabled);
    emit wanderEnabledChanged();
}

QString AppController::lastError() const
{
    return m_lastError;
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

void AppController::setPetWalking(bool walking)
{
    m_actions->setWalking(walking);
}

QString AppController::currentPetActionSource(const QString &action) const
{
    return petActionSource(m_currentPetId, action);
}

QString AppController::resolvePetResource(const QString &relativePath) const
{
    return resolvePetResourceForPet(m_currentPetId, relativePath);
}

QStringList AppController::currentPetFrameUrls(const QString &action) const
{
    return petFrameUrls(m_currentPetId, action);
}

QString AppController::petActionSource(const QString &petId, const QString &action) const
{
    const PetProfile pet = m_petCatalog->petById(petId);
    const QString actionSource = pet.actions.value(action);
    if (!actionSource.isEmpty()) {
        return actionSource;
    }

    return pet.source;
}

QString AppController::resolvePetResourceForPet(const QString &petId, const QString &relativePath) const
{
    if (relativePath.isEmpty()) {
        return {};
    }

    if (relativePath.startsWith(QStringLiteral("qrc:/")) || relativePath.startsWith(QStringLiteral(":/"))
        || relativePath.startsWith(QStringLiteral("file:/"))) {
        return relativePath;
    }

    const PetProfile pet = m_petCatalog->petById(petId);
    if (pet.basePath.isEmpty()) {
        return relativePath;
    }

    return QUrl::fromLocalFile(QDir(pet.basePath).filePath(relativePath)).toString();
}

QStringList AppController::petFrameUrls(const QString &petId, const QString &action) const
{
    const PetProfile pet = m_petCatalog->petById(petId);
    if (pet.basePath.isEmpty()) {
        return {};
    }

    const QString actionSource = petActionSource(petId, action);
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

int AppController::petFps(const QString &petId) const
{
    return m_petCatalog->petById(petId).fps;
}

QString AppController::petAnimationClip(const QString &petId, const QString &action) const
{
    return m_petCatalog->petById(petId).animations.value(action);
}

QString AppController::randomSpeech() const
{
    QStringList phrases = m_petCatalog->petById(m_currentPetId).speeches;
    if (phrases.isEmpty()) {
        phrases = {
            tr("Hi there!"),
            tr("Need a break?"),
            tr("I'm right here with you."),
            tr("Keep going, you've got this!"),
            tr("Let's have some fun."),
        };
    }

    return phrases.at(QRandomGenerator::global()->bounded(phrases.size()));
}

QVariantList AppController::petProfiles() const
{
    QVariantList profiles;
    for (const PetProfile &pet : m_petCatalog->pets()) {
        const QStringList issues = validatePet(pet);
        QVariantMap profile;
        profile.insert(QStringLiteral("id"), pet.id);
        profile.insert(QStringLiteral("name"), pet.name);
        profile.insert(QStringLiteral("type"), pet.type);
        profile.insert(QStringLiteral("renderer"), pet.renderer);
        profile.insert(QStringLiteral("source"), pet.source);
        profile.insert(QStringLiteral("basePath"), pet.basePath);
        profile.insert(QStringLiteral("width"), pet.width);
        profile.insert(QStringLiteral("height"), pet.height);
        profile.insert(QStringLiteral("fps"), pet.fps);
        profile.insert(QStringLiteral("scale"), pet.scale);
        profile.insert(QStringLiteral("idleActions"), pet.idleActions);
        profile.insert(QStringLiteral("speeches"), pet.speeches);
        profile.insert(QStringLiteral("editable"), !pet.basePath.isEmpty());

        QStringList actionNames = pet.actions.keys();
        actionNames.sort();
        QVariantMap actions;
        for (const QString &actionName : actionNames) {
            actions.insert(actionName, pet.actions.value(actionName));
        }
        profile.insert(QStringLiteral("actions"), actions);
        profile.insert(QStringLiteral("actionText"), actionNames.join(QStringLiteral(", ")));

        QVariantMap animations;
        for (auto it = pet.animations.constBegin(); it != pet.animations.constEnd(); ++it) {
            animations.insert(it.key(), it.value());
        }
        profile.insert(QStringLiteral("animations"), animations);

        profile.insert(QStringLiteral("isValid"), issues.isEmpty());
        profile.insert(QStringLiteral("issueText"), issues.isEmpty() ? tr("Ready") : issues.join(QStringLiteral("\n")));
        profiles.append(profile);
    }

    return profiles;
}

QVariantList AppController::reloadPetProfiles()
{
    m_petCatalog->reload();
    if (!m_petCatalog->contains(m_currentPetId)) {
        setCurrentPetId(m_petCatalog->defaultPetId());
    }
    m_tray->setPets(m_petCatalog->pets());
    m_tray->setCurrentPet(m_currentPetId);
    applyPetToActionController();
    return petProfiles();
}

bool AppController::savePetProfile(const QVariantMap &profile)
{
    const QString petId = profile.value(QStringLiteral("id")).toString().trimmed();
    if (petId.isEmpty() || !m_petCatalog->contains(petId)) {
        setLastError(tr("Unknown pet"));
        return false;
    }

    PetProfile pet = m_petCatalog->petById(petId);
    if (pet.basePath.isEmpty()) {
        setLastError(tr("Built-in pets cannot be edited"));
        return false;
    }

    if (profile.contains(QStringLiteral("name"))) {
        const QString name = profile.value(QStringLiteral("name")).toString().trimmed();
        if (!name.isEmpty()) {
            pet.name = name;
        }
    }
    if (profile.contains(QStringLiteral("renderer"))) {
        pet.renderer = profile.value(QStringLiteral("renderer")).toString().trimmed().toLower();
    }
    if (profile.contains(QStringLiteral("source"))) {
        pet.source = profile.value(QStringLiteral("source")).toString().trimmed();
    }
    if (profile.contains(QStringLiteral("width"))) {
        pet.width = profile.value(QStringLiteral("width")).toInt();
    }
    if (profile.contains(QStringLiteral("height"))) {
        pet.height = profile.value(QStringLiteral("height")).toInt();
    }
    if (profile.contains(QStringLiteral("fps"))) {
        pet.fps = profile.value(QStringLiteral("fps")).toInt();
    }
    if (profile.contains(QStringLiteral("scale"))) {
        const qreal scale = profile.value(QStringLiteral("scale")).toReal();
        pet.scale = scale > 0.0 ? scale : 1.0;
    }
    if (profile.contains(QStringLiteral("actions"))) {
        const QVariantMap actions = profile.value(QStringLiteral("actions")).toMap();
        pet.actions.clear();
        for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
            const QString value = it.value().toString().trimmed();
            if (!value.isEmpty()) {
                pet.actions.insert(it.key(), value);
            }
        }
    }
    if (profile.contains(QStringLiteral("animations"))) {
        const QVariantMap animations = profile.value(QStringLiteral("animations")).toMap();
        pet.animations.clear();
        for (auto it = animations.constBegin(); it != animations.constEnd(); ++it) {
            const QString value = it.value().toString().trimmed();
            if (!value.isEmpty()) {
                pet.animations.insert(it.key(), value);
            }
        }
    }
    if (profile.contains(QStringLiteral("idleActions"))) {
        pet.idleActions = profile.value(QStringLiteral("idleActions")).toStringList();
    }
    if (profile.contains(QStringLiteral("speeches"))) {
        pet.speeches = profile.value(QStringLiteral("speeches")).toStringList();
    }

    QString error;
    if (!m_petCatalog->saveProfile(pet, &error)) {
        setLastError(error);
        return false;
    }

    reloadPetProfiles();
    if (m_currentPetId == petId) {
        applyPetToActionController();
        emit currentPetChanged();
    }
    setLastError(QString());
    return true;
}

bool AppController::importPetPack(const QString &folderUrl)
{
    const QString localPath = QUrl(folderUrl).isLocalFile() ? QUrl(folderUrl).toLocalFile() : folderUrl;
    QString error;
    if (!m_petCatalog->importPetPack(localPath, &error)) {
        setLastError(error);
        return false;
    }

    m_tray->setPets(m_petCatalog->pets());
    m_tray->setCurrentPet(m_currentPetId);
    setLastError(QString());
    return true;
}

bool AppController::exportPetPack(const QString &petId, const QString &folderUrl)
{
    const QString localPath = QUrl(folderUrl).isLocalFile() ? QUrl(folderUrl).toLocalFile() : folderUrl;
    QString error;
    if (!m_petCatalog->exportPetPack(petId, localPath, &error)) {
        setLastError(error);
        return false;
    }

    setLastError(QString());
    return true;
}

bool AppController::createPet(const QString &name, const QString &type)
{
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        setLastError(tr("Pet name is required"));
        return false;
    }

    const QString baseId = slugifyPetId(trimmedName);
    QString uniqueId = baseId;
    int suffix = 1;
    while (m_petCatalog->contains(uniqueId)) {
        uniqueId = baseId + QStringLiteral("_%1").arg(suffix++);
    }

    QString error;
    if (!m_petCatalog->createPet(uniqueId, trimmedName, type, &error)) {
        setLastError(error);
        return false;
    }

    m_tray->setPets(m_petCatalog->pets());
    m_tray->setCurrentPet(m_currentPetId);
    setLastError(QString());
    return true;
}

bool AppController::deletePet(const QString &petId)
{
    QString error;
    if (!m_petCatalog->deletePet(petId, &error)) {
        setLastError(error);
        return false;
    }

    if (!m_petCatalog->contains(m_currentPetId)) {
        m_currentPetId = m_petCatalog->defaultPetId();
        m_settings->setCurrentPetId(m_currentPetId);
        applyPetToActionController();
        emit currentPetChanged();
    }

    m_tray->setPets(m_petCatalog->pets());
    m_tray->setCurrentPet(m_currentPetId);
    setLastError(QString());
    return true;
}

void AppController::quit()
{
    QApplication::quit();
}
