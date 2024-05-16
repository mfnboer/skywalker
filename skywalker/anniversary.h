// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "user_settings.h"
#include <QDateTime>
#include <QObject>

namespace Skywalker {

class Anniversary : public QObject
{
    Q_OBJECT

public:
    explicit Anniversary(const QString& userDid, IUserSettings& userSettings, QObject* parent = nullptr);

    Q_INVOKABLE bool isAnniversary() const;
    Q_INVOKABLE int getAnniversaryYears() const;
    bool checkAnniversary() const;
    void setFirstAppearance(QDateTime firstAppearance) { mFirstAppearance = firstAppearance; }

private:
    const QString& mUserDid;
    IUserSettings& mUserSettings;
    QDateTime mFirstAppearance;
};

}
