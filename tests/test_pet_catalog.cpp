#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>

#include "PetCatalog.h"

class TestPetCatalog : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void hasBuiltInPets();
    void readExternalPet();
    void roundTripView3d();
    void roundTripPet2d();
    void legacyTopLevelFps();
    void availableSamplePetIds();

private:
    QTemporaryDir m_tempDir;
    QString writeJsonProfile(const QString &folderName, const QJsonObject &obj);
    PetCatalog *makeCatalog();
};

void TestPetCatalog::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    // Point applicationDirPath-style scans to our temp dir by setting the
    // working directory; PetCatalog falls back to currentPath/assets/pets.
    QDir().mkpath(m_tempDir.filePath(QStringLiteral("assets/pets")));
    QDir::setCurrent(m_tempDir.path());
}

QString TestPetCatalog::writeJsonProfile(const QString &folderName, const QJsonObject &obj)
{
    const QString dir = m_tempDir.filePath(QStringLiteral("assets/pets/") + folderName);
    QDir().mkpath(dir);
    const QString path = QDir(dir).filePath(QStringLiteral("pet.json"));
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return {};
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    file.close();
    return dir;
}

PetCatalog *TestPetCatalog::makeCatalog()
{
    auto *catalog = new PetCatalog(this);
    return catalog;
}

void TestPetCatalog::hasBuiltInPets()
{
    PetCatalog catalog;
    const auto pets = catalog.pets();
    QVERIFY(pets.size() >= 3);
    QVERIFY(catalog.contains(QStringLiteral("classic_2d")));
    QVERIFY(catalog.contains(QStringLiteral("orb_3d")));
}

void TestPetCatalog::readExternalPet()
{
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), QStringLiteral("test_pet"));
    obj.insert(QStringLiteral("name"), QStringLiteral("Test Pet"));
    obj.insert(QStringLiteral("type"), QStringLiteral("2d"));
    obj.insert(QStringLiteral("renderer"), QStringLiteral("svg"));
    obj.insert(QStringLiteral("source"), QStringLiteral("idle.svg"));
    QJsonObject actions;
    actions.insert(QStringLiteral("idle"), QStringLiteral("idle.svg"));
    obj.insert(QStringLiteral("actions"), actions);
    writeJsonProfile(QStringLiteral("test_pet"), obj);

    PetCatalog catalog;
    QVERIFY(catalog.contains(QStringLiteral("test_pet")));

    const PetProfile pet = catalog.petById(QStringLiteral("test_pet"));
    QCOMPARE(pet.name, QStringLiteral("Test Pet"));
    QCOMPARE(pet.type, QStringLiteral("2d"));
    QCOMPARE(pet.renderer, QStringLiteral("svg"));
    QCOMPARE(pet.source, QStringLiteral("idle.svg"));
    QCOMPARE(pet.actions.value(QStringLiteral("idle")), QStringLiteral("idle.svg"));
}

void TestPetCatalog::roundTripView3d()
{
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), QStringLiteral("rt_view3d"));
    obj.insert(QStringLiteral("name"), QStringLiteral("Round-trip view3d"));
    obj.insert(QStringLiteral("type"), QStringLiteral("3d"));
    obj.insert(QStringLiteral("renderer"), QStringLiteral("quick3d"));
    obj.insert(QStringLiteral("source"), QStringLiteral("Pet.qml"));
    writeJsonProfile(QStringLiteral("rt_view3d"), obj);

    PetCatalog catalog;
    QVERIFY(catalog.contains(QStringLiteral("rt_view3d")));

    PetProfile pet = catalog.petById(QStringLiteral("rt_view3d"));
    pet.view3d.cameraDistance = 500.0;
    pet.view3d.cameraHeight = 120.0;
    pet.view3d.modelRotationY = 45.0;
    pet.view3d.lightBrightness = 2.0;

    QString error;
    QVERIFY2(catalog.saveProfile(pet, &error), qPrintable(error));

    catalog.reload();
    const PetProfile reloaded = catalog.petById(QStringLiteral("rt_view3d"));
    QCOMPARE(reloaded.view3d.cameraDistance, 500.0);
    QCOMPARE(reloaded.view3d.cameraHeight, 120.0);
    QCOMPARE(reloaded.view3d.modelRotationY, 45.0);
    QCOMPARE(reloaded.view3d.lightBrightness, 2.0);
}

void TestPetCatalog::roundTripPet2d()
{
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), QStringLiteral("rt_pet2d"));
    obj.insert(QStringLiteral("name"), QStringLiteral("Round-trip pet2d"));
    obj.insert(QStringLiteral("type"), QStringLiteral("2d"));
    obj.insert(QStringLiteral("renderer"), QStringLiteral("png-sequence"));
    obj.insert(QStringLiteral("source"), QStringLiteral("idle"));
    QJsonObject pet2d;
    pet2d.insert(QStringLiteral("fps"), 24);
    obj.insert(QStringLiteral("pet2d"), pet2d);
    writeJsonProfile(QStringLiteral("rt_pet2d"), obj);

    PetCatalog catalog;
    QVERIFY(catalog.contains(QStringLiteral("rt_pet2d")));
    const PetProfile pet = catalog.petById(QStringLiteral("rt_pet2d"));
    QCOMPARE(pet.pet2d.fps, 24);
}

void TestPetCatalog::legacyTopLevelFps()
{
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), QStringLiteral("legacy_fps"));
    obj.insert(QStringLiteral("name"), QStringLiteral("Legacy FPS"));
    obj.insert(QStringLiteral("type"), QStringLiteral("2d"));
    obj.insert(QStringLiteral("renderer"), QStringLiteral("png-sequence"));
    obj.insert(QStringLiteral("source"), QStringLiteral("idle"));
    obj.insert(QStringLiteral("fps"), 18);
    writeJsonProfile(QStringLiteral("legacy_fps"), obj);

    PetCatalog catalog;
    const PetProfile pet = catalog.petById(QStringLiteral("legacy_fps"));
    QCOMPARE(pet.pet2d.fps, 18);
}

void TestPetCatalog::availableSamplePetIds()
{
    PetCatalog catalog;
    const QStringList samples = catalog.availableSamplePets();
    QVERIFY(samples.contains(QStringLiteral("sample_svg_2d")));
    QVERIFY(samples.contains(QStringLiteral("sample_quick3d")));
}

QTEST_GUILESS_MAIN(TestPetCatalog)
#include "test_pet_catalog.moc"
