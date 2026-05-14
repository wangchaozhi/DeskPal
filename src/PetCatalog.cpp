#include "PetCatalog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
bool copyDirectory(const QString &sourcePath, const QString &targetPath)
{
    const QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
        return false;
    }

    QDir().mkpath(targetPath);

    const QFileInfoList entries =
        sourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &entry : entries) {
        const QString targetEntry = QDir(targetPath).filePath(entry.fileName());
        if (entry.isDir()) {
            if (!copyDirectory(entry.absoluteFilePath(), targetEntry)) {
                return false;
            }
        } else {
            QFile::remove(targetEntry);
            if (!QFile::copy(entry.absoluteFilePath(), targetEntry)) {
                return false;
            }
        }
    }

    return true;
}
} // namespace

PetCatalog::PetCatalog(QObject *parent)
    : QObject(parent)
{
    reload();
}

void PetCatalog::reload()
{
    m_pets.clear();
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

QString PetCatalog::writablePetsRoot() const
{
    const QString appPath = QCoreApplication::applicationDirPath();
    const QString preferred = QDir(appPath).filePath(QStringLiteral("assets/pets"));
    QDir().mkpath(preferred);
    return preferred;
}

bool PetCatalog::saveProfile(const PetProfile &pet, QString *error) const
{
    if (pet.id.isEmpty()) {
        if (error) {
            *error = tr("Cannot save a pet without an ID");
        }
        return false;
    }

    if (pet.basePath.isEmpty()) {
        if (error) {
            *error = tr("Built-in pets cannot be edited");
        }
        return false;
    }

    QJsonObject object;
    object.insert(QStringLiteral("id"), pet.id);
    object.insert(QStringLiteral("name"), pet.name);
    object.insert(QStringLiteral("type"), pet.type);
    object.insert(QStringLiteral("renderer"), pet.renderer);
    object.insert(QStringLiteral("source"), pet.source);

    if (pet.width > 0) {
        object.insert(QStringLiteral("width"), pet.width);
    }
    if (pet.height > 0) {
        object.insert(QStringLiteral("height"), pet.height);
    }
    if (pet.fps > 0) {
        object.insert(QStringLiteral("fps"), pet.fps);
    }
    if (pet.scale > 0.0 && pet.scale != 1.0) {
        object.insert(QStringLiteral("scale"), pet.scale);
    }

    QJsonObject actions;
    for (auto it = pet.actions.constBegin(); it != pet.actions.constEnd(); ++it) {
        actions.insert(it.key(), it.value());
    }
    object.insert(QStringLiteral("actions"), actions);

    if (!pet.animations.isEmpty()) {
        QJsonObject animations;
        for (auto it = pet.animations.constBegin(); it != pet.animations.constEnd(); ++it) {
            animations.insert(it.key(), it.value());
        }
        object.insert(QStringLiteral("animations"), animations);
    }

    if (!pet.idleActions.isEmpty()) {
        object.insert(QStringLiteral("idleActions"), QJsonArray::fromStringList(pet.idleActions));
    }

    if (!pet.speeches.isEmpty()) {
        object.insert(QStringLiteral("speeches"), QJsonArray::fromStringList(pet.speeches));
    }

    const QString profilePath = QDir(pet.basePath).filePath(QStringLiteral("pet.json"));
    QFile file(profilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) {
            *error = tr("Cannot write %1").arg(profilePath);
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool PetCatalog::importPetPack(const QString &sourceDir, QString *error)
{
    const QDir source(sourceDir);
    if (!source.exists()) {
        if (error) {
            *error = tr("Import folder does not exist");
        }
        return false;
    }

    if (!QFileInfo::exists(source.filePath(QStringLiteral("pet.json")))) {
        if (error) {
            *error = tr("Selected folder has no pet.json");
        }
        return false;
    }

    QString folderName = source.dirName();
    if (folderName.isEmpty()) {
        folderName = QStringLiteral("imported_pet");
    }

    QString targetPath = QDir(writablePetsRoot()).filePath(folderName);
    int suffix = 1;
    while (QFileInfo::exists(targetPath)) {
        targetPath = QDir(writablePetsRoot()).filePath(folderName + QStringLiteral("_%1").arg(suffix));
        ++suffix;
    }

    if (!copyDirectory(sourceDir, targetPath)) {
        if (error) {
            *error = tr("Failed to copy pet pack");
        }
        return false;
    }

    reload();
    return true;
}

bool PetCatalog::exportPetPack(const QString &petId, const QString &targetDir, QString *error) const
{
    const PetProfile pet = petById(petId);
    if (pet.basePath.isEmpty()) {
        if (error) {
            *error = tr("Built-in pets cannot be exported");
        }
        return false;
    }

    const QString targetPath = QDir(targetDir).filePath(pet.id);
    if (!copyDirectory(pet.basePath, targetPath)) {
        if (error) {
            *error = tr("Failed to export pet pack");
        }
        return false;
    }

    return true;
}

void PetCatalog::loadBuiltInPets()
{
    PetProfile classic;
    classic.id = QStringLiteral("classic_2d");
    classic.name = QStringLiteral("Default 2D");
    classic.type = QStringLiteral("2d");
    classic.renderer = QStringLiteral("qml");
    classic.source = QStringLiteral("PetBody.qml");
    classic.actions = {
        {QStringLiteral("idle"), QStringLiteral("idle")},
        {QStringLiteral("happy"), QStringLiteral("happy")},
        {QStringLiteral("sleepy"), QStringLiteral("sleepy")},
        {QStringLiteral("dragging"), QStringLiteral("dragging")},
    };
    classic.width = 220;
    classic.height = 250;
    addPet(classic);

    PetProfile cat = classic;
    cat.id = QStringLiteral("cat_2d");
    cat.name = QStringLiteral("Cat 2D");
    addPet(cat);

    PetProfile orb;
    orb.id = QStringLiteral("orb_3d");
    orb.name = QStringLiteral("Robot 3D");
    orb.type = QStringLiteral("3d");
    orb.renderer = QStringLiteral("quick3d");
    orb.source = QStringLiteral("PetModel3D.qml");
    orb.actions = {
        {QStringLiteral("idle"), QStringLiteral("idle")},
        {QStringLiteral("happy"), QStringLiteral("happy")},
        {QStringLiteral("sleepy"), QStringLiteral("sleepy")},
        {QStringLiteral("dragging"), QStringLiteral("dragging")},
    };
    orb.width = 240;
    orb.height = 260;
    addPet(orb);
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
    profile.width = object.value(QStringLiteral("width")).toInt(0);
    profile.height = object.value(QStringLiteral("height")).toInt(0);
    profile.fps = object.value(QStringLiteral("fps")).toInt(0);
    profile.scale = object.value(QStringLiteral("scale")).toDouble(1.0);
    if (profile.scale <= 0.0) {
        profile.scale = 1.0;
    }

    const QJsonObject actions = object.value(QStringLiteral("actions")).toObject();
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        profile.actions.insert(it.key(), it.value().toString());
    }

    const QJsonObject animations = object.value(QStringLiteral("animations")).toObject();
    for (auto it = animations.constBegin(); it != animations.constEnd(); ++it) {
        profile.animations.insert(it.key(), it.value().toString());
    }

    const QJsonArray idleActions = object.value(QStringLiteral("idleActions")).toArray();
    for (const QJsonValue &value : idleActions) {
        const QString action = value.toString().trimmed();
        if (!action.isEmpty()) {
            profile.idleActions.append(action);
        }
    }

    const QJsonArray speeches = object.value(QStringLiteral("speeches")).toArray();
    for (const QJsonValue &value : speeches) {
        const QString speech = value.toString().trimmed();
        if (!speech.isEmpty()) {
            profile.speeches.append(speech);
        }
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
