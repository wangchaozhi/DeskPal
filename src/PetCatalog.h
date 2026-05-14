#pragma once

#include <QObject>
#include <QHash>
#include <QString>
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

private:
    void loadBuiltInPets();
    void loadExternalPets();
    void addPet(const PetProfile &pet);
    PetProfile readProfile(const QString &profilePath) const;
    QVector<QString> petRootPaths() const;

    QVector<PetProfile> m_pets;
};
