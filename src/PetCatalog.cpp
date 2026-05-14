#include "PetCatalog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

PetCatalog::PetCatalog(QObject *parent)
    : QObject(parent)
{
    loadBuiltInPets();
    loadExternalPets();
}

QVector<PetProfile> PetCatalog::pets() const
{
    return m_pets;
}

PetProfile PetCatalog::petById(const QString &id) const
{
    for (const PetProfile &pet : m_pets) {
        if (pet.id == id) {
            return pet;
        }
    }

    return m_pets.constFirst();
}

QString PetCatalog::defaultPetId() const
{
    return m_pets.constFirst().id;
}

bool PetCatalog::contains(const QString &id) const
{
    for (const PetProfile &pet : m_pets) {
        if (pet.id == id) {
            return true;
        }
    }

    return false;
}

void PetCatalog::loadBuiltInPets()
{
    addPet({
        QStringLiteral("classic_2d"),
        QStringLiteral("Default 2D"),
        QStringLiteral("2d"),
        QStringLiteral("qml"),
        QStringLiteral("PetBody.qml"),
        QString(),
        {
            {QStringLiteral("idle"), QStringLiteral("idle")},
            {QStringLiteral("happy"), QStringLiteral("happy")},
            {QStringLiteral("sleepy"), QStringLiteral("sleepy")},
            {QStringLiteral("dragging"), QStringLiteral("dragging")},
        },
    });

    addPet({
        QStringLiteral("cat_2d"),
        QStringLiteral("Cat 2D"),
        QStringLiteral("2d"),
        QStringLiteral("qml"),
        QStringLiteral("PetBody.qml"),
        QString(),
        {
            {QStringLiteral("idle"), QStringLiteral("idle")},
            {QStringLiteral("happy"), QStringLiteral("happy")},
            {QStringLiteral("sleepy"), QStringLiteral("sleepy")},
            {QStringLiteral("dragging"), QStringLiteral("dragging")},
        },
    });

    addPet({
        QStringLiteral("orb_3d"),
        QStringLiteral("Robot 3D"),
        QStringLiteral("3d"),
        QStringLiteral("quick3d"),
        QStringLiteral("PetModel3D.qml"),
        QString(),
        {
            {QStringLiteral("idle"), QStringLiteral("idle")},
            {QStringLiteral("happy"), QStringLiteral("happy")},
            {QStringLiteral("sleepy"), QStringLiteral("sleepy")},
            {QStringLiteral("dragging"), QStringLiteral("dragging")},
        },
    });
}

void PetCatalog::loadExternalPets()
{
    for (const QString &rootPath : petRootPaths()) {
        const QDir root(rootPath);
        if (!root.exists()) {
            continue;
        }

        const QFileInfoList entries = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries) {
            const QString profilePath = QDir(entry.absoluteFilePath()).filePath(QStringLiteral("pet.json"));
            const PetProfile profile = readProfile(profilePath);
            addPet(profile);
        }
    }
}

void PetCatalog::addPet(const PetProfile &pet)
{
    if (pet.id.isEmpty() || pet.name.isEmpty() || pet.type.isEmpty() || contains(pet.id)) {
        return;
    }

    m_pets.append(pet);
}

PetProfile PetCatalog::readProfile(const QString &profilePath) const
{
    QFile file(profilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return {};
    }

    const QJsonObject object = document.object();
    PetProfile profile;
    profile.id = object.value(QStringLiteral("id")).toString().trimmed();
    profile.name = object.value(QStringLiteral("name")).toString().trimmed();
    profile.type = object.value(QStringLiteral("type")).toString(QStringLiteral("2d")).toLower();
    profile.renderer = object.value(QStringLiteral("renderer")).toString(QStringLiteral("qml")).toLower();
    profile.source = object.value(QStringLiteral("source")).toString();
    profile.basePath = QFileInfo(profilePath).absolutePath();

    const QJsonObject actions = object.value(QStringLiteral("actions")).toObject();
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        profile.actions.insert(it.key(), it.value().toString());
    }

    if (profile.type != QStringLiteral("3d")) {
        profile.type = QStringLiteral("2d");
    }

    return profile;
}

QVector<QString> PetCatalog::petRootPaths() const
{
    const QString appPath = QCoreApplication::applicationDirPath();
    return {
        QDir(appPath).filePath(QStringLiteral("assets/pets")),
        QDir(appPath).filePath(QStringLiteral("../assets/pets")),
        QDir(QDir::currentPath()).filePath(QStringLiteral("assets/pets")),
    };
}
