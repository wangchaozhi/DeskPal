#pragma once

#include <QObject>
#include <QPoint>
#include <QString>

class SettingsStore : public QObject
{
    Q_OBJECT

public:
    explicit SettingsStore(QObject *parent = nullptr);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool enabled);

    QPoint windowPosition() const;
    void setWindowPosition(const QPoint &position);
    void resetWindowPosition();

    QString language() const;
    void setLanguage(const QString &language);

    QString renderMode() const;
    void setRenderMode(const QString &renderMode);

    QString currentPetId() const;
    void setCurrentPetId(const QString &petId);
};
