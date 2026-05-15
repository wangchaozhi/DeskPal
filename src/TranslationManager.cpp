#include "TranslationManager.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent)
{
}

QString TranslationManager::language() const
{
    return m_language;
}

bool TranslationManager::installLanguage(const QString &language)
{
    QCoreApplication::removeTranslator(&m_appTranslator);
    QCoreApplication::removeTranslator(&m_qtTranslator);

    m_language = language.isEmpty() ? QStringLiteral("system") : language;

    const QString localeName = localeNameForLanguage(m_language);
    bool loaded = true;

    if (localeName != QStringLiteral("en")) {
        const QString translationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
        const QString qtBaseLocale = localeName.section(QLatin1Char('_'), 0, 0);
        loaded = m_qtTranslator.load(QStringLiteral("qt_%1").arg(qtBaseLocale), translationsPath);
        if (loaded) {
            QCoreApplication::installTranslator(&m_qtTranslator);
        }

        if (m_appTranslator.load(QStringLiteral(":/i18n/DeskPal_%1.qm").arg(localeName))) {
            QCoreApplication::installTranslator(&m_appTranslator);
        } else {
            loaded = false;
        }
    }

    return loaded;
}

QString TranslationManager::localeNameForLanguage(const QString &language) const
{
    if (language == QStringLiteral("zh_CN")) {
        return QStringLiteral("zh_CN");
    }

    if (language == QStringLiteral("ja_JP")) {
        return QStringLiteral("ja_JP");
    }

    if (language == QStringLiteral("en")) {
        return QStringLiteral("en");
    }

    const QString systemLocale = QLocale::system().name();
    if (systemLocale.startsWith(QStringLiteral("zh"))) {
        return QStringLiteral("zh_CN");
    }
    if (systemLocale.startsWith(QStringLiteral("ja"))) {
        return QStringLiteral("ja_JP");
    }
    return QStringLiteral("en");
}
