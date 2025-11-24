// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <anniversary.h>
#include <QtTest/QTest>

using namespace Skywalker;
using namespace std::chrono;

class TestAnniversary : public QObject
{
    Q_OBJECT
private slots:
    void isAnniversary_data()
    {
        QTest::addColumn<QDateTime>("firstAppearance");
        QTest::addColumn<QDate>("today");
        QTest::addColumn<bool>("anniversary");

        const QDateTime today{QDate{2024, 5, 17}, {}};

        QTest::newRow("unkown") << today << QDate{} << false;
        QTest::newRow("today") << today << today.date() << false;
        QTest::newRow("yesterday") << today.addDays(-1) << today.date() << false;
        QTest::newRow("tomorrow") << today.addDays(-1) << today.date() << false;
        QTest::newRow("1 year") << today.addYears(-1) << today.date() << true;
        QTest::newRow("2 year") << today.addYears(-2) << today.date() << true;

        const QDateTime feb29{{2020, 2, 29}, {}};

        QTest::newRow("feb29 4 year") << feb29 << QDate{2024, 2, 29} << true;
        QTest::newRow("feb29 4 year feb 28") << feb29 << QDate{2024, 2, 28} << false;
        QTest::newRow("feb29 3 year") << feb29 << QDate{2023, 2, 28} << true;
        QTest::newRow("feb29 2 year") << feb29 << QDate{2022, 2, 28} << true;
        QTest::newRow("feb29 1 year") << feb29 << QDate{2021, 2, 28} << true;
        QTest::newRow("feb29 0 year") << feb29 << QDate{2020, 2, 29} << false;
    }

    void isAnniversary()
    {
        QFETCH(QDateTime, firstAppearance);
        QFETCH(QDate, today);
        QFETCH(bool, anniversary);

        mAnniversary.setFirstAppearance(firstAppearance);
        QCOMPARE(mAnniversary.isAnniversary(today), anniversary);
    }

    void anniversaryYears_data()
    {
        QTest::addColumn<QDateTime>("firstAppearance");
        QTest::addColumn<QDate>("today");
        QTest::addColumn<int>("years");

        const QDateTime today{QDate{2024, 5, 17}, {}};

        QTest::newRow("1 year") << today.addYears(-1) << today.date() << 1;
        QTest::newRow("2 year") << today.addYears(-2) << today.date() << 2;

        const QDateTime feb29{{2020, 2, 29}, {}};

        QTest::newRow("feb29 4 year") << feb29 << QDate{2024, 2, 29} << 4;
        QTest::newRow("feb29 3 year") << feb29 << QDate{2023, 2, 28} << 3;
        QTest::newRow("feb29 2 year") << feb29 << QDate{2022, 2, 28} << 2;
        QTest::newRow("feb29 1 year") << feb29 << QDate{2021, 2, 28} << 1;
    }

    void anniversaryYears()
    {
        QFETCH(QDateTime, firstAppearance);
        QFETCH(QDate, today);
        QFETCH(int, years);

        mAnniversary.setFirstAppearance(firstAppearance);
        QCOMPARE(mAnniversary.getAnniversaryYears(today), years);
    }

    void checkAnniversary()
    {
        mAnniversary.setFirstAppearance({{2023, 5, 17}, {}});

        QVERIFY(!mAnniversary.checkAnniversary({2024, 5, 16}));
        QVERIFY(mAnniversary.checkAnniversary({2024, 5, 17}));
        QVERIFY(!mAnniversary.checkAnniversary({2024, 5, 17})); // notice already given

        QVERIFY(!mAnniversary.checkAnniversary({2025, 5, 16}));
        QVERIFY(mAnniversary.checkAnniversary({2025, 5, 17}));
        QVERIFY(!mAnniversary.checkAnniversary({2025, 5, 17})); // notice already given

    }

private:
    class MockUserSettings : public IUserSettingsAnniversary
    {
    public:
        QDate getAnniversaryNoticeDate(const QString&) const override { return mAnniversaryNoticeDate; }
        void setAnniversaryNoticeDate(const QString&, QDate date) override { mAnniversaryNoticeDate = date; }

    private:
        QDate mAnniversaryNoticeDate;
    };

    QString mUserDid = "did:test";
    MockUserSettings mUserSettings;
    Anniversary mAnniversary{mUserDid, mUserSettings};
};
