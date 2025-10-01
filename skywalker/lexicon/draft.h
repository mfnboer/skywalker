// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "web_link.h"
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <atproto/lib/lexicon/app_bsky_feed.h>
#include <atproto/lib/lexicon/app_bsky_graph.h>
#include <QJsonObject>

namespace Skywalker::Draft {

struct ReplyToPost
{
    ATProto::AppBskyActor::ProfileViewBasic::SharedPtr mAuthor;
    QString mText;
    QDateTime mDateTime;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ReplyToPost>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct QuotePost
{
    ATProto::AppBskyActor::ProfileViewBasic::SharedPtr mAuthor;
    QString mText;
    QDateTime mDateTime;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<QuotePost>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct Quote
{
    enum class RecordType
    {
        QUOTE_POST,
        QUOTE_FEED,
        QUOTE_LIST,
        QUOTE_LABELER,
        UNKNOWN
    };

    static RecordType stringToRecordType(const QString& str);

    RecordType mRecordType = RecordType::UNKNOWN;

    std::variant<QuotePost::SharedPtr,
                 ATProto::AppBskyFeed::GeneratorView::SharedPtr,
                 ATProto::AppBskyGraph::ListView::SharedPtr,
                 ATProto::AppBskyLabeler::LabelerView::SharedPtr> mRecord;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Quote>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct EmbeddedLinks
{
    std::vector<WebLink::SharedPtr> mEmbeddedLinks;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<EmbeddedLinks>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct Draft
{
    ATProto::AppBskyFeed::Record::Post::SharedPtr mPost;
    ReplyToPost::SharedPtr mReplyToPost;
    Quote::SharedPtr mQuote;
    ATProto::AppBskyFeed::Threadgate::SharedPtr mThreadgate;
    std::vector<ATProto::AppBskyFeed::Record::Post::SharedPtr> mThreadPosts;
    bool mEmbeddingDisabled = false;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Draft>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using DraftList = std::vector<Draft::SharedPtr>;

}
