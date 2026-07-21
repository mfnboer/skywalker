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

    if (ATProto::holdsNonNull<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue))
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);

        if (recordValue->mBridgyOriginalText && !recordValue->mBridgyOriginalText->isEmpty())
            return UnicodeFonts::toPlainText(*recordValue->mBridgyOriginalText);
        else
            return recordValue->mText;
    }
    else if (ATProto::holdsNonNull<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(mRecord->mValue))
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(mRecord->mValue);
        return recordValue->mDescription.value_or("");
    }
    else if (ATProto::holdsNonNull<ATProto::AppBskyGraph::ListView::SharedPtr>(mRecord->mValue))
    {
        const auto& recordValue = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(mRecord->mValue);
        return recordValue->mDescription.value_or("");
    }
    else if (ATProto::holdsNonNull<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(mRecord->mValue))
    {
        const auto& recordValue = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(mRecord->mValue);
        return recordValue->mCreator->mDescription.value_or("");
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
    ATProto::AppBskyEmbed::GalleryView::SharedPtr galleryView;
    auto* imagesEmbed = getEmbedView<ATProto::AppBskyEmbed::ImagesView::SharedPtr>();
    auto* galleryEmbed = getEmbedView<ATProto::AppBskyEmbed::GalleryView::SharedPtr>();

    if (imagesEmbed)
    {
        imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(*imagesEmbed);
    }
    else if (galleryEmbed)
    {
        galleryView = std::get<ATProto::AppBskyEmbed::GalleryView::SharedPtr>(*galleryEmbed);
    }
    else
    {
        auto* recordWithMediaEmbed = getEmbedView<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>();

        if (recordWithMediaEmbed)
        {
            const auto& recordWithMediaView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(*recordWithMediaEmbed);

            if (ATProto::holdsNonNull<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(recordWithMediaView->mMedia))
                imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(recordWithMediaView->mMedia);
            else if (ATProto::holdsNonNull<ATProto::AppBskyEmbed::GalleryView::SharedPtr>(recordWithMediaView->mMedia))
                galleryView = std::get<ATProto::AppBskyEmbed::GalleryView::SharedPtr>(recordWithMediaView->mMedia);
        }
    }

    QList<ImageView> images;

    if (imagesView)
    {
        for (const auto& img : imagesView->mImages)
            images.append(ImageView(img));
    }

    if (galleryView)
    {
        for (const auto& item : galleryView->mItems)
        {
            const auto* viewImage = std::get_if<ATProto::AppBskyEmbed::GalleryViewImage::SharedPtr>(&item);

            if (viewImage)
                images.append(ImageView(*viewImage));
        }
    }

    return images;
}

VideoView::Ptr RecordWordIndex::getVideoView() const
{
    if (!mVideo.isNull())
        return  std::make_unique<VideoView>(mVideo);

    auto* embed = getEmbedView<ATProto::AppBskyEmbed::VideoView::SharedPtr>();

    if (embed)
    {
        const auto& videoView = std::get<ATProto::AppBskyEmbed::VideoView::SharedPtr>(*embed);
        return std::make_unique<VideoView>(videoView);
    }

    embed = getEmbedView<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>();

    if (!embed)
        return {};

    const auto& recordWithMediaView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(*embed);

    if (!ATProto::holdsNonNull<ATProto::AppBskyEmbed::VideoView::SharedPtr>(recordWithMediaView->mMedia))
        return {};

    const auto& videoView = std::get<ATProto::AppBskyEmbed::VideoView::SharedPtr>(recordWithMediaView->mMedia);
    return std::make_unique<VideoView>(videoView);
}

ExternalView::Ptr RecordWordIndex::getExternalView() const
{
    if (!mExternal.isNull())
        return std::make_unique<ExternalView>(mExternal);

    auto* embed = getEmbedView<ATProto::AppBskyEmbed::ExternalView::SharedPtr>();

    if (embed)
    {
        const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(*embed)->mExternal;
        return std::make_unique<ExternalView>(external);
    }

    embed = getEmbedView<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>();

    if (!embed)
        return {};

    const auto& recordWithMediaView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(*embed);

    if (!ATProto::holdsNonNull<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(recordWithMediaView->mMedia))
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(recordWithMediaView->mMedia)->mExternal;
    return std::make_unique<ExternalView>(external);
}

std::vector<QString> RecordWordIndex::getHashtags() const
{
    if (!mRecord)
        return {};

    if (!ATProto::holdsNonNull<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue))
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

    if (!ATProto::holdsNonNull<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue))
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

    if (!ATProto::holdsNonNull<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue))
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    const auto tags = ATProto::RichTextMaster::getFacetTags(*recordValue);
    return tags;
}

std::vector<QString> RecordWordIndex::getWebLinks() const
{
    if (!mRecord)
        return {};

    if (!ATProto::holdsNonNull<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue))
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    auto links = ATProto::RichTextMaster::getFacetLinks(*recordValue);

    if (recordValue->mEmbed && ATProto::holdsNonNull<ATProto::AppBskyEmbed::External::SharedPtr>(*recordValue->mEmbed))
    {
        const auto& external = std::get<ATProto::AppBskyEmbed::External::SharedPtr>(*recordValue->mEmbed);
        Q_ASSERT(external);
        Q_ASSERT(external->mExternal);

        if (external && external->mExternal)
            links.push_back(external->mExternal->mUri);
    }

    return links;
}

template <typename ViewType>
ATProto::AppBskyEmbed::EmbedViewUnion* RecordWordIndex::getEmbedView() const
{
    if (!mRecord || mRecord->mEmbeds.empty())
        return nullptr;

    // There is a list of embeds; can there be more than 1?
    auto& embed = mRecord->mEmbeds[0];

    if (!ATProto::holdsNonNull<ViewType>(embed))
        return nullptr;

    return &embed;
}

}
