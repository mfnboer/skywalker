// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_record.h"
#include "author_cache.h"
#include "user_settings.h"
#include <atproto/lib/at_uri.h>
#include <atproto/lib/rich_text_master.h>

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

    return ATProto::RichTextMaster::getFormattedPostText(*mRecord, UserSettings::getLinkColor());
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

QString PostRecord::getReplyRootCid() const
{
    const auto& ref = getReplyRootRef();
    return ref ? ref->mCid : QString();
}

QString PostRecord::getReplyRootUri() const
{
    const auto& ref = getReplyRootRef();
    return ref ? ref->mUri : QString();
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
    auto* author = AuthorCache::instance().get(did);

    if (!author)
        return {};

    return *author;
}

bool PostRecord::hasEmbeddedContent() const
{
    return mRecord ? mRecord->mEmbed != nullptr : false;
}

}
