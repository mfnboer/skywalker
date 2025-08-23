// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "record_view.h"
#include "author_cache.h"
#include "content_filter.h"
#include "external_view.h"
#include "post_thread_cache.h"
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
    record->mPrivate->mDetachedPostUri = postUri;
    record->mPrivate->mDetachedByDid = PostUtils::extractDidFromUri(postUri);
    record->mPrivate->mDetached = true;
    record->mValid = true;

    return record;
}

RecordView::RecordView(const ATProto::AppBskyEmbed::RecordView& view)
{
    switch (view.mRecordType)
    {
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND:
    {
        mPrivate->mNotFound = true;
        break;
    }
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_BLOCKED:
    {
        mPrivate->mBlocked = true;
        break;
    }
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_DETACHED:
    {
        const auto record = std::get<ATProto::AppBskyEmbed::RecordViewDetached::SharedPtr>(view.mRecord);
        mPrivate->mDetachedByDid = PostUtils::extractDidFromUri(record->mUri);
        mPrivate->mDetachedPostUri = record->mUri;
        mPrivate->mDetached = true;
        break;
    }
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD:
    {
        mPrivate->mRecord = std::get<ATProto::AppBskyEmbed::RecordViewRecord::SharedPtr>(view.mRecord);
        break;
    }
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
    {
        mPrivate->mFeed = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(view.mRecord);
        break;
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
    {
        mPrivate->mList = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(view.mRecord);
        break;
    }
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
    {
        mPrivate->mLabeler = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(view.mRecord);
        break;
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_STARTER_PACK_VIEW_BASIC:
        mPrivate->mStarterPack = std::get<ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr>(view.mRecord);
        break;
    default:
        qWarning() << "Record type not supported:" << view.mUnsupportedType;
        mPrivate->mNotSupported = true;
        mPrivate->mUnsupportedType = view.mUnsupportedType;
        break;
    }

    mValid = true;
}

QString RecordView::getUri() const
{
    return mPrivate->mRecord ? mPrivate->mRecord->mUri : QString();
}

QString RecordView::getCid() const
{
    return mPrivate->mRecord ? mPrivate->mRecord->mCid : QString();
}

QString RecordView::getText() const
{
    if (!mPrivate->mRecord)
        return {};

    switch (mPrivate->mRecord->mValueType)
    {
    case ATProto::RecordType::APP_BSKY_FEED_POST:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPrivate->mRecord->mValue);

        if (recordValue->mBridgyOriginalText && !recordValue->mBridgyOriginalText->isEmpty())
            return UnicodeFonts::toPlainText(*recordValue->mBridgyOriginalText);
        else
            return recordValue->mText;
    }
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(mPrivate->mRecord->mValue);
        return recordValue->mDescription.value_or("");
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(mPrivate->mRecord->mValue);
        return recordValue->mDescription.value_or("");
    }
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(mPrivate->mRecord->mValue);
        return recordValue->mCreator->mDescription.value_or("");
    }
    default:
        break;
    }

    return {};
}

QString RecordView::getFormattedText() const
{
    if (!mPrivate->mRecord)
        return {};

    switch (mPrivate->mRecord->mValueType)
    {
    case ATProto::RecordType::APP_BSKY_FEED_POST:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPrivate->mRecord->mValue);

        if (recordValue->mBridgyOriginalText && !recordValue->mBridgyOriginalText->isEmpty())
            return *recordValue->mBridgyOriginalText;

        return ATProto::RichTextMaster::getFormattedPostText(*recordValue, UserSettings::getCurrentLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(mPrivate->mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedFeedDescription(*recordValue, UserSettings::getCurrentLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(mPrivate->mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedListDescription(*recordValue, UserSettings::getCurrentLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(mPrivate->mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedLabelerDescription(*recordValue, UserSettings::getCurrentLinkColor());

    }
    default:
        break;
    }

    return {};
}

BasicProfile RecordView::getAuthor() const
{
    return mPrivate->mRecord ? BasicProfile(mPrivate->mRecord->mAuthor) : BasicProfile();
}

QDateTime RecordView::getIndexedAt() const
{
    return mPrivate->mRecord ? mPrivate->mRecord->mIndexedAt : QDateTime();

    return {};
}

ATProto::AppBskyEmbed::EmbedView::SharedPtr RecordView::getEmbedView(ATProto::AppBskyEmbed::EmbedViewType embedViewType) const
{
    if (!mPrivate->mRecord || mPrivate->mRecord->mEmbeds.empty())
        return nullptr;

    // There is a list of embeds; can there be more than 1?
    const auto& embed = mPrivate->mRecord->mEmbeds[0];
    if (embed->mType != embedViewType)
        return nullptr;

    return embed;
}

bool RecordView::hasUnknownEmbed() const
{
    auto embed = getEmbedView(ATProto::AppBskyEmbed::EmbedViewType::UNKNOWN);
    return embed != nullptr;
}

QString RecordView::getUnknownEmbedType() const
{
    auto embed = getEmbedView(ATProto::AppBskyEmbed::EmbedViewType::UNKNOWN);

    if (!embed)
        return {};

    return embed->mRawType;
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

    // TODO: should pointer not be deleted?
    // return QVariant::fromValue(*externalView.release());
    qDebug() << "TEST EXTERNAL VIEW";
    return QVariant::fromValue(*externalView);
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
    if (!mPrivate->mRecord)
    {
        static const ContentLabelList NO_LABELS;
        return NO_LABELS;
    }

    if (mPrivate->mContentLabels)
        return *mPrivate->mContentLabels;

    const_cast<RecordView*>(this)->mPrivate->mContentLabels = ContentFilter::getContentLabels(mPrivate->mRecord->mLabels);
    return *mPrivate->mContentLabels;
}

const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>& RecordView::getLabels() const
{
    static const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr> NO_LABELS;
    if (!mPrivate->mRecord)
        return NO_LABELS;

    return mPrivate->mRecord->mLabels;
}

const ContentLabelList& RecordView::getLabelsIncludingAuthorLabels() const
{
    if (mPrivate->mLabelsIncludingAuthorLabels)
        return *mPrivate->mLabelsIncludingAuthorLabels;

    const auto& author = getAuthor();
    ContentLabelList contentLabels = author.getContentLabels();
    ContentFilter::addContentLabels(contentLabels, getLabels());
    mPrivate->mLabelsIncludingAuthorLabels = contentLabels;

    return *mPrivate->mLabelsIncludingAuthorLabels;
}

bool RecordView::isReply() const
{
    if (!mPrivate->mRecord)
        return false;

    if (mPrivate->mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return false;

    const auto& post = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPrivate->mRecord->mValue);
    return post->mReply != nullptr;
}

QEnums::TripleBool RecordView::isThread() const
{
    if (!mPrivate->mRecord)
        return QEnums::TRIPLE_BOOL_NO;

    if (mPrivate->mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return QEnums::TRIPLE_BOOL_NO;;

    if (isReply())
        return QEnums::TRIPLE_BOOL_NO;

    const QString uri = getUri();

    if (uri.isEmpty())
        return QEnums::TRIPLE_BOOL_NO;

    const bool* postIsThread = PostThreadCache::instance().getIsThread(uri);

    if (postIsThread == nullptr)
        return QEnums::TRIPLE_BOOL_UNKNOWN;

    return *postIsThread ? QEnums::TRIPLE_BOOL_YES : QEnums::TRIPLE_BOOL_NO;
}

QString RecordView::getReplyToAuthorDid() const
{
    if (!mPrivate->mRecord)
        return {};

    if (mPrivate->mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& post = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPrivate->mRecord->mValue);

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
    if (!mPrivate->mLanguages.empty())
        return mPrivate->mLanguages;

    if (!mPrivate->mRecord)
        return mPrivate->mLanguages;

    if (mPrivate->mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return mPrivate->mLanguages;

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPrivate->mRecord->mValue);
    const_cast<RecordView*>(this)->mPrivate->mLanguages = LanguageUtils::getLanguages(recordValue->mLanguages);
    return mPrivate->mLanguages;
}

std::vector<QString> RecordView::getHashtags() const
{
    if (!mPrivate->mRecord)
        return {};

    if (mPrivate->mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPrivate->mRecord->mValue);
    return ATProto::RichTextMaster::getFacetTags(*recordValue);
}

std::vector<QString> RecordView::getWebLinks() const
{
    if (!mPrivate->mRecord)
        return {};

    if (mPrivate->mRecord->mValueType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPrivate->mRecord->mValue);
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
    if (!mPrivate->mFeed)
        return {};

    return GeneratorView(mPrivate->mFeed);
}

ListView RecordView::getList() const
{
    if (!mPrivate->mList)
        return {};

    return ListView(mPrivate->mList);
}

LabelerView RecordView::getLabeler() const
{
    if (!mPrivate->mLabeler)
        return {};

    return LabelerView(mPrivate->mLabeler);
}

StarterPackViewBasic RecordView::getStarterPack() const
{
    if (!mPrivate->mStarterPack)
        return {};

    return StarterPackViewBasic(mPrivate->mStarterPack);
}

}
