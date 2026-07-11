// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <expiry_cache.h>
#include <QtTest/QTest>

using namespace Skywalker;
using namespace std::chrono;

class TestExpiryCache : public QObject
{
    Q_OBJECT
private slots:

    void notExpired()
    {
        QVERIFY(!mCache.contains("foo"));
        QVERIFY(mCache.object("foo") == nullptr);
        QVERIFY(mCache["foo"] == nullptr);
        QVERIFY(mCache.getEntry("foo") == nullptr);

        mCache.insert("foo", new bool(false));
        QVERIFY(mCache.contains("foo"));
        QVERIFY(*mCache.object("foo") == false);
        QVERIFY(*mCache["foo"] == false);
        QVERIFY(*mCache.getEntry("foo")->mValue == false);

        QVERIFY(!mCache.contains("bar"));

        mCache.clear();
        QVERIFY(!mCache.contains("foo"));
        QVERIFY(mCache.object("foo") == nullptr);
        QVERIFY(mCache["foo"] == nullptr);

        auto timestamp = QDateTime::currentDateTime() - 59min;
        auto* entry = new ExpiryCache<QString, bool>::Entry;
        entry->mValue = new bool(true);
        entry->mTimestamp = timestamp;
        mCache.insertEntry("foo", entry);
        QVERIFY(mCache.contains("foo"));
        QVERIFY(*mCache.object("foo") == true);
        QVERIFY(*mCache["foo"] == true);
        QVERIFY(*mCache.getEntry("foo")->mValue == true);
        QVERIFY(mCache.getEntry("foo")->mTimestamp == timestamp);
    }

    void expired()
    {
        auto* entry = new ExpiryCache<QString, bool>::Entry;
        entry->mValue = new bool(true);
        entry->mTimestamp = QDateTime::currentDateTime() - 1h;
        mCache.insertEntry("bar", entry);
        QVERIFY(!mCache.contains("bar"));
        QVERIFY(mCache.object("bar") == nullptr);
        QVERIFY(mCache["bar"] == nullptr);

        mExpiredCache.insert("foo", new bool(false));
        QVERIFY(!mExpiredCache.contains("foo"));
        QVERIFY(mExpiredCache.object("foo") == nullptr);
        QVERIFY(mExpiredCache["foo"] == nullptr);
    }

    void take()
    {
        QVERIFY(mCache.take("foo") == nullptr);
        mCache.insert("foo", new bool(false));

        bool* value = mCache.take("foo");
        Q_ASSERT(value);
        QVERIFY(*value == false);

        delete value;
    }

    void cleanup()
    {
        mCache.clear();
        mExpiredCache.clear();
    }

private:
    ExpiryCache<QString, bool> mCache{1h};
    ExpiryCache<QString, bool> mExpiredCache{0min};
};
