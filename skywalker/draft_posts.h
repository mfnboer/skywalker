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

namespace Skywalker {

class DraftPosts : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool hasDrafts READ hasDrafts NOTIFY draftsChanged FINAL)
    Q_PROPERTY(StorageType storageType READ getStorageType WRITE setStorageType NOTIFY storageTypeChanged FINAL)
    QML_ELEMENT

public:
    enum StorageType { STORAGE_FILE, STORAGE_REPO };
    Q_ENUM(StorageType)

    static constexpr int MAX_DRAFTS = 50;

    explicit DraftPosts(QObject* parent = nullptr);
    ~DraftPosts();

    bool hasDrafts() const;

    Q_INVOKABLE bool canSaveDraft() const;

    Q_INVOKABLE bool saveDraftPost(const QString& text,
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
                                   const QStringList& allowLists,
                                   QDateTime timestamp = QDateTime::currentDateTime());

    Q_INVOKABLE void loadDraftPosts();
    Q_INVOKABLE DraftPostsModel* getDraftPostsModel();
    Q_INVOKABLE DraftPostData* getDraftPostData(int index);
    Q_INVOKABLE void removeDraftPost(int index);
    Q_INVOKABLE void removeDraftPostsModel();

    StorageType getStorageType() const { return mStorageType; }
    void setStorageType(StorageType storageType);

signals:
    void saveDraftPostOk();
    void saveDraftPostFailed(QString error);
    void uploadingImage(int seq);
    void loadDraftPostsOk();
    void loadDraftPostsFailed(QString error);
    void draftsChanged();
    void storageTypeChanged();

private:
    using UploadImageSuccessCb = std::function<void(ATProto::Blob::Ptr)>;
    using SuccessCb = std::function<void()>;
    using DoneCb = std::function<void()>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    QString getDraftUri(const QString& ref) const;
    QString getDraftsPath() const;
    QString getPictureDraftsPath() const;
    QString createDraftPostFileName(const QString& baseName) const;
    QString createDraftImageFileName(const QString& baseName, int seq) const;
    QString getBaseNameFromPostFileName(const QString& fileName) const;

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

    void addGifToPost(ATProto::AppBskyFeed::Record::Post& post, const TenorGif& gif) const;

    // FILE STORAGE
    void loadDraftFeed();
    QStringList getDraftPostFiles(const QString& draftsPath) const;
    Draft::Draft::Ptr loadDraft(const QString& fileName, const QString& draftsPath) const;
    bool save(const Draft::Draft& draft, const QString& draftsPath, const QString& baseName);
    bool addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                         const QStringList& imageFileNames, const QStringList& altTexts,
                         const QString& draftsPath, const QString& baseName);
    ATProto::Blob::Ptr saveImage(const QString& imgName, const QString& draftsPath,
                                 const QString& baseName, int seq);
    void dropImages(const QString& draftsPath, const QString& baseName, int count) const;
    void dropImage(const QString& draftsPath, const QString& baseName, int seq) const;
    void dropDraftPostFiles(const QString& draftsPath, const QString& fileName);
    void dropDraftPost(const QString& fileName);

    // PDS REPO STORAGE
    bool writeRecord(const Draft::Draft& draft);
    void listRecords();
    void deleteRecord(const QString& recordUri);
    bool uploadImage(const QString& imageName, const UploadImageSuccessCb& successCb, const ErrorCb& errorCb);
    void addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                         const QStringList& imageFileNames, const QStringList& altTexts,
                         const std::function<void()>& continueCb, int imgSeq = 1);

    DraftPostsModel::Ptr mDraftPostsModel;

    StorageType mStorageType = STORAGE_REPO;
};

}
