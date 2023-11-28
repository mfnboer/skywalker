// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <muted_words.h>
#include <QTest>

using namespace Skywalker;

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
    }

    void matchPost()
    {
        QFETCH(std::vector<QString>, muted);
        QFETCH(QString, text);
        QFETCH(bool, match);

        MutedWords mutedWords;

        for (const auto& entry : muted)
            mutedWords.addEntry(entry);

        const auto post = setPost(text);
        QCOMPARE(mutedWords.match(post), match);
    }

    void remove()
    {
        MutedWords mutedWords;
        mutedWords.addEntry("the quick brown fox");
        mutedWords.addEntry("hello");
        mutedWords.addEntry("hello");
        mutedWords.addEntry("world");
        mutedWords.addEntry("yellow fox");

        mutedWords.removeEntry(2);
        mutedWords.removeEntry(0);

        auto post = setPost("hello darkness");
        QVERIFY(mutedWords.match(post));

        post = setPost("dark world");
        QVERIFY(mutedWords.match(post));

        post = setPost("the quick yellow fox jumps");
        QVERIFY(mutedWords.match(post));

        post = setPost("the quick brown fox jumps");
        QVERIFY(!mutedWords.match(post));
    }

private:
    Post setPost(const QString& text)
    {
        mPostView.mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
        auto postRecord = std::make_unique<ATProto::AppBskyFeed::Record::Post>();
        postRecord->mText = text;
        mPostView.mRecord = std::move(postRecord);
        return Post(&mPostView, 0);
    }

    ATProto::AppBskyFeed::PostView mPostView;
};

QTEST_MAIN(TestMutedWords)
