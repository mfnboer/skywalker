// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "record_view.h"
#include "author_cache.h"
#include "content_filter.h"
#include "external_view.h"
#include "post_utils.h"
#include "unicode_fonts.h"
#include "user_settings.h"
#include "video_view.h"
#include <atproto/lib/at_uri.h>
#include <atproto/lib/rich_text_master.h>

using namespace std::chrono_literals;

namespace Skywalker {

RecordView::SharedPtr RecordView::makeDetachedRecord(const QString postUri)
{
    auto record = std::make_shared<RecordView>();
    record->mDetachedPostUri = postUri;
    record->mDetachedByDid = PostUtils::extractDidFromUri(postUri);
    record->mDetached = true;
    record->mValid = true;

    return record;
}

RecordView::RecordView(const ATProto::AppBskyEmbed::RecordView& view)
{
    switch (view.mRecordType)
    {
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND:
    {
        mNotFound = true;
        break;
    }
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_BLOCKED:
    {
        mBlocked = true;
        break;
    }
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_DETACHED:
    {
        const auto record = std::get<ATProto::AppBskyEmbed::RecordViewDetached::SharedPtr>(view.mRecord);
        mDetachedByDid = PostUtils::extractDidFromUri(record->mUri);
        mDetachedPostUri = record->mUri;
        mDetached = true;
        break;
    }
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD:
    {
        mRecord = std::get<ATProto::AppBskyEmbed::RecordViewRecord::SharedPtr>(view.mRecord);
        break;
    }
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
    {
        mFeed = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(view.mRecord);
        break;
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
    {
        mList = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(view.mRecord);
        break;
    }
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
    {
        mLabeler = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(view.mRecord);
        break;
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_STARTER_PACK_VIEW_BASIC:
        mStarterPack = std::get<ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr>(view.mRecord);
        break;
    default:
        qWarning() << "Record type not supported:" << view.mUnsupportedType;
        mNotSupported = true;
        mUnsupportedType = view.mUnsupportedType;
        break;
    }

    mValid = true;
}

QString RecordView::getUri() const
{
    return mRecord ? mRecord->mUri : QString();
}

QString RecordView::getCid() const
{
    return mRecord ? mRecord->mCid : QString();
}

QString RecordView::getText() const
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

QString RecordView::getFormattedText() const
{
    if (!mRecord)
        return {};

    switch (mRecord->mValueType)
    {
    case ATProto::RecordType::APP_BSKY_FEED_POST:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);

        if (recordValue->mBridgyOriginalText && !recordValue->mBridgyOriginalText->isEmpty())
            return *recordValue->mBridgyOriginalText;

        return ATProto::RichTextMaster::getFormattedPostText(*recordValue, UserSettings::getCurrentLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedFeedDescription(*recordValue, UserSettings::getCurrentLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedListDescription(*recordValue, UserSettings::getCurrentLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedLabelerDescription(*recordValue, UserSettings::getCurrentLinkColor());

    }
    default:
        break;
    }

    return {};
}

BasicProfile RecordView::getAuthor() const
{
    return mRecord ? BasicProfile(mRecord->mAuthor) : BasicProfile();
}

QDateTime RecordView::getIndexedAt() const
{
    return mRecord ? mRecord->mIndexedAt : QDateTime();

    return {};
}

ATProto::AppBskyEmbed::EmbedView::SharedPtr RecordView::getEmbedView(ATProto::AppBskyEmbed::EmbedViewType embedViewType) const
{
    if (!mRecord || mRecord->mEmbeds.empty())
        return nullptr;

    // TODO: there is a list of embeds; can there be more than 1?
    const auto& embed = mRecord->mEmbeds[0];
    if (embed->mType != embedViewType)
        return nullptr;

    return embed;
}

QList<ImageView> RecordView::getImages() const
{
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

QVariant RecordView::getVideo() const
{
    auto videoView = getVideoView();

    if (!videoView)
        return {};

    return QVariant::fromValue(*videoView.release());
}

VideoView::Ptr RecordView::getVideoView() const
{
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

QVariant RecordView::getExternal() const
{
    auto externalView = getExternalView();

    if (!externalView)
        return {};

    return QVariant::fromValue(*externalView.release());
}

ExternalView::Ptr RecordView::getExternalView() const
{
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

const ContentLabelList& RecordView::getContentLabels() const
{
    if (!mRecord)
    {
        static const ContentLabelList NO_LABELS;
        return NO_LABELS;
    }

    if (mContentLabels)
        return *mContentLabels;

    const_cast<RecordView*>(this)->mContentLabels = ContentFilter::getContentLabels(mRecord->mLabels);
    return *mContentLabels;
}

const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>& RecordView::getLabels() const
{
    static const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr> NO_LABELS;
    if (!mRecord)
        return NO_LABELS;

    return mRecord->mLabels;
}

const ContentLabelList& RecordView::getLabelsIncludingAuthorLabels() const
{
    if (mLabelsIncludingAuthorLabels)
        return *mLabelsIncludingAuthorLabels;

    const auto& author = getAuthor();
    ContentLabelList contentLabels = author.getContentLabels();
    ContentFilter::addContentLabels(contentLabels, getLabels());
    const_cast<RecordView*>(this)->mLabelsIncludingAuthorLabels = contentLabels;

    return *mLabelsIncludingAuthorLabels;
}

bool RecordView::isReply() const
{
    if (!mRecord)
        return false;

    if (mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return false;

    const auto& post = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    return post->mReply != nullptr;
}

QString RecordView::getReplyToAuthorDid() const
{
    if (!mRecord)
        return {};

    if (mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& post = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);

    if (!post->mReply)
        return {};

    const ATProto::ATUri atUri(post->mReply->mParent->mUri);

    if (!atUri.isValid())
        return {};

    if (atUri.authorityIsHandle())
        return {};

    return atUri.getAuthority();
}

BasicProfile RecordView::getReplyToAuthor() const
{   
    const auto did = getReplyToAuthorDid();

    if (did.isEmpty())
        return {};

    auto* profile = AuthorCache::instance().get(did);
    return profile ? *profile : BasicProfile();
}

const LanguageList& RecordView::getLanguages() const
{
    if (!mLanguages.empty())
        return mLanguages;

    if (!mRecord)
        return mLanguages;

    if (mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return mLanguages;

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    const_cast<RecordView*>(this)->mLanguages = LanguageUtils::getLanguages(recordValue->mLanguages);
    return mLanguages;
}

std::vector<QString> RecordView::getHashtags() const
{
    if (!mRecord)
        return {};

    if (mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mRecord->mValue);
    return ATProto::RichTextMaster::getFacetTags(*recordValue);
}

void RecordView::setMutedReason(const IMatchWords& mutedWords)
{
    if (getAuthor().getViewer().isMuted())
        setMutedReason(QEnums::MUTED_POST_AUTHOR);
    else if (mutedWords.match(*this))
        setMutedReason(QEnums::MUTED_POST_WORDS);
    else
        setMutedReason(QEnums::MUTED_POST_NONE);
}

GeneratorView RecordView::getFeed() const
{
    if (!mFeed)
        return {};

    return GeneratorView(mFeed);
}

ListView RecordView::getList() const
{
    if (!mList)
        return {};

    return ListView(mList);
}

LabelerView RecordView::getLabeler() const
{
    if (!mLabeler)
        return {};

    return LabelerView(mLabeler);
}

StarterPackViewBasic RecordView::getStarterPack() const
{
    if (!mStarterPack)
        return {};

    return StarterPackViewBasic(mStarterPack);
}

}
