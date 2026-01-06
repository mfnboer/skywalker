// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_feed.h>

namespace Skywalker {

class PostFeedModel;

class PostFeedReplay : public QObject
{
    Q_OBJECT

public:
    using Ptr = std::unique_ptr<PostFeedReplay>;
    using SharedPtr = std::shared_ptr<PostFeedReplay>;
    using LoadCb = std::function<void(SharedPtr)>;

    void clear();
    void append(ATProto::AppBskyFeed::OutputFeed::SharedPtr page);
    void prepend(ATProto::AppBskyFeed::OutputFeed::SharedPtr page, bool gapTillNextPage);
    void gapFill(ATProto::AppBskyFeed::OutputFeed::SharedPtr page, const QString& cursor, bool gapTillNextPage);
    void closeGap(const QString& cursor);
    void removeTail(const QString& cursor);
    void removeHead(const QString& cursor);

    void replay(PostFeedModel& model) const;

    QJsonObject toJson() const;
    void save(const QString& userDid, const QString& feedName);

    static SharedPtr fromJson(const QJsonObject& json);
    static bool load(const QString& userDid, const QString& feedName, const LoadCb& loadCb);

private:
    struct PageList
    {
        ATProto::AppBskyFeed::OutputFeed::List mPages;
        bool mGapTillNextPage = false;

        QJsonObject toJson() const;

        using SharedPtr = std::shared_ptr<PageList>;
        static SharedPtr fromJson(const QJsonObject& json);
    };

    PageList* findGap(const QString& cursor);
    void save(const QString& userDid, const QString& feedName, const QJsonObject json);
    static QString getReplayFileName(const QString& userDid, const QString& feedName);
    void finishSave();
    static SharedPtr loadContinue(const QString& userDid, const QString& feedName);
    static void finishLoad();

    std::vector<PageList::SharedPtr> mPagePrependSequence;
    bool mDirty = false;
    std::unique_ptr<QThread> mSaveThread;

    static std::unique_ptr<QThread> sLoadThread;
    static SharedPtr sLoadedReplay;
    static LoadCb sLoadCb;
};

}
