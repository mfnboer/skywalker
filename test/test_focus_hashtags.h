// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <focus_hashtags.h>
#include <post.h>
#include <atproto/lib/post_master.h>
#include <QtTest/QTest>

using namespace Skywalker;

class TestFocusHashTags : public QObject
{
    Q_OBJECT
public:
    struct TestFocus
    {
        QStringList mHashtags;
        QColor mColor;
    };

private slots:
    void matchPost_data()
    {
        QTest::addColumn<std::vector<TestFocus>>("focus");
        QTest::addColumn<QString>("text");
        QTest::addColumn<bool>("match");
        QTest::addColumn<QColor>("color");
        QTest::addColumn<std::set<QString>>("matchedTags");

        QTest::newRow("no focus hashtags")
            << std::vector<TestFocus>{}
            << "#hello #world"
            << false
            << QColor{}
            << std::set<QString>{};

        QTest::newRow("one focus hashtag 1")
            << std::vector<TestFocus>{ {{"hello"}, "blue"} }
            << "#hello #world"
            << true
            << QColor{"blue"}
            << std::set<QString>{ "hello" };

        QTest::newRow("one focus hashtag 2")
            << std::vector<TestFocus>{ {{"world"}, "blue"} }
            << "#hello #world"
            << true
            << QColor{"blue"}
            << std::set<QString>{ "world" };

        QTest::newRow("one focus hashtag no match")
            << std::vector<TestFocus>{ {{"bye"}, "blue"} }
            << "#hello #world"
            << false
            << QColor{}
            << std::set<QString>{};

        QTest::newRow("multi focus hashtag")
            << std::vector<TestFocus>{ {{"world"}, "blue"}, {{"hello"}, "yellow"} }
            << "#hello #world"
            << true
            << QColor{"yellow"}
            << std::set<QString>{ "hello", "world" };

        QTest::newRow("multi hashtag entry 1")
            << std::vector<TestFocus>{ {{"hello", "moon"}, "blue"} }
            << "#hello #world"
            << true
            << QColor{"blue"}
            << std::set<QString>{ "hello", "moon" };

        QTest::newRow("multi hashtag entry 2")
            << std::vector<TestFocus>{ {{"hello", "moon"}, "blue"}, {{"world", "order"}, "red"} }
            << "#hello #world"
            << true
            << QColor{"blue"}
            << std::set<QString>{ "hello", "moon", "order", "world" };


        QTest::newRow("normalization")
            << std::vector<TestFocus>{ {{"Hello"}, "blue"} }
            << "#HELLO #world"
            << true
            << QColor{"blue"}
            << std::set<QString>{ "hello" };
    }

    void matchPost()
    {
        QFETCH(std::vector<TestFocus>, focus);
        QFETCH(QString, text);
        QFETCH(bool, match);
        QFETCH(QColor, color);
        QFETCH(std::set<QString>, matchedTags);

        FocusHashtags focusHashtags;

        for (const auto& testFocus : focus)
        {
            auto* entry = new FocusHashtagEntry(this);

            for (const auto& tag : testFocus.mHashtags)
                entry->addHashtag(tag);

            entry->setHighlightColor(testFocus.mColor);
            focusHashtags.addEntry(entry);
        }

        const auto post = setPost(text);
        QCOMPARE(focusHashtags.match(post), match);

        if (color.isValid())
            QCOMPARE(focusHashtags.highlightColor(post), color);
        else
            QVERIFY(!focusHashtags.highlightColor(post).isValid());

        QCOMPARE(focusHashtags.getNormalizedMatchHashtags(post), matchedTags);
    }

    void addEntry()
    {
        const auto post = setPost("#World #order");

        FocusHashtags focusHashtags;
        focusHashtags.addEntry("hello");
        QVERIFY(!focusHashtags.match(post));

        focusHashtags.addEntry("world");
        QVERIFY(focusHashtags.match(post));
    }

    void removeEntry()
    {
        const auto post = setPost("#World #order");

        FocusHashtags focusHashtags;
        focusHashtags.addEntry("hello");
        focusHashtags.addEntry("world");
        QVERIFY(focusHashtags.match(post));

        const auto entries = focusHashtags.getMatchEntries(post);
        QCOMPARE(entries.size(), 1);

        focusHashtags.removeEntry(entries.first()->getId());
        QVERIFY(!focusHashtags.match(post));
    }

    void addToEntry()
    {
        const auto post = setPost("#World #order");

        FocusHashtags focusHashtags;
        focusHashtags.addEntry("hello");
        QVERIFY(!focusHashtags.match(post));

        const auto& entries = focusHashtags.getEntries();
        focusHashtags.addHashtagToEntry(entries.front(), "world");
        QVERIFY(focusHashtags.match(post));
    }

    void removeFromEntry()
    {
        const auto post = setPost("#World #order");

        FocusHashtags focusHashtags;
        focusHashtags.addEntry("hello");
        const auto& entries = focusHashtags.getEntries();
        focusHashtags.addHashtagToEntry(entries.front(), "world");
        QVERIFY(focusHashtags.match(post));

        focusHashtags.removeHashtagFromEntry(entries.front(), "world");
        QVERIFY(!focusHashtags.match(post));
    }

    void removeEmptyEntry()
    {
        FocusHashtags focusHashtags;
        focusHashtags.addEntry("hello");
        QCOMPARE(focusHashtags.getEntries().size(), 1);

        focusHashtags.removeHashtagFromEntry(focusHashtags.getEntries().front(), "hello");
        QCOMPARE(focusHashtags.getEntries().size(), 0);
    }

    void noDuplicates()
    {
        FocusHashtagEntry entry;
        entry.addHashtag("hello");
        QCOMPARE(entry.getHashtags().size(), 1);

        entry.addHashtag("world");
        QCOMPARE(entry.getHashtags().size(), 2);

        entry.addHashtag("Hello");
        QCOMPARE(entry.getHashtags().size(), 2);
    }

private:
    Post setPost(const QString& text)
    {
        ATProto::Client client(nullptr);
        ATProto::PostMaster pm(client);
        auto postView = std::make_shared<ATProto::AppBskyFeed::PostView>();

        pm.createPost(text, "", nullptr, [postView](auto&& postRecord){
            const auto json = postRecord->toJson();
            postView->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
            postView->mRecord = ATProto::AppBskyFeed::Record::Post::fromJson(json);
            postView->mAuthor = std::make_shared<ATProto::AppBskyActor::ProfileViewBasic>();
        });

        return Post(postView);
    }
};
