// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft.h"
#include "lexicon.h"
#include <atproto/lib/xjson.h>
#include <unordered_map>

namespace Skywalker::Draft {

QJsonObject ReplyToPost::toJson() const
{
    QJsonObject json;
    Q_ASSERT(mAuthor);

    if (mAuthor)
        json.insert("author", mAuthor->toJson());

    json.insert("text", mText);
    json.insert("date", mDateTime.toString(Qt::ISODateWithMs));
    return json;
}

ReplyToPost::SharedPtr ReplyToPost::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto replyToPost = std::make_shared<ReplyToPost>();
    replyToPost->mAuthor = xjson.getRequiredObject<ATProto::AppBskyActor::ProfileViewBasic>("author");
    replyToPost->mText = xjson.getRequiredString("text");
    replyToPost->mDateTime = xjson.getRequiredDateTime("date");
    return replyToPost;
}

QJsonObject QuotePost::toJson() const
{
    QJsonObject json;
    json.insert("$type", Lexicon::DRAFT_DEFS_QUOTE_POST);
    Q_ASSERT(mAuthor);

    if (mAuthor)
        json.insert("author", mAuthor->toJson());

    json.insert("text", mText);
    json.insert("date", mDateTime.toString(Qt::ISODateWithMs));
    return json;
}

QuotePost::SharedPtr QuotePost::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto quotePost = std::make_shared<QuotePost>();
    quotePost->mAuthor = xjson.getRequiredObject<ATProto::AppBskyActor::ProfileViewBasic>("author");
    quotePost->mText = xjson.getRequiredString("text");
    quotePost->mDateTime = xjson.getRequiredDateTime("date");
    return quotePost;
}

Quote::RecordType Quote::stringToRecordType(const QString& str)
{
    static const std::unordered_map<QString, RecordType> recordMapping = {
        { Lexicon::DRAFT_DEFS_QUOTE_POST, RecordType::QUOTE_POST },
        { "app.bsky.feed.defs#generatorView", RecordType::QUOTE_FEED },
        { "app.bsky.graph.defs#listView", RecordType::QUOTE_LIST },
        { ATProto::AppBskyLabeler::LabelerView::TYPE, RecordType::QUOTE_LABELER }
    };

    const auto it = recordMapping.find(str);
    if (it != recordMapping.end())
        return it->second;

    qWarning() << "Unknown record type:" << str;
    return RecordType::UNKNOWN;
}

QJsonObject Quote::toJson() const
{
    QJsonObject json;

    switch (mRecordType)
    {
    case RecordType::QUOTE_POST:
        json.insert("record", std::get<QuotePost::SharedPtr>(mRecord)->toJson());
        break;
    case RecordType::QUOTE_FEED:
        json.insert("record", std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(mRecord)->toJson());
        break;
    case RecordType::QUOTE_LIST:
        json.insert("record", std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(mRecord)->toJson());
        break;
    case RecordType::QUOTE_LABELER:
        json.insert("record", std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(mRecord)->toJson());
        break;
    case RecordType::UNKNOWN:
        qWarning() << "Unknown record type:" << (int)mRecordType;
        Q_ASSERT(false);
        break;
    }

    return json;
}

Quote::SharedPtr Quote::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto quote = std::make_shared<Quote>();
    const auto recordJson = xjson.getRequiredJsonObject("record");
    const ATProto::XJsonObject recordXJson(recordJson);
    const QString rawRecordType = recordXJson.getRequiredString("$type");
    quote->mRecordType = stringToRecordType(rawRecordType);

    switch (quote->mRecordType)
    {
    case RecordType::QUOTE_POST:
        quote->mRecord = xjson.getRequiredObject<QuotePost>("record");
        break;
    case RecordType::QUOTE_FEED:
        quote->mRecord = xjson.getRequiredObject<ATProto::AppBskyFeed::GeneratorView>("record");
        break;
    case RecordType::QUOTE_LIST:
        quote->mRecord = xjson.getRequiredObject<ATProto::AppBskyGraph::ListView>("record");
        break;
    case RecordType::QUOTE_LABELER:
        quote->mRecord = xjson.getRequiredObject<ATProto::AppBskyLabeler::LabelerView>("record");
        break;
    case RecordType::UNKNOWN:
        qWarning() << "Unknown record type:" << rawRecordType;
        throw ATProto::InvalidJsonException("Unknown record type");
    }

    return quote;
}

QJsonObject EmbeddedLinks::toJson() const
{
    QJsonObject json;
    ATProto::XJsonObject::insertOptionalArray<WebLink>(json, "embeddedLinks", mEmbeddedLinks);
    return json;
}

EmbeddedLinks::SharedPtr EmbeddedLinks::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto links = std::make_shared<EmbeddedLinks>();
    links->mEmbeddedLinks = xjson.getOptionalVector<WebLink>("embeddedLinks");
    return links;
}

QJsonObject Draft::toJson() const
{
    QJsonObject json;
    json.insert("$type", Lexicon::COLLECTION_DRAFT_POST);
    json.insert("post", mPost->toJson());
    ATProto::XJsonObject::insertOptionalJsonObject<ReplyToPost>(json, "replyToPost", mReplyToPost);
    ATProto::XJsonObject::insertOptionalJsonObject<Quote>(json, "quote", mQuote);
    ATProto::XJsonObject::insertOptionalJsonObject<ATProto::AppBskyFeed::Threadgate>(json, "threadgate", mThreadgate);
    json.insert("threadPosts", ATProto::XJsonObject::toJsonArray<ATProto::AppBskyFeed::Record::Post>(mThreadPosts));
    json.insert("embeddingDisabled", mEmbeddingDisabled);
    return json;
}

Draft::SharedPtr Draft::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto draft = std::make_shared<Draft>();
    draft->mPost = xjson.getRequiredObject<ATProto::AppBskyFeed::Record::Post>("post");
    draft->mReplyToPost = xjson.getOptionalObject<ReplyToPost>("replyToPost");
    draft->mQuote = xjson.getOptionalObject<Quote>("quote");
    draft->mThreadgate = xjson.getOptionalObject<ATProto::AppBskyFeed::Threadgate>("threadgate");
    draft->mThreadPosts = xjson.getOptionalVector<ATProto::AppBskyFeed::Record::Post>("threadPosts");
    draft->mEmbeddingDisabled = xjson.getOptionalBool("embeddingDisabled", false);
    return draft;
}

}
