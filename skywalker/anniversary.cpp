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

bool Anniversary::isAnniversary(QDate date) const
{
    if (mFirstAppearance.isNull())
    {
        qWarning() << "First appearane unknown";
        return false;
    }

    const QDate firstDate = mFirstAppearance.toLocalTime().date();

    if (date.year() <= firstDate.year())
        return false;

    if (date.month() != firstDate.month())
        return false;

    if (date.day() == firstDate.day())
        return true;

    // Check for Feb 29 as first appearance
    if (date.month() != 2)
        return false;

    if (firstDate.day() != 29)
        return false;

    if (date.day() != 28)
        return false;

    // Today is Feb 28 and the first appearance day is Feb 29
    // If this year is a leap year, then the anniversary is Feb 29
    // Otherwise we celebrate at Feb 28
    return !QDate::isLeapYear(date.year());
}

int Anniversary::getAnniversaryYears(QDate date) const
{
    if (mFirstAppearance.isNull())
    {
        qWarning() << "First appearane unknown";
        return 0;
    }

    const int firstYear = mFirstAppearance.toLocalTime().date().year();
    const int thisYear = date.year();
    return thisYear - firstYear;
}

bool Anniversary::checkAnniversary(QDate date) const
{
    if (isAnniversary(date))
    {
        qDebug() << "Today is anniversary!";
        const QDate noticeDate = mUserSettings.getAnniversaryNoticeDate(mUserDid);

        if (noticeDate == date)
        {
            qDebug() << "Anniversary notice already given";
            return false;
        }

        mUserSettings.setAnniversaryNoticeDate(mUserDid, date);
        return true;
    }
    else
    {
        qDebug() << "No anniversary";
        return false;
    }
}

}
