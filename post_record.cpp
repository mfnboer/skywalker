// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_record.h"
#include "abstract_post_feed_model.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

PostRecord::PostRecord(ATProto::AppBskyFeed::Record::Post* record) :
    mRecord(record)
{
    Q_ASSERT(mRecord);
}

bool PostRecord::isNull() const
{
    return mRecord == nullptr;
}

QString PostRecord::getText() const
{
    return mRecord ? mRecord->mText : QString();
}

QString PostRecord::getFormattedText() const
{
    if (!mRecord)
        return {};

    return ATProto::PostMaster::getFormattedPostText(*mRecord);
}

ATProto::ComATProtoRepo::StrongRef::Ptr PostRecord::getReplyToRef() const
{
    if (!mRecord || !mRecord->mReply)
        return nullptr;

    return std::make_unique<ATProto::ComATProtoRepo::StrongRef>(*mRecord->mReply->mParent);
}

ATProto::ComATProtoRepo::StrongRef::Ptr PostRecord::getReplyRootRef() const
{
    if (!mRecord || !mRecord->mReply)
        return nullptr;

    return std::make_unique<ATProto::ComATProtoRepo::StrongRef>(*mRecord->mReply->mRoot);
}

BasicProfile PostRecord::getReplyToAuthor() const
{
    const auto replyToRef = getReplyToRef();
    if (!replyToRef)
        return {};

    ATProto::ATUri atUri(replyToRef->mUri);
    if (!atUri.isValid())
    {
        qWarning() << "Not a valid at-uri:" << replyToRef->mUri;
        return {};
    }

    const auto did = atUri.getAuthority();
    const auto& authorCache = AbstractPostFeedModel::getAuthorCache();
    auto* author = authorCache[did];

    if (!author)
        return {};

    return author->getProfile();
}

}
