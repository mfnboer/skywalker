// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "record_word_index.h"
#include "unicode_fonts.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

RecordWordIndex::RecordWordIndex(const ATProto::AppBskyEmbed::RecordViewRecord::SharedPtr& record) :
    mRecord(record)
{
}

QString RecordWordIndex::getText() const
{
    if (!mRecord)
        return {};

    switch (mRecord->mValueType)
    {
    case ATProto::RecordType::APP_BSKY_FEED_POST:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);

        if (recordValue->mBridgyOriginalText && !recordValue->mBridgyOriginalText->isEmpty())
            return UnicodeFonts::toPlainText(*recordValue->mBridgyOriginalText);
        else
            return recordValue->mText;
    }
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(mRecord->mValue);
        return recordValue->mDescription.value_or("");
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(mRecord->mValue);
        return recordValue->mDescription.value_or("");
    }
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(mRecord->mValue);
        return recordValue->mCreator->mDescription.value_or("");
    }
    default:
        break;
    }

    return {};
}

BasicProfile RecordWordIndex::getAuthor() const
{
    return mRecord ? BasicProfile(mRecord->mAuthor) : BasicProfile();
}

QList<ImageView> RecordWordIndex::getImages() const
{
    if (!mImages.empty())
        return mImages;

    ATProto::AppBskyEmbed::ImagesView::SharedPtr imagesView;
    auto embed = getEmbedView(ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW);

    if (embed)
    {
        imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(embed->mEmbed);
    }
    else
    {
        embed = getEmbedView(ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW);

        if (!embed)
            return {};

        const auto& recordWithMediaView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(embed->mEmbed);

        if (recordWithMediaView->mMediaType != ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW)
            return {};

        imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(recordWithMediaView->mMedia);
    }

    QList<ImageView> images;

    for (const auto& img : imagesView->mImages)
        images.append(ImageView(img));

    return images;
}

VideoView::Ptr RecordWordIndex::getVideoView() const
{
    if (!mVideo.isNull())
        return  std::make_unique<VideoView>(mVideo);

    auto embed = getEmbedView(ATProto::AppBskyEmbed::EmbedViewType::VIDEO_VIEW);

    if (embed)
    {
        const auto& videoView = std::get<ATProto::AppBskyEmbed::VideoView::SharedPtr>(embed->mEmbed);
        return std::make_unique<VideoView>(videoView);
    }

    embed = getEmbedView(ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW);

    if (!embed)
        return {};

    const auto& recordWithMediaView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(embed->mEmbed);

    if (recordWithMediaView->mMediaType != ATProto::AppBskyEmbed::EmbedViewType::VIDEO_VIEW)
        return {};

    const auto& videoView = std::get<ATProto::AppBskyEmbed::VideoView::SharedPtr>(recordWithMediaView->mMedia);
    return std::make_unique<VideoView>(videoView);
}

ExternalView::Ptr RecordWordIndex::getExternalView() const
{
    if (!mExternal.isNull())
        return std::make_unique<ExternalView>(mExternal);

    auto embed = getEmbedView(ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW);

    if (embed)
    {
        const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(embed->mEmbed)->mExternal;
        return std::make_unique<ExternalView>(external);
    }

    embed = getEmbedView(ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW);

    if (!embed)
        return {};

    const auto& recordWithMediaView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(embed->mEmbed);

    if (recordWithMediaView->mMediaType != ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(recordWithMediaView->mMedia)->mExternal;
    return std::make_unique<ExternalView>(external);
}

std::vector<QString> RecordWordIndex::getHashtags() const
{
    if (!mRecord)
        return {};

    if (mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    const auto tags = ATProto::RichTextMaster::getFacetTags(*recordValue);
    std::vector<QString> hashtags;
    hashtags.reserve(tags.size());

    for (const auto& tag : tags)
    {
        if (!tag.startsWith('$'))
            hashtags.push_back(tag);
    }

    return hashtags;
}

std::vector<QString> RecordWordIndex::getCashtags() const
{
    if (!mRecord)
        return {};

    if (mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    const auto tags = ATProto::RichTextMaster::getFacetTags(*recordValue);
    std::vector<QString> cashtags;
    cashtags.reserve(tags.size());

    for (const auto& tag : tags)
    {
        if (tag.startsWith('$'))
            cashtags.push_back(tag);
    }

    return cashtags;
}

std::vector<QString> RecordWordIndex::getAllTags() const
{
    if (!mRecord)
        return {};

    if (mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    const auto tags = ATProto::RichTextMaster::getFacetTags(*recordValue);
    return tags;
}

std::vector<QString> RecordWordIndex::getWebLinks() const
{
    if (!mRecord)
        return {};

    if (mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    auto links = ATProto::RichTextMaster::getFacetLinks(*recordValue);

    if (recordValue->mEmbed && recordValue->mEmbed->mType == ATProto::AppBskyEmbed::EmbedType::EXTERNAL)
    {
        const auto& external = std::get<ATProto::AppBskyEmbed::External::SharedPtr>(recordValue->mEmbed->mEmbed);
        Q_ASSERT(external);
        Q_ASSERT(external->mExternal);

        if (external && external->mExternal)
            links.push_back(external->mExternal->mUri);
    }

    return links;
}

ATProto::AppBskyEmbed::EmbedView::SharedPtr RecordWordIndex::getEmbedView(ATProto::AppBskyEmbed::EmbedViewType embedViewType) const
{
    if (!mRecord || mRecord->mEmbeds.empty())
        return nullptr;

    // There is a list of embeds; can there be more than 1?
    const auto& embed = mRecord->mEmbeds[0];
    if (embed->mType != embedViewType)
        return nullptr;

    return embed;
}

}
