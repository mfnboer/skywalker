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
        QTest::addColumn<std::vector<QString>>("found");

        QTest::newRow("no cached hashtags")
            << std::vector<QString>{}
            << "tag"
            << std::vector<QString>{};

        QTest::newRow("cached hashtag found")
            << std::vector<QString>{"tag"}
            << "tag"
            << std::vector<QString>{"tag"};

        QTest::newRow("cached hashtag not found")
            << std::vector<QString>{"hash"}
            << "tag"
            << std::vector<QString>{};

        QTest::newRow("cached multiple exact")
            << std::vector<QString>{"tag", "Tag", "TAG", "hash", "Hash"}
            << "Tag"
            << std::vector<QString>{"Tag", "TAG", "tag"};

        QTest::newRow("cached multiple prefix")
            << std::vector<QString>{"tag", "Tag", "TAG", "hash", "Hash"}
            << "Ta"
            << std::vector<QString>{"Tag", "TAG", "tag"};

        QTest::newRow("limit")
            << std::vector<QString>{"t1", "t2", "t3", "t4", "t5", "t6"}
            << "t"
            << std::vector<QString>{"t1", "t2", "t3", "t4", "t5"};

        QTest::newRow("cache overflow")
            << std::vector<QString>{"t01", "t02", "t03", "t04", "t05", "t06", "t07", "t08", "t09", "t10", "t11"}
            << "t"
            << std::vector<QString>{"t02", "t03", "t04", "t05", "t06"};
    }

    void findHashtag()
    {
        QFETCH(std::vector<QString>, cached);
        QFETCH(QString, hashtag);
        QFETCH(std::vector<QString>, found);

        HashtagIndex index(10);

        for (const auto& entry : cached)
            index.insert(entry);

        QCOMPARE(index.find(hashtag, 5), found);
    }
};
