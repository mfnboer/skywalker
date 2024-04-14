// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <atproto/lib/lexicon/app_bsky_feed.h>
#include <atproto/lib/lexicon/app_bsky_graph.h>
#include <QJsonObject>

namespace Skywalker::Draft {

struct ReplyToPost
{
    ATProto::AppBskyActor::ProfileViewBasic::Ptr mAuthor;
    QString mText;
    QDateTime mDateTime;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ReplyToPost>;
    static Ptr fromJson(const QJsonObject& json);
};

struct QuotePost
{
    ATProto::AppBskyActor::ProfileViewBasic::Ptr mAuthor;
    QString mText;
    QDateTime mDateTime;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<QuotePost>;
    static Ptr fromJson(const QJsonObject& json);
};

struct Quote
{
    enum class RecordType
    {
        QUOTE_POST,
        QUOTE_FEED,
        QUOTE_LIST,
        UNKNOWN
    };

    static RecordType stringToRecordType(const QString& str);

    RecordType mRecordType;

    std::variant<QuotePost::Ptr,
                 ATProto::AppBskyFeed::GeneratorView::Ptr,
                 ATProto::AppBskyGraph::ListView::Ptr> mRecord;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Quote>;
    static Ptr fromJson(const QJsonObject& json);
};

struct Draft
{
    ATProto::AppBskyFeed::Record::Post::Ptr mPost;
    ReplyToPost::Ptr mReplyToPost;
    Quote::Ptr mQuote;
    ATProto::AppBskyFeed::Threadgate::Ptr mThreadgate;
    std::vector<ATProto::AppBskyFeed::Record::Post::Ptr> mThreadPosts;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Draft>;
    static Ptr fromJson(const QJsonObject& json);
};

using DraftList = std::vector<Draft::Ptr>;

}
