// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "language_utils.h"
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
    ATProto::ComATProtoRepo::StrongRef::SharedPtr getReplyToRef() const;
    ATProto::ComATProtoRepo::StrongRef::SharedPtr getReplyRootRef() const;
    QString getReplyRootCid() const;
    QString getReplyRootUri() const;
    QString getReplyRootAuthorDid() const;
    QString getReplyToAuthorDid() const;
    BasicProfile getReplyToAuthor() const;
    const LanguageList& getLanguages() const;
    bool hasLanguage() const;
    QStringList getMentionDids() const;
    bool isReply() const;
    bool hasEmbeddedContent() const;
    ATProto::AppBskyEmbed::Images::SharedPtr getImages() const;
    ATProto::AppBskyEmbed::Video::SharedPtr getVideo() const;
    ATProto::AppBskyEmbed::External::SharedPtr getExternal() const;

private:
    ATProto::AppBskyFeed::Record::Post* mRecord = nullptr;
    LanguageList mLanguages;
};

}
