// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include "list_view.h"
#include "profile.h"
#include <atproto/lib/post_master.h>
#include <QJsonDocument>
#include <QString>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class DraftPosts : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DraftPosts(QObject* parent = nullptr);

    Q_INVOKABLE void saveDraftPost(const QString& text,
                                   const QStringList& imageFileNames, const QStringList& altTexts,
                                   const QString& replyToUri, const QString& replyToCid,
                                   const QString& replyRootUri, const QString& replyRootCid,
                                   const BasicProfile& replyToAuthor, const QString& replyToText,
                                   const QDateTime& replyToDateTime,
                                   const QString& quoteUri, const QString& quoteCid,
                                   const BasicProfile& quoteAuthor, const QString& quoteText,
                                   const QDateTime& quoteDateTime,
                                   const GeneratorView& quoteFeed, const ListView& quoteList,
                                   const QStringList& labels);

signals:
    void saveDraftPostOk();
    void saveDraftPostFailed(QString error);

private:
    struct ReplyToPost
    {
        ATProto::AppBskyActor::ProfileViewBasic::Ptr mAuthor;
        QString mText;
        QDateTime mDateTime;

        QJsonObject toJson() const;

        using Ptr = std::unique_ptr<ReplyToPost>;
    };

    struct QuotePost
    {
        ATProto::AppBskyActor::ProfileViewBasic::Ptr mAuthor;
        QString mText;
        QDateTime mDateTime;

        QJsonObject toJson() const;

        using Ptr = std::unique_ptr<QuotePost>;
    };

    struct Quote
    {
        ATProto::RecordType mRecordType;

        std::variant<QuotePost::Ptr,
                     ATProto::AppBskyFeed::GeneratorView::Ptr,
                     ATProto::AppBskyGraph::ListView::Ptr> mRecord;

        QJsonObject toJson() const;

        using Ptr = std::unique_ptr<Quote>;
    };

    QString createDraftPostFileName(const QString& baseName) const;
    QString createDraftImageFileName(const QString& baseName, int seq) const;

    ATProto::AppBskyActor::ProfileViewBasic::Ptr createProfileViewBasic(const BasicProfile& author) const;
    ATProto::AppBskyActor::ProfileView::Ptr createProfileView(const Profile& author) const;

    ReplyToPost::Ptr createReplyToPost(const QString& replyToUri, const BasicProfile& author,
                                       const QString& text, const QDateTime& dateTime) const;

    Quote::Ptr createQuote(const QString& quoteUri, const BasicProfile& quoteAuthor,
                           const QString& quoteText, const QDateTime& quoteDateTime,
                           const GeneratorView& quoteFeed, const ListView& quoteList) const;

    QuotePost::Ptr createQuotePost(const BasicProfile& author, const QString& text, const QDateTime& dateTime) const;
    ATProto::AppBskyFeed::GeneratorView::Ptr createQuoteFeed(const GeneratorView& feed) const;
    ATProto::AppBskyGraph::ListView::Ptr createQuoteList(const ListView& list) const;

    QJsonDocument createJsonDoc(const ATProto::AppBskyFeed::Record::Post& post,
                                ReplyToPost::Ptr replyToPost, Quote::Ptr quote) const;

    bool save(const ATProto::AppBskyFeed::Record::Post& post, ReplyToPost::Ptr replyToPost,
              Quote::Ptr quote, const QString& draftsPath, const QString& baseName);

    ATProto::Blob::Ptr saveImage(const QString& imgName, const QString& draftsPath,
                                 const QString& baseName, int seq);

    void dropImages(const QString& draftsPath, const QString& baseName, int count);
    void dropImage(const QString& draftsPath, const QString& baseName, int seq);
};

}
