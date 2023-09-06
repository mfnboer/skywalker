// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "record_view.h"
#include "external_view.h"

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
        qWarning() << "Record type not supported:" << int(view.mRecordType);
        mNotSupported = true;
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
        return std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mRecord->mValue)->mText;

    return {};
}

BasicProfile RecordView::getAuthor() const
{
    return mRecord ? BasicProfile(mRecord->mAuthor.get()) : BasicProfile();
}

QDateTime RecordView::getCreatedAt() const
{
    if (mRecord && mRecord->mValueType == ATProto::RecordType::APP_BSKY_FEED_POST)
        return std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mRecord->mValue)->mCreatedAt;

    return {};
}

qint64 RecordView::getCreatedSecondsAgo() const
{
    const auto duration = QDateTime::currentDateTime() - getCreatedAt();
    return qint64(duration / 1000ms);
}

QList<ImageView> RecordView::getImages() const
{
    if (!mRecord || mRecord->mEmbeds.empty())
        return {};

    // TODO: there is a list of embeds; can there be more than 1?
    const auto& embed = mRecord->mEmbeds[0];
    if (embed->mType != ATProto::AppBskyEmbed::EmbedType::IMAGES_VIEW)
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
    if (embed->mType != ATProto::AppBskyEmbed::EmbedType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::Ptr>(embed->mEmbed)->mExternal;
    return QVariant::fromValue(ExternalView(external.get()));
}

}
