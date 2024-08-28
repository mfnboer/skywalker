// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_record.h"
#include "post_utils.h"
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

ATProto::ComATProtoRepo::StrongRef::SharedPtr PostRecord::getReplyToRef() const
{
    if (!mRecord || !mRecord->mReply)
        return nullptr;

    return std::make_shared<ATProto::ComATProtoRepo::StrongRef>(*mRecord->mReply->mParent);
}

ATProto::ComATProtoRepo::StrongRef::SharedPtr PostRecord::getReplyRootRef() const
{
    if (!mRecord || !mRecord->mReply)
        return nullptr;

    return std::make_shared<ATProto::ComATProtoRepo::StrongRef>(*mRecord->mReply->mRoot);
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

QString PostRecord::getReplyRootAuthorDid() const
{
    const auto replyRootRef = getReplyRootRef();
    if (!replyRootRef)
        return {};

    return PostUtils::extractDidFromUri(replyRootRef->mUri);
}

QString PostRecord::getReplyToAuthorDid() const
{
    const auto replyToRef = getReplyToRef();
    if (!replyToRef)
        return {};

    return PostUtils::extractDidFromUri(replyToRef->mUri);
}

BasicProfile PostRecord::getReplyToAuthor() const
{
    const auto did = getReplyToAuthorDid();

    if (did.isEmpty())
        return {};

    auto* author = AuthorCache::instance().get(did);

    if (!author)
        return {};

    return *author;
}

const LanguageList& PostRecord::getLanguages() const
{
    if (!mLanguages.empty())
        return mLanguages;

    if (!mRecord)
        return mLanguages;

    const_cast<PostRecord*>(this)->mLanguages = LanguageUtils::getLanguages(mRecord->mLanguages);
    return mLanguages;
}

bool PostRecord::hasLanguage() const
{
    if (!mRecord)
        return false;

    return !mRecord->mLanguages.empty();
}

bool PostRecord::isReply() const
{
    return getReplyRootRef() != nullptr;
}

bool PostRecord::hasEmbeddedContent() const
{
    return mRecord ? mRecord->mEmbed != nullptr : false;
}

}
