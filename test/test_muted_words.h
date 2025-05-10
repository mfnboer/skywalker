// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <muted_words.h>
#include <post.h>
#include <atproto/lib/post_master.h>
#include <QtTest/QTest>

using namespace Skywalker;
using namespace std::chrono_literals;

class TestMutedWords : public QObject
{
    Q_OBJECT
private slots:
    void matchPost_data()
    {
        QTest::addColumn<std::vector<QString>>("muted");
        QTest::addColumn<QString>("text");
        QTest::addColumn<bool>("match");

        QTest::newRow("no muted words")
            << std::vector<QString>{}
            << "Hello world!"
            << false;

        QTest::newRow("single muted word match 1")
            << std::vector<QString>{"hello"}
            << "Hello world!"
            << true;

        QTest::newRow("single muted word match 2")
            << std::vector<QString>{"world"}
            << "Hello world!"
            << true;

        QTest::newRow("single muted word no match")
            << std::vector<QString>{"bye"}
            << "Hello world!"
            << false;

        QTest::newRow("mutiple muted words match")
            << std::vector<QString>{"BYE", "WORLD"}
            << "Hello world!"
            << true;

        QTest::newRow("mutiple muted words no match")
            << std::vector<QString>{"Good", "Bad", "Ugly"}
            << "Hello world!"
            << false;

        QTest::newRow("muti word match")
            << std::vector<QString>{"Beautiful World"}
            << "Hello beautiful world!"
            << true;

        QTest::newRow("muti word no match")
            << std::vector<QString>{"Hello world"}
            << "Hello beautiful world!"
            << false;

        QTest::newRow("muti word repetition")
            << std::vector<QString>{"the quick brown fox"}
            << "The quick, The quick, The quick brown fox jumps!"
            << true;

        QTest::newRow("muti word newline")
            << std::vector<QString>{"the quick brown fox"}
            << "The quick, The quick, The quick\nbrown fox jumps!"
            << true;

        QTest::newRow("hyphen")
            << std::vector<QString>{"hello-world"}
            << "Hello world!"
            << true;

        QTest::newRow("only words")
            << std::vector<QString>{"!"}
            << "Hello world!"
            << false;

        QTest::newRow("hashtag match")
            << std::vector<QString>{"#sky"}
            << "The #sky is blue."
            << true;

        QTest::newRow("hashtag no match")
            << std::vector<QString>{"#sky"}
            << "The sky is blue."
            << false;

        QTest::newRow("hashtag mutiple facet tags 1")
            << std::vector<QString>{"#sky", "#blue"}
            << "Hello blue #sky #walker"
            << true;

        QTest::newRow("hashtag mutiple facet tags 2")
            << std::vector<QString>{"#blue", "#walker"}
            << "Hello blue #sky #walker"
            << true;

        QTest::newRow("keycap is not a hashtag")
            << std::vector<QString>{"#sky"}
            << "The #️⃣sky is blue."
            << false;

        QTest::newRow("partial link")
            << std::vector<QString>{"thereforeiam.eu"}
            << "Test www.thereforeiam.eu link match."
            << true;

        QTest::newRow("tld")
            << std::vector<QString>{".eu"}
            << "Test www.thereforeiam.eu link match."
            << true;

        QTest::newRow("tld no match")
            << std::vector<QString>{".eu"}
            << "Test eu should not match to the TLD."
            << false;

        QTest::newRow("partial link path")
            << std::vector<QString>{"thereforeiam.eu"}
            << "Test www.thereforeiam.eu/skywalker link match."
            << true;

        QTest::newRow("full link")
            << std::vector<QString>{"www.thereforeiam.eu"}
            << "Test www.thereforeiam.eu link match."
            << true;

        QTest::newRow("half link")
            << std::vector<QString>{"thereforeiam.eu"}
            << "Test ithinkthereforeiam.eu link match."
            << false;

        QTest::newRow("uppercase link 1")
            << std::vector<QString>{"thereforeiam.eu"}
            << "Test WWW.THEREFOREIAM.EU link match."
            << true;

        QTest::newRow("uppercase link 2")
            << std::vector<QString>{"THEREFOREIAM.EU"}
            << "Test www.thereforeiam.eu link match."
            << true;

        QTest::newRow("multiple links 1")
            << std::vector<QString>{"thereforeiam.eu", "example.com"}
            << "Test www.thereforeiam.eu link match."
            << true;

        QTest::newRow("multiple links 2")
            << std::vector<QString>{"thereforeiam.eu", "muted.example.com"}
            << "Test www.example.com link match."
            << false;

        QTest::newRow("multiple links 3")
            << std::vector<QString>{"thereforeiam.eu", "muted.example.com"}
            << "Test www.example.com link match. muted.example.com/test.html"
            << true;
    }

    void matchPost()
    {
        QFETCH(std::vector<QString>, muted);
        QFETCH(QString, text);
        QFETCH(bool, match);

        ProfileStore userFollows;
        MutedWords mutedWords{userFollows};

        for (const auto& entry : muted)
            mutedWords.addEntry(entry);

        const auto post = setPost(text);
        QCOMPARE(mutedWords.match(post), match);
    }

    void excludeFollows()
    {
        ProfileStore userFollows;
        userFollows.add(BasicProfile{"did:plc:foo", "@foo.thereforeiam.eu", "Foo", ""});
        MutedWords mutedWords{userFollows};
        mutedWords.addEntry("hello", QEnums::ACTOR_TARGET_EXCLUDE_FOLLOWING);
        const auto post = setPost("hello world");
        QVERIFY(mutedWords.match(post));

        userFollows.add(BasicProfile{"did:plc:test", "@test.thereforeiam.eu", "Test", ""});
        QVERIFY(!mutedWords.match(post));
    }

    void expires()
    {
        const auto now = QDateTime::currentDateTimeUtc();
        ProfileStore userFollows;
        MutedWords mutedWords{userFollows};
        mutedWords.addEntry("hello", QEnums::ACTOR_TARGET_ALL, now + 10s);
        mutedWords.addEntry("world", QEnums::ACTOR_TARGET_ALL, now - 10s);

        const auto post1 = setPost("hello");
        QVERIFY(mutedWords.match(post1));

        const auto post2 = setPost("world");
        QVERIFY(!mutedWords.match(post2));
    }

    void remove()
    {
        ProfileStore userFollows;
        MutedWords mutedWords{userFollows};
        mutedWords.addEntry("the quick brown fox");
        mutedWords.addEntry("hello");
        mutedWords.addEntry("hello!");
        mutedWords.addEntry("world");
        mutedWords.addEntry("yellow fox");

        mutedWords.removeEntry("hello!");
        mutedWords.removeEntry("the quick brown fox");

        auto post = setPost("hello darkness");
        QVERIFY(mutedWords.match(post));

        post = setPost("dark world");
        QVERIFY(mutedWords.match(post));

        post = setPost("the quick yellow fox jumps");
        QVERIFY(mutedWords.match(post));

        post = setPost("the quick brown fox jumps");
        QVERIFY(!mutedWords.match(post));
    }

    void entriesChanged()
    {
        int changeCount = 0;
        ProfileStore userFollows;
        MutedWords mutedWords{userFollows};
        connect(&mutedWords, &MutedWords::entriesChanged, this, [&changeCount]{ ++changeCount; });

        mutedWords.addEntry("Skywalker");
        QCOMPARE(changeCount, 1);

        mutedWords.removeEntry("Skywalker");
        QCOMPARE(changeCount, 2);

        mutedWords.addEntry("Skywalker");
        QCOMPARE(changeCount, 3);

        mutedWords.removeEntry("sky");
        QCOMPARE(changeCount, 3);

        mutedWords.removeEntry("Skywalker");
        QCOMPARE(changeCount, 4);
    }

    void sortedEntries()
    {
        ProfileStore userFollows;
        MutedWords mutedWords{userFollows};
        mutedWords.addEntry("walker");
        mutedWords.addEntry("sky");
        mutedWords.addEntry("sky walker");

        const MutedWordEntry::List entries = mutedWords.getEntries();
        QStringList entryValues;

        for (const auto& entry : entries)
            entryValues.push_back(entry.getValue());

        const QStringList expected{ { "sky", "sky walker", "walker" } };
        QCOMPARE(entryValues, expected);
    }

    void clear()
    {
        int changeCount = 0;
        ProfileStore userFollows;
        MutedWords mutedWords{userFollows};
        connect(&mutedWords, &MutedWords::entriesChanged, this, [&changeCount]{ ++changeCount; });

        mutedWords.addEntry("sky");
        QCOMPARE(changeCount, 1);
        QCOMPARE(mutedWords.getEntries().size(), 1);

        mutedWords.clear();
        QCOMPARE(changeCount, 2);
        QCOMPARE(mutedWords.getEntries().size(), 0);
    }

private:
    Post setPost(const QString& text)
    {
        ATProto::Client client(nullptr);
        ATProto::PostMaster pm(client);
        auto postView = std::make_shared<ATProto::AppBskyFeed::PostView>();

        pm.createPost(text, "", nullptr, {}, [postView](auto&& postRecord){
            const auto json = postRecord->toJson();
            postView->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
            postView->mRecord = ATProto::AppBskyFeed::Record::Post::fromJson(json);
            postView->mAuthor = std::make_unique<ATProto::AppBskyActor::ProfileViewBasic>();
            postView->mAuthor->mDid = "did:plc:test";
        });

        return Post(postView);
    }
};
