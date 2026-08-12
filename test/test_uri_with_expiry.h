// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <uri_with_expiry.h>
#include <QtTest/QTest>

using namespace Skywalker;

class TestUriWithExpiry : public QObject
{
    Q_OBJECT
private slots:
    void empty()
    {
        UriWithExpirySet uriSet;
        QCOMPARE(uriSet.getFirstExpiry(), nullptr);
    }

    void getUriWithExpiry()
    {
        UriWithExpirySet uriSet;
        const UriWithExpiry foo("foo", QDateTime{QDate{2024, 5, 17}, {}}, true, false);
        const UriWithExpiry bar("bar", QDateTime{QDate{2024, 5, 16}, {}}, false, true);
        uriSet.insert(foo);
        uriSet.insert(bar);

        const auto* got1 = uriSet.get("foo");
        Q_ASSERT(got1);
        QCOMPARE(got1->getUri(), "foo");
        QCOMPARE(got1->getExpiry(), (QDateTime{QDate{2024, 5, 17}, {}}));
        QVERIFY(got1->isOnlyReposts());
        QVERIFY(!got1->isOnlyQuotePosts());

        const auto* got2 = uriSet.get("bar");
        Q_ASSERT(got2);
        QCOMPARE(got2->getUri(), "bar");
        QCOMPARE(got2->getExpiry(), (QDateTime{QDate{2024, 5, 16}, {}}));
        QVERIFY(!got2->isOnlyReposts());
        QVERIFY(got2->isOnlyQuotePosts());

        const auto* got3 = uriSet.get("foobar");
        QVERIFY(!got3);
    }

    void insertSingle()
    {
        UriWithExpirySet uriSet;
        const UriWithExpiry foo("foo", QDateTime{QDate{2024, 5, 17}, {}}, false, false);
        uriSet.insert(foo);
        QCOMPARE(*uriSet.getFirstExpiry(), foo);
        QCOMPARE(uriSet.getExpiry("foo"), foo.getExpiry());
        QVERIFY(!uriSet.getExpiry("bar").isValid());
    }

    void insertMultiple()
    {
        UriWithExpirySet uriSet;
        const UriWithExpiry foo("foo", QDateTime{QDate{2024, 5, 17}, {}}, false, false);
        const UriWithExpiry bar("bar", QDateTime{QDate{2024, 5, 16}, {}}, false, false);
        const UriWithExpiry foobar("foobar", QDateTime{QDate{2024, 5, 18}, {}}, false, false);
        uriSet.insert(foo);
        uriSet.insert(bar);
        uriSet.insert(foobar);
        QCOMPARE(*uriSet.getFirstExpiry(), bar);
        QCOMPARE(uriSet.getExpiry("foo"), foo.getExpiry());
        QCOMPARE(uriSet.getExpiry("bar"), bar.getExpiry());
        QCOMPARE(uriSet.getExpiry("foobar"), foobar.getExpiry());
        QVERIFY(!uriSet.getExpiry("sky").isValid());
    }

    void insertDuplicate()
    {
        UriWithExpirySet uriSet;
        const UriWithExpiry foo1("foo", QDateTime{QDate{2024, 5, 17}, {}}, false, false);
        const UriWithExpiry foo2("foo", QDateTime{QDate{2024, 5, 18}, {}}, false, false);
        uriSet.insert(foo1);
        uriSet.insert(foo2);
        QCOMPARE(*uriSet.getFirstExpiry(), foo2);
        QCOMPARE(uriSet.getExpiry("foo"), foo2.getExpiry());

        uriSet.remove("foo");
        QCOMPARE(uriSet.getFirstExpiry(), nullptr);
    }

    void clear()
    {
        UriWithExpirySet uriSet;
        const UriWithExpiry foo("foo", QDateTime{QDate{2024, 5, 17}, {}}, false, false);
        uriSet.insert(foo);
        uriSet.clear();
        QCOMPARE(uriSet.getFirstExpiry(), nullptr);
        QVERIFY(!uriSet.getExpiry("foo").isValid());
    }

    void remove()
    {
        UriWithExpirySet uriSet;
        const UriWithExpiry foo("foo", QDateTime{QDate{2024, 5, 17}, {}}, false, false);
        const UriWithExpiry bar("zbar", QDateTime{QDate{2024, 5, 16}, {}}, false, false);
        const UriWithExpiry foobar("foobar", QDateTime{QDate{2024, 5, 18}, {}}, false, false);
        uriSet.insert(foo);
        uriSet.insert(bar);
        uriSet.insert(foobar);
        QCOMPARE(*uriSet.getFirstExpiry(), bar);
        QCOMPARE(uriSet.getExpiry("zbar"), bar.getExpiry());
        QCOMPARE(uriSet.getExpiry("foo"), foo.getExpiry());
        QCOMPARE(uriSet.getExpiry("foobar"), foobar.getExpiry());

        QVERIFY(uriSet.remove("zbar"));
        QCOMPARE(*uriSet.getFirstExpiry(), foo);
        QVERIFY(!uriSet.getExpiry("zbar").isValid());
        QCOMPARE(uriSet.getExpiry("foo"), foo.getExpiry());
        QCOMPARE(uriSet.getExpiry("foobar"), foobar.getExpiry());

        QVERIFY(uriSet.remove("foo"));
        QCOMPARE(*uriSet.getFirstExpiry(), foobar);
        QVERIFY(!uriSet.getExpiry("zbar").isValid());
        QVERIFY(!uriSet.getExpiry("foo").isValid());
        QCOMPARE(uriSet.getExpiry("foobar"), foobar.getExpiry());

        QVERIFY(uriSet.remove("foobar"));
        QCOMPARE(uriSet.getFirstExpiry(), nullptr);
        QVERIFY(!uriSet.getExpiry("zbar").isValid());
        QVERIFY(!uriSet.getExpiry("foo").isValid());
        QVERIFY(!uriSet.getExpiry("foobar").isValid());
        QVERIFY(!uriSet.remove("foobar"));
    }

    void removeNonExisting()
    {
        UriWithExpirySet uriSet;
        const UriWithExpiry foo("foo", QDateTime{QDate{2024, 5, 17}, {}}, false, false);
        uriSet.insert(foo);
        QCOMPARE(*uriSet.getFirstExpiry(), foo);
        QCOMPARE(uriSet.getExpiry("foo"), foo.getExpiry());

        QVERIFY(!uriSet.remove("bar"));
        QCOMPARE(*uriSet.getFirstExpiry(), foo);
        QCOMPARE(uriSet.getExpiry("foo"), foo.getExpiry());
    }

    void serialization()
    {
        UriWithExpirySet uriSet;
        const UriWithExpiry foo("foo", QDateTime{QDate{2024, 5, 17}, {}}, false, false);
        const UriWithExpiry bar("bar", QDateTime{QDate{2024, 5, 16}, {}}, false, false);
        const UriWithExpiry foobar("foobar", QDateTime{QDate{2024, 5, 18}, {}}, false, false);
        uriSet.insert(foo);
        uriSet.insert(bar);
        uriSet.insert(foobar);

        const auto json = uriSet.toJson();
        const UriWithExpiry sky("sky", QDateTime{QDate{2024, 5, 1}, {}}, false, false);
        uriSet.insert(sky);
        QCOMPARE(*uriSet.getFirstExpiry(), sky);

        uriSet.fromJson(json);
        QCOMPARE(*uriSet.getFirstExpiry(), bar);
    }
};
