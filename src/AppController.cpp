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

QVariantMap view3dToMap(const PetView3D &view)
{
    QVariantMap map;
    map.insert(QStringLiteral("cameraDistance"), view.cameraDistance);
    map.insert(QStringLiteral("cameraHeight"), view.cameraHeight);
    map.insert(QStringLiteral("cameraPitch"), view.cameraPitch);
    map.insert(QStringLiteral("modelRotationX"), view.modelRotationX);
    map.insert(QStringLiteral("modelRotationY"), view.modelRotationY);
    map.insert(QStringLiteral("modelRotationZ"), view.modelRotationZ);
    map.insert(QStringLiteral("modelPositionX"), view.modelPositionX);
    map.insert(QStringLiteral("modelPositionY"), view.modelPositionY);
    map.insert(QStringLiteral("modelPositionZ"), view.modelPositionZ);
    map.insert(QStringLiteral("lightBrightness"), view.lightBrightness);
    map.insert(QStringLiteral("lightPitch"), view.lightPitch);
    map.insert(QStringLiteral("lightYaw"), view.lightYaw);
    return map;
}

QStringList expectedExtensionsFor(const QString &renderer)
{
    if (renderer == QStringLiteral("qml")) {
        return {QStringLiteral("qml")};
    }
    if (renderer == QStringLiteral("svg")) {
        return {QStringLiteral("svg")};
    }
    if (renderer == QStringLiteral("gif")) {
        return {QStringLiteral("gif")};
    }
    if (renderer == QStringLiteral("apng")) {
        return {QStringLiteral("png"), QStringLiteral("apng")};
    }
    if (renderer == QStringLiteral("glb")) {
        return {QStringLiteral("glb")};
    }
    if (renderer == QStringLiteral("gltf")) {
        return {QStringLiteral("gltf")};
    }
    if (renderer == QStringLiteral("quick3d")) {
        return {QStringLiteral("qml")};
    }
    return {};
}

bool extensionMatches(const QString &fileName, const QStringList &allowed)
{
    if (allowed.isEmpty()) {
        return true;
    }
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    return allowed.contains(suffix);
}

struct PetIssue {
    bool isError;
    QString message;
};

using IssueList = QVector<PetIssue>;

const QStringList &expectedActionList()
{
    static const QStringList kActions = {
        QStringLiteral("idle"),
        QStringLiteral("happy"),
        QStringLiteral("sleepy"),
        QStringLiteral("dragging"),
    };
    return kActions;
}

void validateMeta(const PetProfile &pet, IssueList &out)
{
    auto err = [&](const QString &m) { out.push_back({true, m}); };
    auto warn = [&](const QString &m) { out.push_back({false, m}); };

    if (pet.id.isEmpty()) {
        err(QObject::tr("Missing pet ID"));
    }
    if (pet.name.isEmpty()) {
        err(QObject::tr("Missing pet name"));
    }
    if (pet.type != QStringLiteral("2d") && pet.type != QStringLiteral("3d")) {
        err(QObject::tr("Invalid pet type"));
    }
    if (pet.renderer.isEmpty()) {
        err(QObject::tr("Missing renderer"));
    }

    if (pet.width != 0 && (pet.width < 32 || pet.width > 4000)) {
        err(QObject::tr("Width must be between 32 and 4000"));
    }
    if (pet.height != 0 && (pet.height < 32 || pet.height > 4000)) {
        err(QObject::tr("Height must be between 32 and 4000"));
    }
    if (pet.scale <= 0.0 || pet.scale > 10.0) {
        err(QObject::tr("Scale must be between 0.1 and 10"));
    }
    if (pet.renderer == QStringLiteral("png-sequence") && pet.pet2d.fps <= 0) {
        warn(QObject::tr("Frame rate not set for png-sequence"));
    }
}

void validateAssets(const PetProfile &pet, IssueList &out)
{
    auto err = [&](const QString &m) { out.push_back({true, m}); };
    auto warn = [&](const QString &m) { out.push_back({false, m}); };

    const QStringList expectedExtensions = expectedExtensionsFor(pet.renderer);
    const bool needsSourceFile = pet.renderer != QStringLiteral("png-sequence");

    if (pet.source.isEmpty()) {
        warn(QObject::tr("Source not configured"));
    } else if (needsSourceFile) {
        if (!QFileInfo::exists(petResourcePath(pet, pet.source))) {
            err(QObject::tr("Source file not found: %1").arg(pet.source));
        } else if (!extensionMatches(pet.source, expectedExtensions)) {
            err(QObject::tr("Source extension does not match renderer %1")
                    .arg(pet.renderer));
        }
    }

    if (pet.renderer == QStringLiteral("png-sequence")) {
        const QString sourcePath = petResourcePath(pet, pet.source);
        if (!pet.source.isEmpty() && !hasImageFrames(sourcePath)) {
            err(QObject::tr("No frames in source directory"));
        }
    }

    for (const QString &action : expectedActionList()) {
        const QString actionSource = pet.actions.value(action);
        if (actionSource.isEmpty()) {
            warn(QObject::tr("Action not configured: %1").arg(action));
            continue;
        }

        const QString actionPath = petResourcePath(pet, actionSource);
        if (pet.renderer == QStringLiteral("png-sequence")) {
            if (!hasImageFrames(actionPath)) {
                err(QObject::tr("No frames for action: %1").arg(action));
            }
        } else if (!QFileInfo::exists(actionPath)) {
            err(QObject::tr("Action resource not found: %1").arg(action));
        } else if (!extensionMatches(actionSource, expectedExtensions)) {
            err(QObject::tr("Action %1 extension does not match renderer %2")
                    .arg(action, pet.renderer));
        }
    }
}

void validateReferences(const PetProfile &pet, IssueList &out)
{
    auto warn = [&](const QString &m) { out.push_back({false, m}); };
    const QStringList &actions = expectedActionList();

    for (auto it = pet.animations.constBegin(); it != pet.animations.constEnd(); ++it) {
        if (!actions.contains(it.key())) {
            warn(QObject::tr("Animation clip references unknown action: %1").arg(it.key()));
        }
    }

    for (const QString &idleAction : pet.idleActions) {
        if (!actions.contains(idleAction)) {
            warn(QObject::tr("Idle action references unknown action: %1").arg(idleAction));
        }
    }
}

IssueList validatePet(const PetProfile &pet)
{
    IssueList issues;
    validateMeta(pet, issues);
    if (!pet.basePath.isEmpty()) {
        validateAssets(pet, issues);
    }
    validateReferences(pet, issues);
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
    m_actions->setNightSleepyEnabled(m_settings->nightSleepyEnabled());
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
    connect(m_tray, &TrayController::actionTriggered, this, [this](const QString &action) {
        if (action == QStringLiteral("dragging")) {
            m_actions->triggerAction(action, 1500);
        } else if (action == QStringLiteral("idle")) {
            m_actions->triggerAction(action, 0);
        } else {
            m_actions->triggerAction(action, 1800);
        }
    });
    connect(m_actions, &ActionController::currentActionChanged, this, &AppController::petActionChanged);

    applyPetToActionController();
}

AppController::~AppController() = default;

void AppController::applyPetToActionController()
{
    const PetProfile pet = m_petCatalog->petById(m_currentPetId);
    m_actions->setIdleActions(pet.idleActions);

    QStringList actionNames = pet.actions.keys();
    actionNames.sort();
    if (m_tray) {
        m_tray->setActions(actionNames);
    }
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
    return m_petCatalog->petById(m_currentPetId).pet2d.fps;
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

bool AppController::nightSleepyEnabled() const
{
    return m_settings->nightSleepyEnabled();
}

void AppController::setNightSleepyEnabled(bool enabled)
{
    if (m_settings->nightSleepyEnabled() == enabled) {
        return;
    }

    m_settings->setNightSleepyEnabled(enabled);
    m_actions->setNightSleepyEnabled(enabled);
    emit nightSleepyEnabledChanged();
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
    return m_petCatalog->petById(petId).pet2d.fps;
}

QString AppController::petAnimationClip(const QString &petId, const QString &action) const
{
    return m_petCatalog->petById(petId).animations.value(action);
}

QVariantMap AppController::petView3d(const QString &petId) const
{
    if (petId == m_liveView3dPetId && !m_liveView3d.isEmpty()) {
        return m_liveView3d;
    }
    return view3dToMap(m_petCatalog->petById(petId).view3d);
}

QVariantMap AppController::currentLiveView3d() const
{
    QVariantMap map = m_liveView3d;
    map.insert(QStringLiteral("petId"), m_liveView3dPetId);
    return map;
}

void AppController::setLiveView3d(const QString &petId, const QVariantMap &view)
{
    if (m_liveView3dPetId == petId && m_liveView3d == view) {
        return;
    }
    m_liveView3dPetId = petId;
    m_liveView3d = view;
    emit liveView3dChanged();
}

void AppController::clearLiveView3d()
{
    if (m_liveView3dPetId.isEmpty() && m_liveView3d.isEmpty()) {
        return;
    }
    m_liveView3dPetId.clear();
    m_liveView3d.clear();
    emit liveView3dChanged();
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
        const QVector<PetIssue> issues = validatePet(pet);
        QStringList warnings;
        QStringList errors;
        for (const PetIssue &issue : issues) {
            if (issue.isError) {
                errors.append(issue.message);
            } else {
                warnings.append(issue.message);
            }
        }
        QVariantMap profile;
        profile.insert(QStringLiteral("id"), pet.id);
        profile.insert(QStringLiteral("name"), pet.name);
        profile.insert(QStringLiteral("type"), pet.type);
        profile.insert(QStringLiteral("renderer"), pet.renderer);
        profile.insert(QStringLiteral("source"), pet.source);
        profile.insert(QStringLiteral("basePath"), pet.basePath);
        profile.insert(QStringLiteral("width"), pet.width);
        profile.insert(QStringLiteral("height"), pet.height);
        QVariantMap pet2dMap;
        pet2dMap.insert(QStringLiteral("fps"), pet.pet2d.fps);
        profile.insert(QStringLiteral("pet2d"), pet2dMap);
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
        profile.insert(QStringLiteral("view3d"), view3dToMap(pet.view3d));

        profile.insert(QStringLiteral("isValid"), errors.isEmpty());
        profile.insert(QStringLiteral("warnings"), warnings);
        profile.insert(QStringLiteral("errors"), errors);

        QStringList combined;
        for (const QString &error : errors) {
            combined.append(QStringLiteral("● ") + error);
        }
        for (const QString &warning : warnings) {
            combined.append(QStringLiteral("○ ") + warning);
        }
        profile.insert(QStringLiteral("issueText"),
                       combined.isEmpty() ? tr("Ready") : combined.join(QStringLiteral("\n")));
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
    if (profile.contains(QStringLiteral("pet2d"))) {
        const QVariantMap pet2dMap = profile.value(QStringLiteral("pet2d")).toMap();
        if (pet2dMap.contains(QStringLiteral("fps"))) {
            pet.pet2d.fps = pet2dMap.value(QStringLiteral("fps")).toInt();
        }
    } else if (profile.contains(QStringLiteral("fps"))) {
        pet.pet2d.fps = profile.value(QStringLiteral("fps")).toInt();
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
    if (profile.contains(QStringLiteral("view3d"))) {
        const QVariantMap view = profile.value(QStringLiteral("view3d")).toMap();
        pet.view3d.cameraDistance =
            view.value(QStringLiteral("cameraDistance"), pet.view3d.cameraDistance).toReal();
        pet.view3d.cameraHeight =
            view.value(QStringLiteral("cameraHeight"), pet.view3d.cameraHeight).toReal();
        pet.view3d.cameraPitch =
            view.value(QStringLiteral("cameraPitch"), pet.view3d.cameraPitch).toReal();
        pet.view3d.modelRotationX =
            view.value(QStringLiteral("modelRotationX"), pet.view3d.modelRotationX).toReal();
        pet.view3d.modelRotationY =
            view.value(QStringLiteral("modelRotationY"), pet.view3d.modelRotationY).toReal();
        pet.view3d.modelRotationZ =
            view.value(QStringLiteral("modelRotationZ"), pet.view3d.modelRotationZ).toReal();
        pet.view3d.modelPositionX =
            view.value(QStringLiteral("modelPositionX"), pet.view3d.modelPositionX).toReal();
        pet.view3d.modelPositionY =
            view.value(QStringLiteral("modelPositionY"), pet.view3d.modelPositionY).toReal();
        pet.view3d.modelPositionZ =
            view.value(QStringLiteral("modelPositionZ"), pet.view3d.modelPositionZ).toReal();
        pet.view3d.lightBrightness =
            view.value(QStringLiteral("lightBrightness"), pet.view3d.lightBrightness).toReal();
        pet.view3d.lightPitch =
            view.value(QStringLiteral("lightPitch"), pet.view3d.lightPitch).toReal();
        pet.view3d.lightYaw =
            view.value(QStringLiteral("lightYaw"), pet.view3d.lightYaw).toReal();
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

QStringList AppController::availableSamplePets() const
{
    return m_petCatalog->availableSamplePets();
}

QString AppController::importPetAsset(const QString &petId, const QString &fileUrl)
{
    if (!m_petCatalog->contains(petId)) {
        setLastError(tr("Unknown pet"));
        return QString();
    }

    const QString localPath = QUrl(fileUrl).isLocalFile()
            ? QUrl(fileUrl).toLocalFile()
            : fileUrl;

    QString error;
    const QString relPath = m_petCatalog->copyAssetIntoPet(petId, localPath, &error);
    if (relPath.isEmpty()) {
        setLastError(error);
        return QString();
    }

    setLastError(QString());
    return relPath;
}

bool AppController::installSamplePet(const QString &sampleId)
{
    QString error;
    if (!m_petCatalog->installSamplePet(sampleId, &error)) {
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

QRect AppController::settingsWindowGeometry() const
{
    return m_settings->settingsWindowGeometry();
}

void AppController::saveSettingsWindowGeometry(int x, int y, int w, int h)
{
    m_settings->setSettingsWindowGeometry(QRect(x, y, w, h));
}

void AppController::quit()
{
    QApplication::quit();
}
