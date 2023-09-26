// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
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

private:
    ATProto::AppBskyFeed::Record::Post* mRecord = nullptr;
};

}
