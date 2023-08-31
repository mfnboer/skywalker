// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post.h"

namespace Skywalker {

int Post::sNextGapId = 1;

Post Post::createGapPlaceHolder(const QString& gapCursor)
{
    Post post;
    post.mGapCursor = gapCursor;
    post.mGapId = sNextGapId++;
    return post;
}

Post::Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost, int rawIndex) :
    mFeedViewPost(feedViewPost),
    mRawIndex(rawIndex)
{
    Q_ASSERT((feedViewPost && rawIndex >= 0) || (!feedViewPost && rawIndex == -1));
}

const QString& Post::getCid() const
{
    static const QString NO_STRING;
    return mFeedViewPost ? mFeedViewPost->mPost->mCid : NO_STRING;
}

QString Post::getText() const
{
    static const QString NO_STRING;

    if (!mFeedViewPost)
        return NO_STRING;

    const auto& post = mFeedViewPost->mPost;
    if (post->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(post->mRecord);

        if (record->mFacets.empty())
            return record->mText.toHtmlEscaped();
        else
            return applyFacets(*record);
    }

    return NO_STRING;
}

BasicProfile Post::getAuthor() const
{
    return mFeedViewPost ? BasicProfile(mFeedViewPost->mPost->mAuthor.get()) : BasicProfile();
}

QDateTime Post::getIndexedAt() const
{
    if (!mFeedViewPost)
        return {};

    // NOTE: the createdAt timestamp is not reliable as clients can put in a local timestamp
    // without timezone (seen in feeds)
    return mFeedViewPost->mPost->mIndexedAt;
}

QDateTime Post::getTimelineTimestamp() const
{
    if (!mFeedViewPost)
        return {};

    if (mFeedViewPost->mReason)
        return mFeedViewPost->mReason->mIndexedAt;

    return getIndexedAt();
}

std::optional<BasicProfile> Post::getRepostedBy() const
{
    if (!mFeedViewPost)
        return {};

    if (!mFeedViewPost->mReason)
        return {};

    return BasicProfile(mFeedViewPost->mReason->mBy.get());
}

std::vector<ImageView::Ptr> Post::getImages() const
{
    if (!mFeedViewPost)
        return {};

    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::Ptr>(post->mEmbed->mEmbed);
    std::vector<ImageView::Ptr> images;

    for (const auto& img : imagesView->mImages)
        images.push_back(std::make_unique<ImageView>(img.get()));

    return images;
}

ExternalView::Ptr Post::getExternalView() const
{
    if (!mFeedViewPost)
        return {};

    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::Ptr>(post->mEmbed->mEmbed)->mExternal;
    return std::make_unique<ExternalView>(external.get());
}

RecordView::Ptr Post::getRecordView() const
{
    if (!mFeedViewPost)
        return {};

    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::RECORD_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordView::Ptr>(post->mEmbed->mEmbed);
    return std::make_unique<RecordView>(*recordView);
}

RecordWithMediaView::Ptr Post::getRecordWithMediaView() const
{
    if (!mFeedViewPost)
        return {};

    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::Ptr>(post->mEmbed->mEmbed);
    return std::make_unique<RecordWithMediaView>(recordView.get());
}

QString Post::applyFacets(const ATProto::AppBskyFeed::Record::Post& post) const
{
    std::map<int, HyperLink> startLinkMap;
    const auto& bytes = post.mText.toUtf8();

    for (const auto& facet : post.mFacets)
    {
        if (facet->mFeatures.empty())
        {
            qWarning() << "Empty facet:" << post.mText;
            continue;
        }

        // What to do with multiple features?
        if (facet->mFeatures.size() > 1)
            qWarning() << "Multiple features, taking only the first";

        HyperLink link;
        link.mStart = facet->mIndex.mByteStart;
        link.mEnd = facet->mIndex.mByteEnd;
        const int sliceSize = link.mEnd - link.mStart;

        if (link.mStart < 0 || link.mEnd > bytes.size() || sliceSize < 0)
        {
            qWarning() << "Invalid index in facet:" << post.mText;
            continue;
        }

        const auto linkText = QString(bytes.sliced(link.mStart, sliceSize));
        const auto& feature = facet->mFeatures.front();

        switch (feature.mType)
        {
        case ATProto::AppBskyRichtext::Facet::Feature::Type::MENTION:
        {
            const auto& facetMention = std::get<ATProto::AppBskyRichtext::FacetMention::Ptr>(feature.mFeature);
            link.mText = QString("<a href=\"%1\">%2</a>").arg(facetMention->mDid, linkText);
            break;
        }
        case ATProto::AppBskyRichtext::Facet::Feature::Type::LINK:
        {
            const auto& facetLink = std::get<ATProto::AppBskyRichtext::FacetLink::Ptr>(feature.mFeature);
            link.mText = QString("<a href=\"%1\">%2</a>").arg(facetLink->mUri, linkText);
            break;
        }
        case ATProto::AppBskyRichtext::Facet::Feature::Type::UNKNOWN:
            qWarning() << "Uknown facet type:" << int(feature.mType) << "post:" << post.mText;
            continue;
            break;
        }

        startLinkMap[link.mStart] = link;
    }

    QString result;
    int bytePos = 0;

    for (const auto& [start, link] : startLinkMap)
    {
        if (start < bytePos)
        {
            qWarning() << "Overlapping facets:" << post.mText;
            result.clear();
            bytePos = 0;
            break;
        }

        const auto before = bytes.sliced(bytePos, start - bytePos);
        result.append(QString(before).toHtmlEscaped());
        result.append(link.mText);
        bytePos = link.mEnd;
    }

    result.append(QString(bytes.sliced(bytePos)).toHtmlEscaped());
    qDebug() << "Orig:   " << post.mText;
    qDebug() << "Faceted:" << result;
    return result;
}

}
