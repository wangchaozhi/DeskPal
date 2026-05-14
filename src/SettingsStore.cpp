#include "SettingsStore.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

namespace {
constexpr auto kSettingsOrg = "DeskPal";
constexpr auto kSettingsApp = "DeskPal";
constexpr auto kWindowPosition = "window/position";
constexpr auto kAlwaysOnTop = "window/alwaysOnTop";
constexpr auto kLanguage = "ui/language";
constexpr auto kRenderMode = "pet/renderMode";
constexpr auto kCurrentPetId = "pet/currentPetId";
constexpr auto kPetOpacity = "pet/opacity";
constexpr auto kDefaultWindowPositionX = 120;
constexpr auto kDefaultWindowPositionY = 120;
constexpr auto kDefaultLanguage = "system";
constexpr auto kDefaultRenderMode = "2d";
constexpr auto kDefaultPetId = "classic_2d";
constexpr auto kDefaultPetOpacity = 1.0;

#ifdef Q_OS_WIN
constexpr auto kRunKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr auto kRunValueName = "DeskPal";
#endif
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
    return settings.value(kWindowPosition, defaultWindowPosition()).toPoint();
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

QPoint SettingsStore::defaultWindowPosition() const
{
    return QPoint(kDefaultWindowPositionX, kDefaultWindowPositionY);
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

qreal SettingsStore::petOpacity() const
{
    QSettings settings;
    const qreal opacity = settings.value(kPetOpacity, kDefaultPetOpacity).toReal();
    if (opacity < 0.2) {
        return 0.2;
    }
    if (opacity > 1.0) {
        return 1.0;
    }
    return opacity;
}

void SettingsStore::setPetOpacity(qreal opacity)
{
    QSettings settings;
    settings.setValue(kPetOpacity, opacity);
}

bool SettingsStore::autoStartEnabled() const
{
#ifdef Q_OS_WIN
    QSettings runKeys(QString::fromLatin1(kRunKey), QSettings::NativeFormat);
    return runKeys.contains(QString::fromLatin1(kRunValueName));
#else
    return false;
#endif
}

void SettingsStore::setAutoStartEnabled(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings runKeys(QString::fromLatin1(kRunKey), QSettings::NativeFormat);
    if (enabled) {
        const QString executable = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        runKeys.setValue(QString::fromLatin1(kRunValueName), QStringLiteral("\"%1\"").arg(executable));
    } else {
        runKeys.remove(QString::fromLatin1(kRunValueName));
    }
#else
    Q_UNUSED(enabled);
#endif
}
