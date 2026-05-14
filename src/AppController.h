#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class ActionController;
class PetCatalog;
class SettingsStore;
class TrayController;
class TranslationManager;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool alwaysOnTop READ alwaysOnTop WRITE setAlwaysOnTop NOTIFY alwaysOnTopChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString petAction READ petAction NOTIFY petActionChanged)
    Q_PROPERTY(QString currentPetId READ currentPetId WRITE setCurrentPetId NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetName READ currentPetName NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetType READ currentPetType NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetRenderer READ currentPetRenderer NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetSource READ currentPetSource NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetBasePath READ currentPetBasePath NOTIFY currentPetChanged)
    Q_PROPERTY(int currentPetWidth READ currentPetWidth NOTIFY currentPetChanged)
    Q_PROPERTY(int currentPetHeight READ currentPetHeight NOTIFY currentPetChanged)
    Q_PROPERTY(int currentPetFps READ currentPetFps NOTIFY currentPetChanged)
    Q_PROPERTY(qreal currentPetScale READ currentPetScale NOTIFY currentPetChanged)
    Q_PROPERTY(qreal petOpacity READ petOpacity WRITE setPetOpacity NOTIFY petOpacityChanged)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(bool wanderEnabled READ wanderEnabled WRITE setWanderEnabled NOTIFY wanderEnabledChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool enabled);
    QString language() const;
    void setLanguage(const QString &language);
    QString petAction() const;
    QString currentPetId() const;
    void setCurrentPetId(const QString &petId);
    QString currentPetName() const;
    QString currentPetType() const;
    QString currentPetRenderer() const;
    QString currentPetSource() const;
    QString currentPetBasePath() const;
    int currentPetWidth() const;
    int currentPetHeight() const;
    int currentPetFps() const;
    qreal currentPetScale() const;
    qreal petOpacity() const;
    void setPetOpacity(qreal opacity);
    bool autoStart() const;
    void setAutoStart(bool enabled);
    bool wanderEnabled() const;
    void setWanderEnabled(bool enabled);
    QString lastError() const;

    Q_INVOKABLE QPoint windowPosition() const;
    Q_INVOKABLE QRect availableGeometry(int x, int y) const;
    Q_INVOKABLE void saveWindowPosition(int x, int y);
    Q_INVOKABLE void resetWindowPosition();
    Q_INVOKABLE void showContextMenu();
    Q_INVOKABLE void showWindow();
    Q_INVOKABLE void hideWindow();
    Q_INVOKABLE void setPetVisible(bool visible);
    Q_INVOKABLE void triggerPetAction(const QString &action, int durationMs = 0);
    Q_INVOKABLE void setPetDragging(bool dragging);
    Q_INVOKABLE void setPetWalking(bool walking);
    Q_INVOKABLE QString currentPetActionSource(const QString &action) const;
    Q_INVOKABLE QString resolvePetResource(const QString &relativePath) const;
    Q_INVOKABLE QStringList currentPetFrameUrls(const QString &action) const;
    Q_INVOKABLE QString petActionSource(const QString &petId, const QString &action) const;
    Q_INVOKABLE QString resolvePetResourceForPet(const QString &petId, const QString &relativePath) const;
    Q_INVOKABLE QStringList petFrameUrls(const QString &petId, const QString &action) const;
    Q_INVOKABLE int petFps(const QString &petId) const;
    Q_INVOKABLE QString petAnimationClip(const QString &petId, const QString &action) const;
    Q_INVOKABLE QString randomSpeech() const;
    Q_INVOKABLE QVariantList petProfiles() const;
    Q_INVOKABLE QVariantList reloadPetProfiles();
    Q_INVOKABLE bool savePetProfile(const QVariantMap &profile);
    Q_INVOKABLE bool importPetPack(const QString &folderUrl);
    Q_INVOKABLE bool exportPetPack(const QString &petId, const QString &folderUrl);
    Q_INVOKABLE void quit();

signals:
    void alwaysOnTopChanged();
    void languageChanged();
    void petActionChanged();
    void currentPetChanged();
    void petOpacityChanged();
    void autoStartChanged();
    void wanderEnabledChanged();
    void lastErrorChanged();
    void showRequested();
    void hideRequested();
    void settingsRequested();
    void resetPositionRequested();

private:
    void applyPetToActionController();
    void setLastError(const QString &error);

    bool m_alwaysOnTop = true;
    QString m_currentPetId = QStringLiteral("classic_2d");
    QString m_lastError;
    SettingsStore *m_settings = nullptr;
    ActionController *m_actions = nullptr;
    PetCatalog *m_petCatalog = nullptr;
    TranslationManager *m_translations = nullptr;
    TrayController *m_tray = nullptr;
    bool m_petVisible = true;
};
