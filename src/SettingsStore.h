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
    QPoint defaultWindowPosition() const;

    QString language() const;
    void setLanguage(const QString &language);

    QString currentPetId() const;
    void setCurrentPetId(const QString &petId);

    qreal petOpacity() const;
    void setPetOpacity(qreal opacity);

    bool autoStartEnabled() const;
    void setAutoStartEnabled(bool enabled);

    bool wanderEnabled() const;
    void setWanderEnabled(bool enabled);
};
