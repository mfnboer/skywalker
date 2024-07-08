// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "record_view.h"
#include "author_cache.h"
#include "content_filter.h"
#include "external_view.h"
#include "user_settings.h"
#include <atproto/lib/at_uri.h>
#include <atproto/lib/rich_text_master.h>

using namespace std::chrono_literals;

namespace Skywalker {

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
    case ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD:
    {
        const auto& record = std::get<ATProto::AppBskyEmbed::RecordViewRecord::SharedPtr>(view.mRecord);
        mRecord = record.get();
        break;
    }
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
    {
        const auto& record = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(view.mRecord);
        mFeed = record.get();
        break;
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
    {
        const auto& record = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(view.mRecord);
        mList = record.get();
        break;
    }
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
    {
        const auto& record = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(view.mRecord);
        mLabeler = record.get();
        break;
    }
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

        return ATProto::RichTextMaster::getFormattedPostText(*recordValue, UserSettings::getLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedFeedDescription(*recordValue, UserSettings::getLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedListDescription(*recordValue, UserSettings::getLinkColor());
    }
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
    {
        const auto& recordValue = std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(mRecord->mValue);
        return ATProto::RichTextMaster::getFormattedLabelerDescription(*recordValue, UserSettings::getLinkColor());

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

QList<ImageView> RecordView::getImages() const
{
    if (!mRecord || mRecord->mEmbeds.empty())
        return {};

    // TODO: there is a list of embeds; can there be more than 1?
    const auto& embed = mRecord->mEmbeds[0];
    if (embed->mType != ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(embed->mEmbed);
    QList<ImageView> images;

    for (const auto& img : imagesView->mImages)
        images.append(ImageView(img.get()));

    return images;
}

QVariant RecordView::getExternal() const
{
    if (!mRecord || mRecord->mEmbeds.empty())
        return {};

    // TODO: there is a list of embeds; can there be more than 1?
    const auto& embed = mRecord->mEmbeds[0];
    if (embed->mType != ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(embed->mEmbed)->mExternal;
    return QVariant::fromValue(ExternalView(external.get()));
}

ContentLabelList RecordView::getContentLabels() const
{
    if (!mRecord)
        return {};

    return ContentFilter::getContentLabels(mRecord->mLabels);
}

const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>& RecordView::getLabels() const
{
    static const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr> NO_LABELS;
    if (!mRecord)
        return NO_LABELS;

    return mRecord->mLabels;
}

ContentLabelList RecordView::getLabelsIncludingAuthorLabels() const
{
    const auto author = getAuthor();
    ContentLabelList contentLabels = author.getContentLabels();
    ContentFilter::addContentLabels(contentLabels, getLabels());
    return contentLabels;
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

BasicProfile RecordView::getReplyToAuthor() const
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

    const auto did = atUri.getAuthority();
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

    return LabelerView(*mLabeler);
}

}
