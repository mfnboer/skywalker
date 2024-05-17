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

    Q_INVOKABLE bool isAnniversary(QDate date = QDate::currentDate()) const;
    Q_INVOKABLE int getAnniversaryYears(QDate date = QDate::currentDate()) const;
    bool checkAnniversary(QDate date = QDate::currentDate()) const;
    void setFirstAppearance(QDateTime firstAppearance) { mFirstAppearance = firstAppearance; }

private:
    const QString& mUserDid;
    IUserSettings& mUserSettings;
    QDateTime mFirstAppearance;
};

}
