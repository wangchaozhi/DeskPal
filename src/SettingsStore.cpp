#include "SettingsStore.h"

#include <QCoreApplication>
#include <QSettings>

namespace {
constexpr auto kSettingsOrg = "DeskPal";
constexpr auto kSettingsApp = "DeskPal";
constexpr auto kWindowPosition = "window/position";
constexpr auto kAlwaysOnTop = "window/alwaysOnTop";
constexpr auto kLanguage = "ui/language";
constexpr auto kRenderMode = "pet/renderMode";
constexpr auto kCurrentPetId = "pet/currentPetId";
constexpr auto kDefaultWindowPositionX = 120;
constexpr auto kDefaultWindowPositionY = 120;
constexpr auto kDefaultLanguage = "system";
constexpr auto kDefaultRenderMode = "2d";
constexpr auto kDefaultPetId = "classic_2d";
} // namespace

SettingsStore::SettingsStore(QObject *parent)
    : QObject(parent)
{
    QCoreApplication::setOrganizationName(kSettingsOrg);
    QCoreApplication::setApplicationName(kSettingsApp);
}

bool SettingsStore::alwaysOnTop() const
{
    QSettings settings;
    return settings.value(kAlwaysOnTop, true).toBool();
}

void SettingsStore::setAlwaysOnTop(bool enabled)
{
    QSettings settings;
    settings.setValue(kAlwaysOnTop, enabled);
}

QPoint SettingsStore::windowPosition() const
{
    QSettings settings;
    return settings.value(kWindowPosition, QPoint(kDefaultWindowPositionX, kDefaultWindowPositionY)).toPoint();
}

void SettingsStore::setWindowPosition(const QPoint &position)
{
    QSettings settings;
    settings.setValue(kWindowPosition, position);
}

void SettingsStore::resetWindowPosition()
{
    QSettings settings;
    settings.remove(kWindowPosition);
}

QString SettingsStore::language() const
{
    QSettings settings;
    return settings.value(kLanguage, QString::fromLatin1(kDefaultLanguage)).toString();
}

void SettingsStore::setLanguage(const QString &language)
{
    QSettings settings;
    settings.setValue(kLanguage, language);
}

QString SettingsStore::renderMode() const
{
    QSettings settings;
    return settings.value(kRenderMode, QString::fromLatin1(kDefaultRenderMode)).toString();
}

void SettingsStore::setRenderMode(const QString &renderMode)
{
    QSettings settings;
    settings.setValue(kRenderMode, renderMode);
}

QString SettingsStore::currentPetId() const
{
    QSettings settings;
    return settings.value(kCurrentPetId, QString::fromLatin1(kDefaultPetId)).toString();
}

void SettingsStore::setCurrentPetId(const QString &petId)
{
    QSettings settings;
    settings.setValue(kCurrentPetId, petId);
}
