// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_embed.h>

namespace Skywalker {

class PostRecord
{
public:
    PostRecord() = default;
    explicit PostRecord(ATProto::AppBskyFeed::Record::Post* record);

    bool isNull() const;
    QString getText() const;
    QString getFormattedText() const;
    ATProto::ComATProtoRepo::StrongRef::Ptr getReplyToRef() const;
    ATProto::ComATProtoRepo::StrongRef::Ptr getReplyRootRef() const;
    QString getReplyRootCid() const;
    QString getReplyRootUri() const;
    BasicProfile getReplyToAuthor() const;
    bool isReply() const;
    bool hasEmbeddedContent() const;

private:
    ATProto::AppBskyFeed::Record::Post* mRecord = nullptr;
};

}
