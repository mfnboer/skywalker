// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "draft_post_data.h"
#include "draft_posts_model.h"
#include "generator_view.h"
#include "list_view.h"
#include "presence.h"
#include "profile.h"
#include "tenor_gif.h"
#include "wrapped_skywalker.h"
#include "lexicon/draft.h"
#include <atproto/lib/post_master.h>
#include <QString>
#include <QObject>
#include <QtQmlIntegration>
#include <unordered_map>

namespace Skywalker {

class DraftPosts : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool hasDrafts READ hasDrafts NOTIFY draftsChanged FINAL)
    QML_ELEMENT

public:
    static constexpr int MAX_DRAFTS = 50;

    explicit DraftPosts(QObject* parent = nullptr);
    ~DraftPosts();

    bool hasDrafts() const;

    Q_INVOKABLE bool canSaveDraft() const;

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

    Q_INVOKABLE void loadDraftPosts();
    Q_INVOKABLE DraftPostsModel* getDraftPostsModel();
    Q_INVOKABLE DraftPostData* getDraftPostData(int index);
    Q_INVOKABLE void removeDraftPost(int index);
    Q_INVOKABLE void removeDraftPostsModel();

signals:
    void saveDraftPostOk();
    void saveDraftPostFailed(QString error);
    void uploadingImage(int seq);
    void loadDraftPostsOk();
    void loadDraftPostsFailed(QString error);
    void draftsChanged();

private:
    using CidImgSourceMap = std::unordered_map<QString, QString>;
    using UploadImageSuccessCb = std::function<void(ATProto::Blob::Ptr)>;
    using SuccessCb = std::function<void()>;
    using DoneCb = std::function<void()>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    QString getDraftUri(const QString& ref) const;

    static ATProto::AppBskyActor::ProfileViewBasic::Ptr createProfileViewBasic(const BasicProfile& author);
    static ATProto::AppBskyActor::ProfileView::Ptr createProfileView(const Profile& author);

    Draft::ReplyToPost::Ptr createReplyToPost(const QString& replyToUri, const BasicProfile& author,
                                       const QString& text, const QDateTime& dateTime) const;

    Draft::Quote::Ptr createQuote(const QString& quoteUri, const BasicProfile& quoteAuthor,
                           const QString& quoteText, const QDateTime& quoteDateTime,
                           const GeneratorView& quoteFeed, const ListView& quoteList) const;

    Draft::QuotePost::Ptr createQuotePost(const BasicProfile& author, const QString& text, const QDateTime& dateTime) const;
    ATProto::AppBskyFeed::GeneratorView::Ptr createQuoteFeed(const GeneratorView& feed) const;
    ATProto::AppBskyGraph::ListView::Ptr createQuoteList(const ListView& list) const;

    ATProto::AppBskyFeed::FeedViewPost::Ptr convertDraftToFeedViewPost(Draft::Draft& draft, const QString& recordUri);
    ATProto::AppBskyFeed::PostView::Ptr convertDraftToPostView(Draft::Draft& draft, const QString& recordUri);
    ATProto::AppBskyFeed::ThreadgateView::Ptr createThreadgateView(Draft::Draft& draft) const;
    ATProto::AppBskyFeed::Record::Post::Ptr createReplyToPost(const Draft::Draft& draft) const;
    ATProto::AppBskyFeed::PostView::Ptr convertReplyToPostView(Draft::Draft& draft) const;
    ATProto::AppBskyFeed::ReplyRef::Ptr createReplyRef(Draft::Draft& draft) const;
    ATProto::ComATProtoLabel::LabelList createContentLabels(const ATProto::AppBskyFeed::Record::Post& post, const QString& recordUri) const;
    ATProto::AppBskyEmbed::EmbedView::Ptr createEmbedView(
        const ATProto::AppBskyEmbed::Embed* embed, Draft::Quote::Ptr quote);
    ATProto::AppBskyEmbed::ImagesView::Ptr createImagesView(const ATProto::AppBskyEmbed::Images* images);
    ATProto::AppBskyEmbed::ExternalView::Ptr createExternalView(const ATProto::AppBskyEmbed::External* external) const;
    ATProto::AppBskyEmbed::RecordView::Ptr createRecordView(const ATProto::AppBskyEmbed::Record* record, Draft::Quote::Ptr quote) const;
    ATProto::AppBskyEmbed::RecordWithMediaView::Ptr createRecordWithMediaView(
        const ATProto::AppBskyEmbed::RecordWithMedia* record, Draft::Quote::Ptr quote);

    bool writeRecord(const Draft::Draft& draft);
    void listRecords();
    void deleteRecord(const QString& recordUri);
    bool uploadImage(const QString& imageName, const UploadImageSuccessCb& successCb, const ErrorCb& errorCb);

    void addGifToPost(ATProto::AppBskyFeed::Record::Post& post, const TenorGif& gif) const;
    void addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                         const QStringList& imageFileNames, const QStringList& altTexts,
                         const std::function<void()>& continueCb, int imgSeq = 1);

    QString getImgSource(const QString& cid) const;

    DraftPostsModel::Ptr mDraftPostsModel;
    CidImgSourceMap mCidImgSourceMap;
};

}
