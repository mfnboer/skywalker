// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "record_view.h"
#include "content_filter.h"
#include "external_view.h"
#include "user_settings.h"
#include <atproto/lib/post_master.h>

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
        const auto& record = std::get<ATProto::AppBskyEmbed::RecordViewRecord::Ptr>(view.mRecord);
        mRecord = record.get();
        break;
    }
    default:
        qWarning() << "Record type not supported:" << view.mUnsupportedType;
        mNotSupported = true;
        mUnsupportedType = view.mUnsupportedType;
        break;
    }
}

QString RecordView::getUri() const
{
    return mRecord ? mRecord->mUri : QString();
}

QString RecordView::getText() const
{
    if (mRecord && mRecord->mValueType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mRecord->mValue);
        return recordValue->mText;
    }

    return {};
}

QString RecordView::getFormattedText() const
{
    if (mRecord && mRecord->mValueType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& recordValue = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mRecord->mValue);
        return ATProto::PostMaster::getFormattedPostText(*recordValue, UserSettings::getLinkColor());
    }

    return {};
}

BasicProfile RecordView::getAuthor() const
{
    return mRecord ? BasicProfile(mRecord->mAuthor.get()) : BasicProfile();
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

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::Ptr>(embed->mEmbed);
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

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::Ptr>(embed->mEmbed)->mExternal;
    return QVariant::fromValue(ExternalView(external.get()));
}

QStringList RecordView::getLabelTexts() const
{
    if (!mRecord)
        return {};

    return ContentFilter::getLabelTexts(mRecord->mLabels);
}

const std::vector<ATProto::ComATProtoLabel::Label::Ptr>& RecordView::getLabels() const
{
    static const std::vector<ATProto::ComATProtoLabel::Label::Ptr> NO_LABELS;
    if (!mRecord)
        return NO_LABELS;

    return mRecord->mLabels;
}

void RecordView::setMutedReason(const MutedWords& mutedWords)
{
    if (getAuthor().getViewer().isMuted())
        setMutedReason(QEnums::MUTED_POST_AUTHOR);
    else if (mutedWords.match(*this))
        setMutedReason(QEnums::MUTED_POST_WORDS);
    else
        setMutedReason(QEnums::MUTED_POST_NONE);
}

}
