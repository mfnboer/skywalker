// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "draft_post_data.h"
#include "draft_posts_model.h"
#include "generator_view.h"
#include "list_view.h"
#include "profile.h"
#include "tenor_gif.h"
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

    Q_INVOKABLE bool hasDrafts() const;

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
                                   const TenorGif gif, const QStringList& labels,
                                   bool restrictReplies, bool allowMention, bool allowFollowing,
                                   const QStringList& allowLists);

    Q_INVOKABLE void loadDraftPostsModel(DraftPostsModel* model);
    Q_INVOKABLE DraftPostData* getDraftPostData(const DraftPostsModel* model, int index);
    Q_INVOKABLE void removeDraftPost(const QString& fileName);

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

        QJsonObject toJson() const;

        using Ptr = std::unique_ptr<Draft>;
        static Ptr fromJson(const QJsonObject& json);
    };

    QString getDraftUri(const QString& fileName) const;
    QString getDraftsPath() const;
    QString createDraftPostFileName(const QString& baseName) const;
    QString createDraftImageFileName(const QString& baseName, int seq) const;
    QString getBaseNameFromPostFileName(const QString& fileName) const;

    static ATProto::AppBskyActor::ProfileViewBasic::Ptr createProfileViewBasic(const BasicProfile& author);
    static ATProto::AppBskyActor::ProfileView::Ptr createProfileView(const Profile& author);

    ReplyToPost::Ptr createReplyToPost(const QString& replyToUri, const BasicProfile& author,
                                       const QString& text, const QDateTime& dateTime) const;

    Quote::Ptr createQuote(const QString& quoteUri, const BasicProfile& quoteAuthor,
                           const QString& quoteText, const QDateTime& quoteDateTime,
                           const GeneratorView& quoteFeed, const ListView& quoteList) const;

    QuotePost::Ptr createQuotePost(const BasicProfile& author, const QString& text, const QDateTime& dateTime) const;
    ATProto::AppBskyFeed::GeneratorView::Ptr createQuoteFeed(const GeneratorView& feed) const;
    ATProto::AppBskyGraph::ListView::Ptr createQuoteList(const ListView& list) const;

    ATProto::AppBskyFeed::FeedViewPost::Ptr convertDraftToFeedViewPost(Draft& draft, const QString& fileName, const QString& draftsPath) const;
    ATProto::AppBskyFeed::PostView::Ptr convertDraftToPostView(Draft& draft, const QString& fileName, const QString& draftsPath) const;
    ATProto::AppBskyFeed::ThreadgateView::Ptr createThreadgateView(Draft& draft) const;
    ATProto::AppBskyFeed::Record::Post::Ptr createReplyToPost(const Draft& draft) const;
    ATProto::AppBskyFeed::PostView::Ptr convertReplyToPostView(Draft& draft) const;
    ATProto::AppBskyFeed::ReplyRef::Ptr createReplyRef(Draft& draft) const;
    ATProto::ComATProtoLabel::LabelList createContentLabels(const ATProto::AppBskyFeed::Record::Post& post, const QString& fileName) const;
    ATProto::AppBskyEmbed::EmbedView::Ptr createEmbedView(
        const ATProto::AppBskyEmbed::Embed* embed, Quote::Ptr quote, const QString& draftsPath) const;
    ATProto::AppBskyEmbed::ImagesView::Ptr createImagesView(
        const ATProto::AppBskyEmbed::Images* images, const QString& draftsPath) const;
    ATProto::AppBskyEmbed::ExternalView::Ptr createExternalView(
        const ATProto::AppBskyEmbed::External* external) const;
    ATProto::AppBskyEmbed::RecordView::Ptr createRecordView(
        const ATProto::AppBskyEmbed::Record* record, Quote::Ptr quote) const;
    ATProto::AppBskyEmbed::RecordWithMediaView::Ptr createRecordWithMediaView(
        const ATProto::AppBskyEmbed::RecordWithMedia* record, Quote::Ptr quote, const QString& draftsPath) const;

    bool save(Draft::Ptr draft, const QString& draftsPath, const QString& baseName);

    ATProto::Blob::Ptr saveImage(const QString& imgName, const QString& draftsPath,
                                 const QString& baseName, int seq);

    void addGifToPost(ATProto::AppBskyFeed::Record::Post& post, const TenorGif& gif) const;
    bool addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                         const QStringList& imageFileNames, const QStringList& altTexts,
                         const QString& draftsPath, const QString& baseName);
    void dropImages(const QString& draftsPath, const QString& baseName, int count) const;
    void dropImage(const QString& draftsPath, const QString& baseName, int seq) const;
    void dropDraftPost(const QString& draftsPath, const QString& fileName);

    QStringList getDraftPostFiles(const QString& draftsPath) const;
    Draft::Ptr loadDraft(const QString& fileName, const QString& draftsPath) const;
    ATProto::AppBskyFeed::PostFeed loadDraftFeed();
};

}
