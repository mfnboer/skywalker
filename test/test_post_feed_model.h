// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <definitions.h>
#include <focus_hashtags.h>
#include <post_feed_model.h>
#include <QtTest/QTest>

using namespace Skywalker;
using namespace std::chrono_literals;

class TestPostFeedModel : public QObject
{
    Q_OBJECT
private slots:
    void init()
    {
        mPostFeedModel = std::make_unique<PostFeedModel>(
            HOME_FEED, mUserDid, mFollowing, mMutedReposts, mContentFilter,
            mBookmarks, mMutedWords, mFocusHashtags, mHashtags, mUserPreferences, mUserSettings);
    }

    void cleanup()
    {
        mPostFeedModel = nullptr;
        mNextPostId = 1;
    }

    void setFeed()
    {
        mPostFeedModel->setFeed(getFeed(1, TEST_DATE));
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        QVERIFY(mPostFeedModel->getLastCursor().isEmpty());
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE);

        const Post& post = mPostFeedModel->getPost(0);
        QCOMPARE(post.getPostType(), QEnums::POST_STANDALONE);
        QVERIFY(post.isEndOfFeed());
    }

    void clearFeed()
    {
        mPostFeedModel->setFeed(getFeed(1, TEST_DATE));
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        mPostFeedModel->clear();
        QCOMPARE(mPostFeedModel->rowCount(), 0);
    }

    void refreshFeedWithSame()
    {
        mPostFeedModel->setFeed(getFeed(1, TEST_DATE));
        QCOMPARE(mPostFeedModel->rowCount(), 1);

        mNextPostId = 1;
        mPostFeedModel->setFeed(getFeed(1, TEST_DATE));
        QCOMPARE(mPostFeedModel->rowCount(), 1);
    }

    void addToEmptyFeed()
    {
        mPostFeedModel->addFeed(getFeed(1, TEST_DATE));
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        QVERIFY(mPostFeedModel->getLastCursor().isEmpty());
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE);
    }

    void prependToEmptyFeed()
    {
        int gapId = mPostFeedModel->prependFeed(getFeed(1, TEST_DATE));
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        QVERIFY(mPostFeedModel->getLastCursor().isEmpty());
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE);
    }

    void addMultipleFeeds()
    {
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        QCOMPARE(mPostFeedModel->rowCount(), 5);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE - 4s);
        const Post* post = &mPostFeedModel->getPost(4);
        QVERIFY(!post->isEndOfFeed());

        mPostFeedModel->addFeed(getFeed(5, TEST_DATE - 1h, "CUR2"));
        QCOMPARE(mPostFeedModel->rowCount(), 10);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR2");
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE - 1h - 4s);
        post = &mPostFeedModel->getPost(9);
        QVERIFY(!post->isEndOfFeed());

        mPostFeedModel->addFeed(getFeed(5, TEST_DATE - 2h));
        QCOMPARE(mPostFeedModel->rowCount(), 15);
        QVERIFY(mPostFeedModel->getLastCursor().isEmpty());
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE - 2h - 4s);
        post = &mPostFeedModel->getPost(14);
        QVERIFY(post->isEndOfFeed());
    }

    void prependWithOverlap()
    {
        mNextPostId = 2;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        mNextPostId = 1;
        int gapId = mPostFeedModel->prependFeed(getFeed(5, TEST_DATE + 1s, "CUR2"));
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 6);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE - 4s);
    }

    void prependWithOverlapOnTime()
    {
        mNextPostId = 2;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        int gapId = mPostFeedModel->prependFeed(getFeed(5, TEST_DATE + 1s, "CUR2"));
        QCOMPARE(gapId, 0);
        // 2 posts are prepended, then second post has the same time is the current first
        // post in the previous feed but different CID. This is considered to be a new posts.
        QCOMPARE(mPostFeedModel->rowCount(), 7);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE - 4s);
    }

    void prependEmptyFeed()
    {
        mNextPostId = 2;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        QCOMPARE(mPostFeedModel->rowCount(), 5);
        int gapId = mPostFeedModel->prependFeed(getFeed(0, TEST_DATE + 1s, "CUR2"));
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 5);
    }

    void prependFullOverlap()
    {
        mNextPostId = 2;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        QCOMPARE(mPostFeedModel->rowCount(), 5);

        mNextPostId = 2;
        int gapId = mPostFeedModel->prependFeed(getFeed(3, TEST_DATE, "CUR2"));
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 5);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
    }

    void prependFullOverlapOnTime()
    {
        mNextPostId = 2;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        QCOMPARE(mPostFeedModel->rowCount(), 5);

        int gapId = mPostFeedModel->prependFeed(getFeed(3, TEST_DATE - 1s, "CUR2"));
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 5);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
    }

    void gapFill()
    {
        mNextPostId = 3;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        mNextPostId = 1;
        int gapId = mPostFeedModel->prependFeed(getFeed(1, TEST_DATE + 2s, "CUR2"));
        QCOMPARE_GT(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 6 posts + gap place holder
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE - 4s);

        const Post& post = mPostFeedModel->getPost(1);
        QVERIFY(post.isPlaceHolder());
        QVERIFY(post.isGap());
        QCOMPARE(post.getGapId(), gapId);
        QCOMPARE(post.getGapCursor(), "CUR2");

        const Post* gap = mPostFeedModel->getGapPlaceHolder(gapId);
        QCOMPARE(gap, &post);

        mNextPostId = 2;
        gapId = mPostFeedModel->gapFillFeed(getFeed(3, TEST_DATE + 1s, "CUR3"), gapId);
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 7 posts
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
        QCOMPARE(mPostFeedModel->lastTimestamp(), TEST_DATE - 4s);

        for (int i = 0; i < mPostFeedModel->rowCount(); ++i)
        {
            const Post& p = mPostFeedModel->getPost(i);
            QVERIFY(!p.isPlaceHolder());
            QVERIFY(!p.isGap());
        }
    }

    void multipleGapsFill()
    {
        mNextPostId = 5;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        mNextPostId = 1;
        int gapId = mPostFeedModel->prependFeed(getFeed(1, TEST_DATE + 4s, "CUR2"));
        QCOMPARE_GT(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 6 posts + gap place holder
        QCOMPARE(mPostFeedModel->getPost(1).getGapId(), gapId);

        mNextPostId = 2;
        gapId = mPostFeedModel->gapFillFeed(getFeed(2, TEST_DATE + 3s, "CUR3"), gapId);
        QCOMPARE_GT(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 9); // 8 posts + gap place holder
        QCOMPARE(mPostFeedModel->getPost(3).getGapId(), gapId);

        mNextPostId = 4;
        gapId = mPostFeedModel->gapFillFeed(getFeed(3, TEST_DATE + 1s, "CUR4"), gapId);
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 9); // 9 posts

        for (int i = 0; i < mPostFeedModel->rowCount(); ++i)
        {
            const Post& p = mPostFeedModel->getPost(i);
            QVERIFY(!p.isPlaceHolder());
            QVERIFY(!p.isGap());
        }
    }

    void emptyGapFill()
    {
        mNextPostId = 3;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        mNextPostId = 1;
        int gapId = mPostFeedModel->prependFeed(getFeed(2, TEST_DATE + 2s, "CUR2"));
        QCOMPARE_GT(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 8); // 7 posts + gap place holder

        mNextPostId = 3;
        gapId = mPostFeedModel->gapFillFeed(getFeed(2, TEST_DATE, "CUR3"), gapId);
        QCOMPARE(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 7 posts

        for (int i = 0; i < mPostFeedModel->rowCount(); ++i)
        {
            const Post& p = mPostFeedModel->getPost(i);
            QVERIFY(!p.isPlaceHolder());
            QVERIFY(!p.isGap());
        }
    }

    void removeTailPosts()
    {
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE - 1h, "CUR2"));
        QCOMPARE(mPostFeedModel->rowCount(), 10);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR2");

        mPostFeedModel->removeTailPosts(5);
        QCOMPARE(mPostFeedModel->rowCount(), 5);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
    }

    void removeTailPostsLessThanRequested()
    {
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE - 1h, "CUR2"));
        mPostFeedModel->removeTailPosts(6);
        // 6 posts cannot be removed as CUR1 is at the 5th post.
        QCOMPARE(mPostFeedModel->rowCount(), 5);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
    }

    void removeTailPostsDenied()
    {
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE - 1h, "CUR2"));
        mPostFeedModel->removeTailPosts(4);
        // 4 posts cannot be removed as CUR1 is at the 5th post and CUR2 at the 10th
        QCOMPARE(mPostFeedModel->rowCount(), 10);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR2");
    }

    void removeTailPostsWithGap()
    {
        mNextPostId = 3;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        mNextPostId = 1;
        int gapId = mPostFeedModel->prependFeed(getFeed(1, TEST_DATE + 2s, "CUR2"));
        QCOMPARE_GT(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 6 posts + gap
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");

        mPostFeedModel->removeTailPosts(6);
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR2");
        QVERIFY(!mPostFeedModel->getGapPlaceHolder(gapId));
    }

    void removeTailPostsAfterGapDenied()
    {
        mNextPostId = 3;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        mNextPostId = 1;
        int gapId = mPostFeedModel->prependFeed(getFeed(1, TEST_DATE + 2s, "CUR2"));
        QCOMPARE_GT(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 6 posts + gap
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");

        // Cannot remove 5 posts as the gap would become the last post.
        mPostFeedModel->removeTailPosts(5);
        QCOMPARE(mPostFeedModel->rowCount(), 7);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
    }

    void removeHeadPosts()
    {
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE - 1h, "CUR2"));
        QCOMPARE(mPostFeedModel->rowCount(), 10);

        mPostFeedModel->removeHeadPosts(2);
        QCOMPARE(mPostFeedModel->rowCount(), 8);
    }

    void removeHeadAndTailPosts()
    {
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE - 1h, "CUR2"));
        QCOMPARE(mPostFeedModel->rowCount(), 10);

        mPostFeedModel->removeHeadPosts(2);
        QCOMPARE(mPostFeedModel->rowCount(), 8);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR2");

        mPostFeedModel->removeTailPosts(7);
        QCOMPARE(mPostFeedModel->rowCount(), 3);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
    }

    void removeHeadPostsWithGap()
    {
        mNextPostId = 3;
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        mNextPostId = 1;
        int gapId = mPostFeedModel->prependFeed(getFeed(1, TEST_DATE + 2s, "CUR2"));
        QCOMPARE_GT(gapId, 0);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 6 posts + gap

        // The gap should be removed too as it follows the first post
        mPostFeedModel->removeHeadPosts(1);
        QCOMPARE(mPostFeedModel->rowCount(), 5);
        QCOMPARE(mPostFeedModel->getLastCursor(), "CUR1");
        QVERIFY(!mPostFeedModel->getGapPlaceHolder(gapId));
    }

    void findTimestamp()
    {
        mPostFeedModel->addFeed(getFeed(5, TEST_DATE, "CUR1"));

        int index = mPostFeedModel->findTimestamp(TEST_DATE);
        QCOMPARE(index, 0);

        index = mPostFeedModel->findTimestamp(TEST_DATE - 500ms);
        QCOMPARE(index, 1);

        index = mPostFeedModel->findTimestamp(TEST_DATE - 1s);
        QCOMPARE(index, 1);

        index = mPostFeedModel->findTimestamp(TEST_DATE - 4s);
        QCOMPARE(index, 4);

        index = mPostFeedModel->findTimestamp(TEST_DATE - 5s);
        QCOMPARE(index, -1);

        index = mPostFeedModel->findTimestamp(TEST_DATE + 1s);
        QCOMPARE(index, 0);
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

    ATProto::AppBskyFeed::OutputFeed::SharedPtr getFeed(int numPosts, QDateTime startTime, const std::optional<QString>& cursor = {})
    {
        QString feedData = R"###({ "feed": [)###";

        for (int i = 1; i <= numPosts; ++i)
        {
            auto postTime = startTime - (i-1) * 1s;
            QString postData = QString(POST_TEMPLATE).arg(QString::number(mNextPostId++),
                                                          postTime.toString(Qt::ISODateWithMs));
            feedData += postData;

            if (i < numPosts)
                feedData += ',';
        }

        feedData += "]}";

        return getFeed(feedData.toUtf8(), cursor);
    }

    ATProto::AppBskyFeed::OutputFeed::SharedPtr getFeed(const char* data, const std::optional<QString>& cursor)
    {
        QJsonParseError error;
        auto json = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError)
            qFatal() << "Failed to parse json:" << error.errorString() << "offset:" << error.offset;

        auto feed = ATProto::AppBskyFeed::OutputFeed::fromJson(json);
        feed->mCursor = cursor;
        return feed;
    }

    QString mUserDid;
    ProfileStore mFollowing;
    ProfileStore mMutedReposts;
    ATProto::UserPreferences mUserPreferences;
    UserSettings mUserSettings;
    ContentFilter mContentFilter{mUserPreferences, &mUserSettings};
    Bookmarks mBookmarks;
    MutedWords mMutedWords;
    FocusHashtags mFocusHashtags;
    HashtagIndex mHashtags{10};
    PostFeedModel::Ptr mPostFeedModel;
    int mNextPostId = 1;
};
