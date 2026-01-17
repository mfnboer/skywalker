// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "filtered_content_post_feed_model.h"
#include "words_highlighter.h"

namespace Skywalker {

FilteredContentPostFeedModel::FilteredContentPostFeedModel(
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
        QObject* parent) :
    PostFeedModel(feedName, feedVariant, userDid, mutedReposts,
                  feedHide, contentFilter, mutedWords, focusHashtags, hashtags,
                  userPrefs, userSettings, followsActivityStore, bsky, parent)
{
}

QVariant FilteredContentPostFeedModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mFeed.size())
        return {};

    const auto roleData = PostFeedModel::data(index, role);

    if (roleData.isNull())
        return roleData;

    Q_ASSERT(isFilteredPostFeed());

    if (!isFilteredPostFeed())
        return roleData;

    const auto& post = mFeed[index.row()];

    switch (Role(role))
    {
    case Role::PostText:
        return highlightMutedWords(post, roleData.value<QString>());
    case Role::PostImages:
    {
        auto images = roleData.value<QList<ImageView>>();
        highlightMutedWordsImages(post, images);
        return QVariant::fromValue(images);
    }
    case Role::PostVideo:
    {
        auto video = roleData.value<VideoView>();
        highlightMutedWordsVideo(post, video);
        return QVariant::fromValue(video);
    }
    case Role::PostExternal:
    {
        auto external = roleData.value<ExternalView>();
        highlightMutedWordsExternal(post, external);
        return QVariant::fromValue(external);
    }
    case Role::PostRecord:
    {
        auto record = roleData.value<RecordView>();
        record.setContentVisibility(QEnums::CONTENT_VISIBILITY_SHOW);
        record.setMutedReason(QEnums::MUTED_POST_NONE);
        highlightMutedWordsRecord(post, record);
        return QVariant::fromValue(record);
    }
    case Role::PostRecordWithMedia:
    {
        auto recordWithMedia = roleData.value<RecordWithMediaView>();
        auto& record = recordWithMedia.getRecord();

        if (!record.isNull())
        {
            record.setContentVisibility(QEnums::CONTENT_VISIBILITY_SHOW);
            record.setMutedReason(QEnums::MUTED_POST_NONE);
            highlightMutedWordsRecord(post, record);
        }

        auto images = recordWithMedia.getImages();

        if (highlightMutedWordsImages(post, images))
            recordWithMedia.setImages(images);

        VideoView::Ptr video = recordWithMedia.getVideoView();

        if (video && highlightMutedWordsVideo(post, *video))
            recordWithMedia.setVideo(*video);

        ExternalView::Ptr external = recordWithMedia.getExternalView();

        if (external && highlightMutedWordsExternal(post, *external))
            recordWithMedia.setExternal(*external);

        return QVariant::fromValue(recordWithMedia);
    }
    case Role::PostContentVisibility:
        return QEnums::CONTENT_VISIBILITY_SHOW;
    case Role::PostMutedReason:
        return QEnums::MUTED_POST_NONE;
    case Role::FilteredPostHideReason:
    {
        const auto it = mPostHideInfoMap->find(post.getCid());
        return it != mPostHideInfoMap->end() ? it->second.mHideReason : QEnums::HIDE_REASON_NONE;
    }
    case Role::FilteredPostHideDetail:
    {
        const auto it = mPostHideInfoMap->find(post.getCid());
        return it != mPostHideInfoMap->end() ?
                   ContentFilterStats::detailsToString(it->second.mDetails, mContentFilter) :
                   "";
    }
    case Role::FilteredPostContentLabel:
    {
        const auto it = mPostHideInfoMap->find(post.getCid());

        if (it == mPostHideInfoMap->end())
            return QVariant::fromValue(ContentLabel{});

        const auto& hideDetails = it->second.mDetails;

        if (std::holds_alternative<ContentLabel>(hideDetails))
            return QVariant::fromValue(std::get<ContentLabel>(hideDetails));

        return QVariant::fromValue(ContentLabel{});
    }
    default:
        break;
    }

    return roleData;
}

QString FilteredContentPostFeedModel::highlightMutedWords(const Post& post, const QString& text) const
{
    if (!mPostHideInfoMap)
        return text;

    const auto it = mPostHideInfoMap->find(post.getCid());

    if (it == mPostHideInfoMap->end() || it->second.mHideReason != QEnums::HIDE_REASON_MUTED_WORD)
        return text;

    const auto& details = it->second.mDetails;

    if (!std::holds_alternative<MutedWordEntry>(details))
        return text;

    const MutedWordEntry& mutedWordEntry = std::get<MutedWordEntry>(details);

    if (mutedWordEntry.isDomain())
    {
        // Domains should be matched against facets. Do not highlight for now.
        return text;
    }

    const WordsHighlighter wordsHighlighter;
    return wordsHighlighter.highlight(text, mutedWordEntry.getValue(), mHighlightColor,
                                      mutedWordEntry.isHashtag() || mutedWordEntry.isCashtag());
}

bool FilteredContentPostFeedModel::highlightMutedWordsImages(const Post& post, QList<ImageView>& images) const
{
    bool highlighted = false;

    for (auto& image : images)
    {
        const QString alt = UnicodeFonts::toCleanedHtml(image.getAlt());
        const QString highlightedAlt = highlightMutedWords(post, alt);

        if (alt != highlightedAlt)
        {
            image.setHtmlAlt(highlightedAlt);
            highlighted = true;
        }
    }

    return highlighted;
}

bool FilteredContentPostFeedModel::highlightMutedWordsVideo(const Post& post, VideoView& videoView) const
{
    const QString alt = UnicodeFonts::toCleanedHtml(videoView.getAlt());
    const QString highlightedAlt = highlightMutedWords(post, alt);

    if (alt != highlightedAlt)
    {
        videoView.setHtmlAlt(highlightedAlt);
        return true;
    }

    return false;
}

bool FilteredContentPostFeedModel::highlightMutedWordsExternal(const Post& post, ExternalView& external) const
{
    bool highlighted = false;

    const QString title = UnicodeFonts::toCleanedHtml(external.getTitle());
    const QString highlightedTitle = highlightMutedWords(post, title);

    if (title != highlightedTitle)
    {
        external.setHtmlTitle(highlightedTitle);
        highlighted = true;
    }

    const QString description = UnicodeFonts::toCleanedHtml(external.getDescription());
    const QString highlightedDescription = highlightMutedWords(post, description);

    if (description != highlightedDescription)
    {
        external.setHtmlDescription(highlightedDescription);
        highlighted = true;
    }

    return highlighted;
}

bool FilteredContentPostFeedModel::highlightMutedWordsRecord(const Post& post, RecordView& record) const
{
    bool highlighted = false;
    const QString highlightedText = highlightMutedWords(post, record.getFormattedText());

    if (record.getFormattedText() != highlightedText)
    {
        record.setFormattedText(highlightedText);
        highlighted = true;
    }

    auto images = record.getImages();

    if (highlightMutedWordsImages(post, images))
    {
        record.setImages(images);
        highlighted = true;
    }

    VideoView::Ptr video = record.getVideoView();

    if (video && highlightMutedWordsVideo(post, *video))
    {
        record.setVideo(*video);
        highlighted = true;
    }

    ExternalView::Ptr external = record.getExternalView();

    if (external && highlightMutedWordsExternal(post, *external))
    {
        record.setExternal(*external);
        highlighted = true;
    }

    return highlighted;
}

}
