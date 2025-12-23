// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "post_feed_model.h"

namespace Skywalker {

class IListStore;

class FilteredContentPostFeedModel : public PostFeedModel
{
    Q_OBJECT

public:
    explicit FilteredContentPostFeedModel(
        const QString& feedName, const FeedVariant* feedVariant,
        const QString& userDid,
        const IProfileStore& mutedReposts,
        const IListStore& feedHide,
        const IContentFilter& contentFilter,
        const IMatchWords& mutedWords,
        const FocusHashtags& focusHashtags,
        HashtagIndex& hashtags,
        const ATProto::UserPreferences& userPrefs,
        UserSettings& userSettings,
        FollowsActivityStore& followsActivityStore,
        ATProto::Client::SharedPtr bsky,
        QObject* parent = nullptr);

    void setHighlightColor(const QString& color) { mHighlightColor = color; }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    QString highlightMutedWords(const Post& post, const QString& text) const;
    bool highlightMutedWordsImages(const Post& post, QList<ImageView>& images) const;
    bool highlightMutedWordsVideo(const Post& post, VideoView& videoView) const;
    bool highlightMutedWordsExternal(const Post& post, ExternalView& external) const;
    bool highlightMutedWordsRecord(const Post& post, RecordView& record) const;

    PostFeedModel::FeedVariant mFeedVariant;
    QString mHighlightColor = "palevioletred";
};

}
