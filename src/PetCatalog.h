#pragma once

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

struct PetProfile
{
    QString id;
    QString name;
    QString type;
    QString renderer;
    QString source;
    QString basePath;
    QHash<QString, QString> actions;
    QHash<QString, QString> animations;
    QStringList idleActions;
    QStringList speeches;
    int width = 0;
    int height = 0;
    int fps = 0;
    qreal scale = 1.0;
};

class PetCatalog : public QObject
{
    Q_OBJECT

public:
    explicit PetCatalog(QObject *parent = nullptr);

    void reload();
    QVector<PetProfile> pets() const;
    PetProfile petById(const QString &id) const;
    QString defaultPetId() const;
    bool contains(const QString &id) const;

    QString writablePetsRoot() const;
    bool saveProfile(const PetProfile &pet, QString *error = nullptr) const;
    bool importPetPack(const QString &sourceDir, QString *error = nullptr);
    bool exportPetPack(const QString &petId, const QString &targetDir, QString *error = nullptr) const;
    bool createPet(const QString &id, const QString &name, const QString &type, QString *error = nullptr);
    bool deletePet(const QString &petId, QString *error = nullptr);

private:
    void loadBuiltInPets();
    void loadExternalPets();
    void addPet(const PetProfile &pet);
    PetProfile readProfile(const QString &profilePath) const;
    QVector<QString> petRootPaths() const;

    QVector<PetProfile> m_pets;
};
