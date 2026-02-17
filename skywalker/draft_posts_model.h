// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "shared_image_provider.h"

namespace Skywalker {

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

    QString getFeedName() const override { return "Draft posts"; }
    Q_INVOKABLE void clear();
    void setFeed(SkyDrafts feed);
    void setFeed(BlueskyDrafts feed);
    void deleteDraft(int index);
    std::vector<Post> getThread(int index) const;

    QVariant data(const QModelIndex& index, int role) const override;

private:
    using RawDrafts = std::variant<SkyDrafts, BlueskyDrafts>;

    QList<ImageView> createDraftImages(const Post& post) const;
    std::vector<Post> getSkyDraftThread(int index) const;
    std::vector<Post> getBlueskyDraftThread(int index) const;
    ATProto::AppBskyFeed::PostView::SharedPtr toPostView(
        const ATProto::AppBskyDraft::DraftPost::SharedPtr& draftPost,
        const ATProto::AppBskyDraft::DraftView::SharedPtr& draftView) const;

    RawDrafts mRawFeed;
    std::unordered_map<QString, QList<ImageView>> mPostUriDraftImagesMap;
    std::vector<SharedImageSource::Ptr> mMemeSources;
};

}
