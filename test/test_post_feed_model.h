// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <post_feed_model.h>
#include <QTest>

using namespace Skywalker;

class TestPostFeedModel : public QObject
{
    Q_OBJECT
private slots:
    void setFeed()
    {
        int newTopIndex = mPostFeedModel.setFeed(getFeed(1, TEST_DATE));
        QCOMPARE(newTopIndex, -1);
        QCOMPARE(mPostFeedModel.rowCount(), 1);
        QVERIFY(mPostFeedModel.getLastCursor().isEmpty());
        QCOMPARE(mPostFeedModel.lastTimestamp(), TEST_DATE);
    }

    void clearFeed()
    {
        mPostFeedModel.setFeed(getFeed(1, TEST_DATE));
        QCOMPARE(mPostFeedModel.rowCount(), 1);
        mPostFeedModel.clear();
        QCOMPARE(mPostFeedModel.rowCount(), 0);
    }

    void refreshFeedWithSame()
    {
        int newTopIndex = mPostFeedModel.setFeed(getFeed(1, TEST_DATE));
        QCOMPARE(newTopIndex, -1);
        QCOMPARE(mPostFeedModel.rowCount(), 1);
        newTopIndex = mPostFeedModel.setFeed(getFeed(1, TEST_DATE));
        QCOMPARE(newTopIndex, 0);
        QCOMPARE(mPostFeedModel.rowCount(), 1);
    }

    void addToEmptyFeed()
    {
        mPostFeedModel.addFeed(getFeed(1, TEST_DATE));
        QCOMPARE(mPostFeedModel.rowCount(), 1);
        QVERIFY(mPostFeedModel.getLastCursor().isEmpty());
        QCOMPARE(mPostFeedModel.lastTimestamp(), TEST_DATE);
    }

    void prependToEmptyFeed()
    {
        int gapId = mPostFeedModel.prependFeed(getFeed(1, TEST_DATE));
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel.rowCount(), 1);
        QVERIFY(mPostFeedModel.getLastCursor().isEmpty());
        QCOMPARE(mPostFeedModel.lastTimestamp(), TEST_DATE);
    }

private:
    static constexpr char const* POST_TEMPLATE = R"##({
        "post": {
            "uri": "at://did:plc:foo/app.bsky.feed.post/r%1",
            "cid": "cid%1",
            "author": {
                "did": "did:plc:foo",
                "handle": "foo.bsky.social"
            },
            "record": {
                "$type": "app.bsky.feed.post",
                "text": "Hello world!",
                "createdAt": "%2"
            },
            "indexedAt": "%2"
        }
    })##";

    const QDateTime TEST_DATE = QDateTime::fromString("2023-11-20T18:46:00.000Z", Qt::ISODateWithMs);

    ATProto::AppBskyFeed::OutputFeed::Ptr getFeed(int numPosts, QDateTime startTime)
    {
        using namespace std::chrono_literals;
        QString feedData = R"###({ "feed": [)###";

        for (int i = 1; i <= numPosts; ++i)
        {
            auto postTime = startTime + (i-1) * 1s;
            QString postData = QString(POST_TEMPLATE).arg(QString::number(i),
                                                          postTime.toString(Qt::ISODateWithMs));
            feedData += postData;

            if (i < numPosts)
                feedData += ',';
        }

        feedData += "]}";

        return getFeed(feedData.toUtf8());
    }

    ATProto::AppBskyFeed::OutputFeed::Ptr getFeed(const char* data)
    {
        QJsonParseError error;
        auto json = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError)
            qWarning() << "Failed to parse json:" << error.errorString() << "offset:" << error.offset;

        return ATProto::AppBskyFeed::OutputFeed::fromJson(json);
    }

    QString mUserDid;
    ProfileStore mFollowing;
    ATProto::UserPreferences mUserPreferences;
    ContentFilter mContentFilter{mUserPreferences};
    Bookmarks mBookmarks;
    PostFeedModel mPostFeedModel{mUserDid, mFollowing, mContentFilter, mBookmarks, mUserPreferences};
};

QTEST_MAIN(TestPostFeedModel)
