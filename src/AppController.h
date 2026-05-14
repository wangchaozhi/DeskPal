#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QStringList>
#include <QVariantList>

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
    Q_PROPERTY(QString renderMode READ renderMode WRITE setRenderMode NOTIFY renderModeChanged)
    Q_PROPERTY(QString currentPetId READ currentPetId WRITE setCurrentPetId NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetName READ currentPetName NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetType READ currentPetType NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetRenderer READ currentPetRenderer NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetSource READ currentPetSource NOTIFY currentPetChanged)
    Q_PROPERTY(QString currentPetBasePath READ currentPetBasePath NOTIFY currentPetChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool enabled);
    QString language() const;
    void setLanguage(const QString &language);
    QString petAction() const;
    QString renderMode() const;
    void setRenderMode(const QString &renderMode);
    QString currentPetId() const;
    void setCurrentPetId(const QString &petId);
    QString currentPetName() const;
    QString currentPetType() const;
    QString currentPetRenderer() const;
    QString currentPetSource() const;
    QString currentPetBasePath() const;

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
    Q_INVOKABLE QString currentPetActionSource(const QString &action) const;
    Q_INVOKABLE QString resolvePetResource(const QString &relativePath) const;
    Q_INVOKABLE QStringList currentPetFrameUrls(const QString &action) const;
    Q_INVOKABLE QString petActionSource(const QString &petId, const QString &action) const;
    Q_INVOKABLE QString resolvePetResourceForPet(const QString &petId, const QString &relativePath) const;
    Q_INVOKABLE QStringList petFrameUrls(const QString &petId, const QString &action) const;
    Q_INVOKABLE QVariantList petProfiles() const;
    Q_INVOKABLE QVariantList reloadPetProfiles();
    Q_INVOKABLE void quit();

signals:
    void alwaysOnTopChanged();
    void languageChanged();
    void petActionChanged();
    void renderModeChanged();
    void currentPetChanged();
    void showRequested();
    void hideRequested();
    void settingsRequested();
    void resetPositionRequested();

private:
    bool m_alwaysOnTop = true;
    QString m_renderMode = QStringLiteral("2d");
    QString m_currentPetId = QStringLiteral("classic_2d");
    SettingsStore *m_settings = nullptr;
    ActionController *m_actions = nullptr;
    PetCatalog *m_petCatalog = nullptr;
    TranslationManager *m_translations = nullptr;
    TrayController *m_tray = nullptr;
    bool m_petVisible = true;
};
