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
        int newTopIndex = mPostFeedModel.setFeed(getFeed(POST_FEED_SINGLE_POST));
        QCOMPARE(newTopIndex, -1);
        QCOMPARE(mPostFeedModel.rowCount(), 1);
        QVERIFY(mPostFeedModel.getLastCursor().isEmpty());
        QCOMPARE(mPostFeedModel.lastTimestamp(), QDateTime::fromString("2023-11-20T18:46:00.000Z", Qt::ISODateWithMs));
    }

    void clearFeed()
    {
        mPostFeedModel.setFeed(getFeed(POST_FEED_SINGLE_POST));
        QCOMPARE(mPostFeedModel.rowCount(), 1);
        mPostFeedModel.clear();
        QCOMPARE(mPostFeedModel.rowCount(), 0);
    }

    void refreshFeedWithSame()
    {
        int newTopIndex = mPostFeedModel.setFeed(getFeed(POST_FEED_SINGLE_POST));
        QCOMPARE(newTopIndex, -1);
        QCOMPARE(mPostFeedModel.rowCount(), 1);
        newTopIndex = mPostFeedModel.setFeed(getFeed(POST_FEED_SINGLE_POST));
        QCOMPARE(newTopIndex, 0);
        QCOMPARE(mPostFeedModel.rowCount(), 1);
    }

private:
    static constexpr char const* POST_FEED_SINGLE_POST = R"##({
        "feed": [
            {
                "post": {
                    "uri": "at://did:plc:foo/app.bsky.feed.post/r001",
                    "cid": "cid001",
                    "author": {
                        "did": "did:plc:foo",
                        "handle": "foo.bsky.social"
                    },
                    "record": {
                        "$type": "app.bsky.feed.post",
                        "text": "Hello world!",
                        "createdAt": "2023-11-20T18:46:00.000Z"
                    },
                    "indexedAt": "2023-11-20T18:46:00.000Z"
                }
            }
        ]
    })##";

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
