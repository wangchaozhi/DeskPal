#pragma once

#include <QObject>
#include <QPoint>

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
};
