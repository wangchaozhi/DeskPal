#include "SettingsStore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

namespace {
constexpr auto kSettingsOrg = "DeskPal";
constexpr auto kSettingsApp = "DeskPal";
constexpr auto kWindowPosition = "window/position";
constexpr auto kAlwaysOnTop = "window/alwaysOnTop";
constexpr auto kLanguage = "ui/language";
constexpr auto kCurrentPetId = "pet/currentPetId";
constexpr auto kPetOpacity = "pet/opacity";
constexpr auto kWanderEnabled = "pet/wander";
constexpr auto kDefaultWindowPositionX = 120;
constexpr auto kDefaultWindowPositionY = 120;
constexpr auto kDefaultLanguage = "system";
constexpr auto kDefaultPetId = "classic_2d";
constexpr auto kDefaultPetOpacity = 1.0;

#ifdef Q_OS_WIN
constexpr auto kRunKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr auto kRunValueName = "DeskPal";
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
QString autoStartFilePath()
{
#if defined(Q_OS_MACOS)
    return QDir::homePath() + QStringLiteral("/Library/LaunchAgents/com.deskpal.autostart.plist");
#else
    QString configDir = qEnvironmentVariable("XDG_CONFIG_HOME");
    if (configDir.isEmpty()) {
        configDir = QDir::homePath() + QStringLiteral("/.config");
    }
    return configDir + QStringLiteral("/autostart/DeskPal.desktop");
#endif
}

QString autoStartFileContent()
{
    const QString executable = QCoreApplication::applicationFilePath();
#if defined(Q_OS_MACOS)
    return QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
        "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
        "<plist version=\"1.0\">\n"
        "<dict>\n"
        "    <key>Label</key>\n"
        "    <string>com.deskpal.autostart</string>\n"
        "    <key>ProgramArguments</key>\n"
        "    <array>\n"
        "        <string>%1</string>\n"
        "    </array>\n"
        "    <key>RunAtLoad</key>\n"
        "    <true/>\n"
        "</dict>\n"
        "</plist>\n").arg(executable);
#else
    return QStringLiteral(
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=DeskPal\n"
        "Exec=\"%1\"\n"
        "Terminal=false\n"
        "X-GNOME-Autostart-enabled=true\n").arg(executable);
#endif
}
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

bool SettingsStore::wanderEnabled() const
{
    QSettings settings;
    return settings.value(kWanderEnabled, true).toBool();
}

void SettingsStore::setWanderEnabled(bool enabled)
{
    QSettings settings;
    settings.setValue(kWanderEnabled, enabled);
}

bool SettingsStore::autoStartEnabled() const
{
#if defined(Q_OS_WIN)
    QSettings runKeys(QString::fromLatin1(kRunKey), QSettings::NativeFormat);
    return runKeys.contains(QString::fromLatin1(kRunValueName));
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    return QFileInfo::exists(autoStartFilePath());
#else
    return false;
#endif
}

void SettingsStore::setAutoStartEnabled(bool enabled)
{
#if defined(Q_OS_WIN)
    QSettings runKeys(QString::fromLatin1(kRunKey), QSettings::NativeFormat);
    if (enabled) {
        const QString executable = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        runKeys.setValue(QString::fromLatin1(kRunValueName), QStringLiteral("\"%1\"").arg(executable));
    } else {
        runKeys.remove(QString::fromLatin1(kRunValueName));
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    const QString path = autoStartFilePath();
    if (enabled) {
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(autoStartFileContent().toUtf8());
            file.close();
        }
    } else {
        QFile::remove(path);
    }
#else
    Q_UNUSED(enabled);
#endif
}
