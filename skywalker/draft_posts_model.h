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

    DraftPostsModel(const QString& userDid,
                    const IProfileStore& mutedReposts,
                    const IContentFilter& contentFilter,
                    const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                    HashtagIndex& hashtags,
                    QObject* parent = nullptr);

    Q_INVOKABLE int getMaxDrafts() const;
    Q_INVOKABLE void clear();
    void setFeed(std::vector<ATProto::AppBskyFeed::PostFeed> feed);
    void deleteDraft(int index);
    std::vector<Post> getThread(int index) const;

    QVariant data(const QModelIndex& index, int role) const override;

private:
    QList<ImageView> createDraftImages(const Post& post) const;

    std::vector<ATProto::AppBskyFeed::PostFeed> mRawFeed;
    std::unordered_map<QString, QList<ImageView>> mPostUriDraftImagesMap;
    std::vector<SharedImageSource::Ptr> mMemeSources;
};

}
