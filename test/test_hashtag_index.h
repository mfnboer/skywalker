// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <hashtag_index.h>
#include <QtTest/QTest>

using namespace Skywalker;

class TestHashTagIndex : public QObject
{
    Q_OBJECT
private slots:
    void findHashtag_data()
    {
        QTest::addColumn<std::vector<QString>>("cached");
        QTest::addColumn<QString>("hashtag");
        QTest::addColumn<QStringList>("found");

        QTest::newRow("no cached hashtags")
            << std::vector<QString>{}
            << "tag"
            << QStringList{};

        QTest::newRow("cached hashtag found")
            << std::vector<QString>{"tag"}
            << "tag"
            << QStringList{"tag"};

        QTest::newRow("cached hashtag not found")
            << std::vector<QString>{"hash"}
            << "tag"
            << QStringList{};

        QTest::newRow("cached multiple exact")
            << std::vector<QString>{"tag", "Tag", "TAG", "hash", "Hash"}
            << "Tag"
            << QStringList{"Tag", "TAG", "tag"};

        QTest::newRow("cached multiple prefix")
            << std::vector<QString>{"tag", "Tag", "TAG", "hash", "Hash"}
            << "Ta"
            << QStringList{"Tag", "TAG", "tag"};

        QTest::newRow("limit")
            << std::vector<QString>{"t1", "t2", "t3", "t4", "t5", "t6"}
            << "t"
            << QStringList{"t1", "t2", "t3", "t4", "t5"};

        QTest::newRow("cache overflow")
            << std::vector<QString>{"t01", "t02", "t03", "t04", "t05", "t06", "t07", "t08", "t09", "t10", "t11"}
            << "t"
            << QStringList{"t02", "t03", "t04", "t05", "t06"};

        QTest::newRow("insert hashtag twice")
            << std::vector<QString>{"tag", "tag"}
            << "ta"
            << QStringList{"tag"};
    }

    void findHashtag()
    {
        QFETCH(std::vector<QString>, cached);
        QFETCH(QString, hashtag);
        QFETCH(QStringList, found);

        HashtagIndex index(10);

        for (const auto& entry : cached)
            index.insert(entry);

        QCOMPARE(index.find(hashtag, 5), found);
    }

    void suppressTags()
    {
        HashtagIndex index(10);
        index.insert({ "Foo", "bar", "foobar" });
        QCOMPARE(index.find("foo", 10, { "Foo", "barf" }), { "foobar"} );
    }

    void clear()
    {
        HashtagIndex index(10);
        index.insert("tag");
        QCOMPARE(index.find("tag", 10), QStringList{"tag"});
        index.clear();
        QCOMPARE(index.find("tag", 10), QStringList{});
    }
};
