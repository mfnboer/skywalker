// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "draft_post_data.h"
#include "draft_posts_model.h"
#include "generator_view.h"
#include "link_card.h"
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

    Q_INVOKABLE DraftPostData* createDraft(const QString& text,
                                           const QStringList& imageFileNames, const QStringList& altTexts,
                                           const QStringList& memeTopTexts, const QStringList& memeBottomTexts,
                                           const QString& videoFileName, const QString& videoAltText,
                                           int videoStartMs, int videoEndMs, int videoNewHeight,
                                           const QString& replyToUri, const QString& replyToCid,
                                           const QString& replyRootUri, const QString& replyRootCid,
                                           const BasicProfile& replyToAuthor, const QString& replyToText,
                                           const QDateTime& replyToDateTime,
                                           const QString& quoteUri, const QString& quoteCid,
                                           const BasicProfile& quoteAuthor, const QString& quoteText,
                                           const QDateTime& quoteDateTime, bool quoteFixed,
                                           const GeneratorView& quoteFeed, const ListView& quoteList,
                                           const TenorGif gif, const LinkCard* card, const QStringList& labels,
                                           const QString& language,
                                           bool restrictReplies, bool allowMention, bool allowFollowing,
                                           const QStringList& allowLists, bool embeddingDisabled,
                                           QDateTime timestamp = QDateTime::currentDateTime());

    Q_INVOKABLE bool saveDraftPost(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread = {});
    Q_INVOKABLE void loadDraftPosts();
    Q_INVOKABLE DraftPostsModel* getDraftPostsModel();
    Q_INVOKABLE QList<DraftPostData*> getDraftPostData(int index);
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
    using UploadImageSuccessCb = std::function<void(ATProto::Blob::SharedPtr, QSize)>;
    using SuccessCb = std::function<void()>;
    using DoneCb = std::function<void()>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    QString getDraftUri(const QString& ref) const;
    QString getDraftsPath() const;
    QString getPictureDraftsPath() const;
    QString createDraftPostFileName(const QString& baseName) const;
    QString createDraftImageFileName(const QString& baseName, int seq) const;
    QString createDraftVideoFileName(const QString& baseName) const;
    QString getBaseNameFromPostFileName(const QString& fileName) const;

    static ATProto::AppBskyActor::ProfileViewBasic::SharedPtr createProfileViewBasic(const BasicProfile& author);
    static ATProto::AppBskyActor::ProfileView::SharedPtr createProfileView(const Profile& author);

    ATProto::AppBskyFeed::Record::Post::SharedPtr createPost(const DraftPostData* draftPost, const QString& picBaseName);
    Draft::ReplyToPost::SharedPtr createReplyToPost(const QString& replyToUri, const BasicProfile& author,
                                       const QString& text, const QDateTime& dateTime) const;

    Draft::Quote::SharedPtr createQuote(const QString& quoteUri, const BasicProfile& quoteAuthor,
                           const QString& quoteText, const QDateTime& quoteDateTime,
                           const GeneratorView& quoteFeed, const ListView& quoteList) const;

    Draft::QuotePost::SharedPtr createQuotePost(const BasicProfile& author, const QString& text, const QDateTime& dateTime) const;
    ATProto::AppBskyFeed::GeneratorView::SharedPtr createQuoteFeed(const GeneratorView& feed) const;
    ATProto::AppBskyGraph::ListView::SharedPtr createQuoteList(const ListView& list) const;

    ATProto::AppBskyFeed::PostFeed convertDraftToFeedViewPost(Draft::Draft& draft, const QString& recordUri);
    ATProto::AppBskyFeed::PostView::SharedPtr convertDraftToPostView(Draft::Draft& draft, const QString& recordUri);
    ATProto::AppBskyFeed::ViewerState::SharedPtr createViewerState(Draft::Draft& draft) const;
    ATProto::AppBskyFeed::ThreadgateView::SharedPtr createThreadgateView(Draft::Draft& draft) const;
    ATProto::AppBskyFeed::Record::Post::SharedPtr createReplyToPost(const Draft::Draft& draft) const;
    ATProto::AppBskyFeed::PostView::SharedPtr convertReplyToPostView(Draft::Draft& draft) const;
    ATProto::AppBskyFeed::ReplyRef::SharedPtr createReplyRef(Draft::Draft& draft) const;
    ATProto::ComATProtoLabel::LabelList createContentLabels(const ATProto::AppBskyFeed::Record::Post& post, const QString& recordUri) const;
    ATProto::AppBskyEmbed::EmbedView::SharedPtr createEmbedView(
        const ATProto::AppBskyEmbed::Embed* embed, Draft::Quote::SharedPtr quote);
    ATProto::AppBskyEmbed::ImagesView::SharedPtr createImagesView(const ATProto::AppBskyEmbed::Images* images);
    ATProto::AppBskyEmbed::VideoView::SharedPtr createVideoView(const ATProto::AppBskyEmbed::Video* video);
    ATProto::AppBskyEmbed::ExternalView::SharedPtr createExternalView(const ATProto::AppBskyEmbed::External* external) const;
    ATProto::AppBskyEmbed::RecordView::SharedPtr createRecordView(const ATProto::AppBskyEmbed::Record* record, Draft::Quote::SharedPtr quote) const;
    ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr createRecordWithMediaView(
        const ATProto::AppBskyEmbed::RecordWithMedia* record, Draft::Quote::SharedPtr quote);

    void addGifToPost(ATProto::AppBskyFeed::Record::Post& post, const TenorGif& gif) const;
    void addExternalLinkToPost(ATProto::AppBskyFeed::Record::Post& post, const QString& externalLink) const;

    // FILE STORAGE
    void loadDraftFeed();
    QStringList getDraftPostFiles(const QString& draftsPath) const;
    Draft::Draft::SharedPtr loadDraft(const QString& fileName, const QString& draftsPath) const;
    bool save(const Draft::Draft& draft, const QString& draftsPath, const QString& baseName);
    bool addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                         const QList<ImageView>& images,
                         const QString& draftsPath, const QString& baseName);
    bool addVideoToPost(ATProto::AppBskyFeed::Record::Post& post,
                        const VideoView& video,
                        const QString& draftsPath, const QString& baseName);
    std::tuple<ATProto::Blob::SharedPtr, QSize> saveImage(const QString& imgName,
                                       const QString& memeTopText, const QString& memeBottomText,
                                       const QString& draftsPath, const QString& baseName, int seq);
    ATProto::Blob::SharedPtr saveVideo(const QString& videoName, int videoStartMs, int videoEndMs,
                                       int videoNewHeight, const QString& draftsPath, const QString& baseName);
    void dropImages(const QString& draftsPath, const QString& baseName, int count) const;
    void dropImage(const QString& draftsPath, const QString& baseName, int seq) const;
    void dropVideo(const QString& draftsPath, const QString& baseName);
    void dropDraftPostFiles(const QString& draftsPath, const QString& fileName);
    void dropDraftPost(const QString& fileName);

    // PDS REPO STORAGE
    bool writeRecord(const Draft::Draft& draft);
    void listRecords();
    void deleteRecord(const QString& recordUri);
    bool uploadImage(const QString& imageName, const UploadImageSuccessCb& successCb, const ErrorCb& errorCb);
    void addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                         const QList<ImageView>& images,
                         const std::function<void()>& continueCb, int imgSeq = 1);

    DraftPostsModel::Ptr mDraftPostsModel;

    StorageType mStorageType = STORAGE_REPO;
};

}
