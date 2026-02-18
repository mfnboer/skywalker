// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "shared_image_provider.h"

namespace Skywalker {

class DraftPosts;

class DraftPostsModel : public AbstractPostFeedModel
{
    Q_OBJECT

public:
    using Ptr = std::unique_ptr<DraftPostsModel>;
    using SkyDrafts = std::vector<ATProto::AppBskyFeed::PostFeed>;
    using BlueskyDrafts = std::vector<ATProto::AppBskyDraft::DraftView::SharedPtr>;

    DraftPostsModel(const QString& userDid,
                    const IProfileStore& mutedReposts,
                    const IContentFilter& contentFilter,
                    const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                    HashtagIndex& hashtags,
                    QObject* parent = nullptr);

    void setDraftPosts(DraftPosts* draftPosts);
    QString getFeedName() const override { return "Draft posts"; }
    Q_INVOKABLE void clear();
    void setFeed(std::vector<ATProto::AppBskyFeed::PostFeed> feed, const QString& cursor);
    void deleteDraft(int index);
    std::vector<Post> getThread(int index) const;
    const QString& getCursor() const { return mCursor; }

    QVariant data(const QModelIndex& index, int role) const override;

    void updatePostExternal(const Post& post, int index);
    void updatePostRecord(const Post& post, int index);
    void updatePostRecordFailed(const Post& post, int index);

private:
    QList<ImageView> createDraftImages(const Post& post) const;
    void getPostExternal(int index) const;
    void getPostRecord(int index) const;

    QString mCursor;
    std::vector<ATProto::AppBskyFeed::PostFeed> mRawFeed;
    std::unordered_map<QString, QList<ImageView>> mPostUriDraftImagesMap;
    std::vector<SharedImageSource::Ptr> mMemeSources;
    DraftPosts* mDraftPosts = nullptr;
    std::unordered_set<QString> mGettingPostRecord;
    std::unordered_set<QString> mGettingPostExternal;
};

}
