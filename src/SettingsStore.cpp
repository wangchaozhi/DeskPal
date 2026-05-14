#include "SettingsStore.h"

#include <QCoreApplication>
#include <QSettings>

namespace {
constexpr auto kSettingsOrg = "DeskPal";
constexpr auto kSettingsApp = "DeskPal";
constexpr auto kWindowPosition = "window/position";
constexpr auto kAlwaysOnTop = "window/alwaysOnTop";
constexpr auto kLanguage = "ui/language";
constexpr auto kDefaultWindowPositionX = 120;
constexpr auto kDefaultWindowPositionY = 120;
constexpr auto kDefaultLanguage = "system";
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
