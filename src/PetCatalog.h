#pragma once

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

struct PetView3D
{
    qreal cameraDistance = 360.0;
    qreal cameraHeight = 90.0;
    qreal cameraPitch = -12.0;
    qreal modelRotationX = 0.0;
    qreal modelRotationY = 0.0;
    qreal modelRotationZ = 0.0;
    qreal modelPositionX = 0.0;
    qreal modelPositionY = 0.0;
    qreal modelPositionZ = 0.0;
    qreal lightBrightness = 1.4;
    qreal lightPitch = -38.0;
    qreal lightYaw = 28.0;
};

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
    PetView3D view3d;
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

    QStringList availableSamplePets() const;
    bool installSamplePet(const QString &sampleId, QString *error = nullptr);

private:
    void loadBuiltInPets();
    void loadExternalPets();
    void addPet(const PetProfile &pet);
    PetProfile readProfile(const QString &profilePath) const;
    QVector<QString> petRootPaths() const;

    QVector<PetProfile> m_pets;
};
