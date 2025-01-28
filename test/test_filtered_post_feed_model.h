// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <definitions.h>
#include <focus_hashtags.h>
#include <muted_words.h>
#include <filtered_post_feed_model.h>
#include <user_settings.h>
#include <QtTest/QTest>

using namespace Skywalker;
using namespace std::chrono_literals;

class TestFilteredPostFeedModel : public QObject
{
    Q_OBJECT
private slots:
    void init()
    {
        BasicProfile profile("did:plc:foo", "foo.bsky.social", "Foo", "");
        auto filter = std::make_unique<AuthorPostFilter>(profile);

        mPostFeedModel = std::make_unique<FilteredPostFeedModel>(
            std::move(filter), nullptr, mUserDid, mFollowing, mMutedReposts, mContentFilter,
            mBookmarks, mMutedWords, mFocusHashtags, mHashtags);
    }

    void cleanup()
    {
        mPostFeedModel = nullptr;
        mNextPostId = 1;
    }

    void setFeed()
    {
        mPostFeedModel->setPosts(getTimeline(1, TEST_DATE), 1);
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        QCOMPARE(mPostFeedModel->getNumPostsChecked(), 0);

        mPostFeedModel->setPosts(getTimeline(2, TEST_DATE), 2);
        QCOMPARE(mPostFeedModel->rowCount(), 2);
        QCOMPARE(mPostFeedModel->getNumPostsChecked(), 0);

        mPostFeedModel->setPosts(getTimeline(2, TEST_DATE), 1);
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        QCOMPARE(mPostFeedModel->getNumPostsChecked(), 0);

        mPostFeedModel->setPosts(getTimeline(4, TEST_DATE), 4);
        QCOMPARE(mPostFeedModel->rowCount(), 3);
        QCOMPARE(mPostFeedModel->getNumPostsChecked(), 1);

        for (int i = 0; i < 3; ++i)
        {
            const Post& post = mPostFeedModel->getPost(i);
            QCOMPARE(post.getAuthor().getDid(), "did:plc:foo");
        }
    }

    void clearFeed()
    {
        mPostFeedModel->setPosts(getTimeline(1, TEST_DATE), 1);
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        mPostFeedModel->clear();
        QCOMPARE(mPostFeedModel->rowCount(), 0);
    }

    void addToEmptyFeed()
    {
        mPostFeedModel->addPosts(getTimeline(1, TEST_DATE), 1);
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        QCOMPARE(mPostFeedModel->getNumPostsChecked(), 0);

        mPostFeedModel->clear();
        mPostFeedModel->addPosts(getTimeline(4, TEST_DATE), 4);
        QCOMPARE(mPostFeedModel->rowCount(), 3);
        QCOMPARE(mPostFeedModel->getNumPostsChecked(), 1);
    }

    void prependToEmptyFeed()
    {
        mPostFeedModel->prependPosts(getTimeline(1, TEST_DATE), 1);
        QCOMPARE(mPostFeedModel->rowCount(), 1);
        QCOMPARE(mPostFeedModel->getNumPostsChecked(), 0);

        mPostFeedModel->clear();
        mPostFeedModel->prependPosts(getTimeline(4, TEST_DATE), 4);
        QCOMPARE(mPostFeedModel->rowCount(), 3);
        QCOMPARE(mPostFeedModel->getNumPostsChecked(), 0);
    }

    void gapFill()
    {
        mNextPostId = 3;
        mPostFeedModel->addPosts(getTimeline(5, TEST_DATE), 5);
        QCOMPARE(mPostFeedModel->rowCount(), 4);

        mNextPostId = 1;
        auto posts = getTimeline(1, TEST_DATE + 2s);
        auto gap = Post::createGapPlaceHolder("GAP");
        const int gapId = gap.getGapId();
        posts.push_back(gap);
        mPostFeedModel->prependPosts(posts, posts.size());
        QCOMPARE(mPostFeedModel->rowCount(), 6); // 5 posts + gap place holder

        const Post& post = mPostFeedModel->getPost(1);
        QVERIFY(post.isPlaceHolder());
        QVERIFY(post.isGap());
        QCOMPARE(post.getGapId(), gapId);

        mNextPostId = 2;
        mPostFeedModel->gapFill(getTimeline(3, TEST_DATE + 1s), 2, gapId);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 7 posts

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
        mPostFeedModel->addPosts(getTimeline(5, TEST_DATE), 5);
        QCOMPARE(mPostFeedModel->rowCount(), 4);

        mNextPostId = 1;
        auto posts = getTimeline(2, TEST_DATE + 2s);
        auto gap = Post::createGapPlaceHolder("GAP");
        const int gapId = gap.getGapId();
        posts.push_back(gap);
        mPostFeedModel->prependPosts(posts, posts.size());
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 6 posts + gap place holder

        mNextPostId = 3;
        mPostFeedModel->gapFill(getTimeline(2, TEST_DATE, "bar"), 2, gapId);
        QCOMPARE(mPostFeedModel->rowCount(), 6); // 6 posts (all gap posts filtered away)

        for (int i = 0; i < mPostFeedModel->rowCount(); ++i)
        {
            const Post& p = mPostFeedModel->getPost(i);
            QVERIFY(!p.isPlaceHolder());
            QVERIFY(!p.isGap());
        }
    }

    void gapFillAfterGapMoveDown()
    {
        mNextPostId = 3;
        mPostFeedModel->addPosts(getTimeline(5, TEST_DATE), 5);
        QCOMPARE(mPostFeedModel->rowCount(), 4);

        mNextPostId = 1;
        auto posts = getTimeline(1, TEST_DATE + 2s);
        auto gap = Post::createGapPlaceHolder("GAP");
        const int gapId = gap.getGapId();
        posts.push_back(gap);
        mPostFeedModel->prependPosts(posts, posts.size());
        QCOMPARE(mPostFeedModel->rowCount(), 6); // 5 posts + gap place holder

        const Post& post = mPostFeedModel->getPost(1);
        QVERIFY(post.isGap());
        QCOMPARE(post.getGapId(), gapId);

        mPostFeedModel->prependPosts(getTimeline(1, TEST_DATE), 1);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 6 posts + gap place holder

        const Post& post2 = mPostFeedModel->getPost(1);
        QVERIFY(!post2.isGap());

        const Post& post3 = mPostFeedModel->getPost(2);
        QVERIFY(post3.isGap());
        QCOMPARE(post3.getGapId(), gapId);

        mNextPostId = 2;
        mPostFeedModel->gapFill(getTimeline(3, TEST_DATE + 1s), 1, gapId);
        QCOMPARE(mPostFeedModel->rowCount(), 7); // 7 posts

        for (int i = 0; i < mPostFeedModel->rowCount(); ++i)
        {
            const Post& p = mPostFeedModel->getPost(i);
            QVERIFY(!p.isPlaceHolder());
            QVERIFY(!p.isGap());
        }

        const Post& post4 = mPostFeedModel->getPost(2);
        QVERIFY(!post4.isGap());
        QCOMPARE(post4.getCid(), "cid2");
    }

    void gapFillAfterGapMoveUp()
    {
        mNextPostId = 3;
        mPostFeedModel->addPosts(getTimeline(5, TEST_DATE), 5);
        QCOMPARE(mPostFeedModel->rowCount(), 4);

        mNextPostId = 1;
        auto posts = getTimeline(1, TEST_DATE + 2s);
        auto gap = Post::createGapPlaceHolder("GAP");
        const int gapId = gap.getGapId();
        posts.push_back(gap);
        mPostFeedModel->prependPosts(posts, posts.size());
        QCOMPARE(mPostFeedModel->rowCount(), 6); // 5 posts + gap place holder

        const Post& post = mPostFeedModel->getPost(1);
        QVERIFY(post.isGap());
        QCOMPARE(post.getGapId(), gapId);

        mPostFeedModel->removeHeadPosts(getTimeline(1, TEST_DATE + 2s), 1);
        QCOMPARE(mPostFeedModel->rowCount(), 5); // 4 posts + gap place holder

        const Post& post2 = mPostFeedModel->getPost(1);
        QVERIFY(!post2.isGap());

        const Post& post3 = mPostFeedModel->getPost(0);
        QVERIFY(post3.isGap());
        QCOMPARE(post3.getGapId(), gapId);

        mNextPostId = 2;
        mPostFeedModel->gapFill(getTimeline(3, TEST_DATE + 1s), 1, gapId);
        QCOMPARE(mPostFeedModel->rowCount(), 5); // 5 posts

        for (int i = 0; i < mPostFeedModel->rowCount(); ++i)
        {
            const Post& p = mPostFeedModel->getPost(i);
            QVERIFY(!p.isPlaceHolder());
            QVERIFY(!p.isGap());
        }

        const Post& post4 = mPostFeedModel->getPost(0);
        QVERIFY(!post4.isGap());
        QCOMPARE(post4.getCid(), "cid2");
    }

    void removeTailPosts()
    {
        mPostFeedModel->addPosts(getTimeline(5, TEST_DATE), 5);
        mPostFeedModel->addPosts(getTimeline(5, TEST_DATE - 1h), 5);
        QCOMPARE(mPostFeedModel->rowCount(), 8);

        mPostFeedModel->removeTailPosts(getTimeline(5, TEST_DATE - 1h), 5);
        QCOMPARE(mPostFeedModel->rowCount(), 4);
    }

    void removeHeadPosts()
    {
        mPostFeedModel->addPosts(getTimeline(5, TEST_DATE), 5);
        mPostFeedModel->addPosts(getTimeline(5, TEST_DATE - 1h), 5);
        QCOMPARE(mPostFeedModel->rowCount(), 8);

        mPostFeedModel->removeHeadPosts(getTimeline(6, TEST_DATE), 6);
        QCOMPARE(mPostFeedModel->rowCount(), 3);
    }

private:
    static constexpr char const* POST_TEMPLATE = R"##({
        "post": {
            "uri": "at://did:plc:foo/app.bsky.feed.post/r%1",
            "cid": "cid%1",
            "author": {
                "did": "did:plc:%3",
                "handle": "%3.bsky.social"
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

    std::deque<Post> getTimeline(int numPosts, QDateTime startTime, QString authorId = "")
    {
        std::deque<Post> timeline;
        auto feed = getFeed(numPosts, startTime, authorId);

        for (const auto& viewPost : feed->mFeed)
            timeline.push_back(Post(viewPost));

        return timeline;
    }

    ATProto::AppBskyFeed::OutputFeed::SharedPtr getFeed(int numPosts, QDateTime startTime, const QString& authorId)
    {
        QString feedData = R"###({ "feed": [)###";

        for (int i = 1; i <= numPosts; ++i)
        {
            auto postTime = startTime - (i-1) * 1s;
            QString author = authorId.isEmpty() ?
                                 ((i % 4) == 0 ? "bar" : "foo") :
                                 authorId;
            QString postData = QString(POST_TEMPLATE).arg(QString::number(mNextPostId++),
                                                          postTime.toString(Qt::ISODateWithMs),
                                                          author);
            feedData += postData;

            if (i < numPosts)
                feedData += ',';
        }

        feedData += "]}";

        return getFeed(feedData.toUtf8(), {});
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
    MutedWords mMutedWords{mFollowing};
    FocusHashtags mFocusHashtags;
    HashtagIndex mHashtags{10};
    FilteredPostFeedModel::Ptr mPostFeedModel;
    int mNextPostId = 1;
};
