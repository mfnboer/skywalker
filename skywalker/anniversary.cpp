// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "anniversary.h"

namespace Skywalker {

Anniversary::Anniversary(const QString& userDid, IUserSettings& userSettings, QObject* parent) :
    QObject(parent),
    mUserDid(userDid),
    mUserSettings(userSettings)
{
}

bool Anniversary::isAnniversary() const
{
    if (mFirstAppearance.isNull())
    {
        qWarning() << "First appearane unknown";
        return false;
    }

    const QDate firstDate = mFirstAppearance.date();
    const QDate thisDate = QDate::currentDate();

    if (thisDate.year() <= firstDate.year())
        return false;

    if (thisDate.month() != firstDate.month())
        return false;

    if (thisDate.day() == firstDate.day())
        return true;

    // Check for Feb 29 as first appearance
    if (thisDate.month() != 2)
        return false;

    if (firstDate.day() != 29)
        return false;

    if (thisDate.day() != 28)
        return false;

    // Today is Feb 28 and the first appearance day is Feb 29
    // If this year is a leap year, then the anniversary is Feb 29
    // Otherwise we celebrate at Feb 28
    return !QDate::isLeapYear(thisDate.year());
}

int Anniversary::getAnniversaryYears() const
{
    if (mFirstAppearance.isNull())
    {
        qWarning() << "First appearane unknown";
        return 0;
    }

    const int firstYear = mFirstAppearance.date().year();
    const int thisYear = QDate::currentDate().year();
    return thisYear - firstYear;
}

bool Anniversary::checkAnniversary() const
{
    if (isAnniversary())
    {
        qDebug() << "Today is anniversary!";
        const QDate noticeDate = mUserSettings.getAnniversaryNoticeDate(mUserDid);
        const QDate today = QDate::currentDate();

        if (noticeDate == today)
        {
            qDebug() << "Anniversary notice already given";
            return false;
        }

        mUserSettings.setAnniversaryNoticeDate(mUserDid, today);
        return true;
    }
    else
    {
        qDebug() << "No anniversary";
        return false;
    }
}

}
