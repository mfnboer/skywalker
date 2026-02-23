// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "draft_post_data.h"
#include "generator_view.h"
#include "link_card.h"
#include "list_view.h"
#include "post.h"
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

class DraftPostsModel;

// TODO: refactor in sub subclassed for file and bluesky storage
class DraftPosts : public WrappedSkywalker, public Presence
{
    Q_MOC_INCLUDE("draft_posts_model.h")

    Q_OBJECT
    Q_PROPERTY(bool hasDrafts READ hasDrafts NOTIFY draftsChanged FINAL)
    Q_PROPERTY(StorageType storageType READ getStorageType WRITE setStorageType NOTIFY storageTypeChanged FINAL)
    QML_ELEMENT

public:
    enum StorageType {
        STORAGE_FILE = 0,
        STORAGE_BLUESKY,

        STORAGE_LAST = STORAGE_BLUESKY
    };
    Q_ENUM(StorageType)

    static constexpr int MAX_DRAFTS = 100;

    static QString getPictureDraftsPath(StorageType storageType, const QString& did);
    static void setReplyRestrictions(DraftPostData* data, const Post& post);
    static void setDraftPost(DraftPostData* data, const Post& post);

    explicit DraftPosts(QObject* parent = nullptr);
    ~DraftPosts();

    bool hasDrafts() const;

    Q_INVOKABLE bool canSaveDraft() const;

    Q_INVOKABLE DraftPostData* createDraft(
        const QString& text,
        const WebLink::List& embeddedLinks,
        const QStringList& imageFileNames, const QStringList& altTexts,
        const QStringList& memeTopTexts, const QStringList& memeBottomTexts,
        const QString& videoFileName, bool videoIsGif, const QString& videoAltText,
        int videoStartMs, int videoEndMs, int videoNewHeight,
        bool videoRemoveAudio,
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
        bool restrictReplies, bool allowMention, bool allowFollwer, bool allowFollowing,
        const QStringList& allowLists, bool embeddingDisabled,
        QDateTime timestamp = QDateTime::currentDateTime());

    Q_INVOKABLE bool saveDraftPost(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread = {});
    Q_INVOKABLE void loadDraftPosts();
    Q_INVOKABLE void loadDraftPostsNextPage();
    Q_INVOKABLE DraftPostsModel* getDraftPostsModel();
    Q_INVOKABLE QList<DraftPostData*> getDraftPostData(int index);
    Q_INVOKABLE void removeDraftPost(int index);
    Q_INVOKABLE QString getMediaStorageWarning(int index);
    Q_INVOKABLE void removeDraftPostsModel();

    StorageType getStorageType() const { return mStorageType; }
    void setStorageType(StorageType storageType);

    void getPostExternal(const Post& post, int index);
    void getPostRecord(const Post& post, int index);

    QString dumpDraftFeed();

signals:
    void saveDraftPostOk();
    void saveDraftPostFailed(QString error);
    void loadDraftPostsOk();
    void loadDraftPostsFailed(QString error);
    void deleteDraftFailed(QString error);
    void draftsChanged();
    void storageTypeChanged();

private:
    QString getDraftUri(const QString& ref) const;
    QString getRefFromDraftUri(const QString& uri) const;
    QString getDraftsPath() const;
    QString getPictureDraftsPath() const;
    QString createDraftPostFileName(const QString& baseName) const;
    QString createDraftImageFileName(const QString& baseName, int seq) const;
    QString createDraftVideoFileName(const QString& baseName) const;
    QString getBaseNameFromPostFileName(const QString& fileName) const;
    QString getBaseNameFromMediaFullFileUrl(const QString& fileUrl) const;

    static ATProto::AppBskyActor::ProfileViewBasic::SharedPtr createProfileViewBasic(const BasicProfile& author);
    static ATProto::AppBskyActor::ProfileView::SharedPtr createProfileView(const Profile& author);

    ATProto::AppBskyFeed::Record::Post::SharedPtr createPost(const DraftPostData* draftPost, const QString& picBaseName);
    Draft::EmbeddedLinks::SharedPtr createEmbeddedLinks(const WebLink::List& links);
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
    ATProto::ComATProtoLabel::Label::List createContentLabels(const ATProto::AppBskyFeed::Record::Post& post, const QString& recordUri) const;
    ATProto::ComATProtoLabel::Label::List createContentLabels(const ATProto::ComATProtoLabel::SelfLabels::SharedPtr& selfLabels, QDateTime createdAt, const QString& recordUri) const;
    ATProto::AppBskyEmbed::EmbedView::SharedPtr createEmbedView(
        const ATProto::AppBskyEmbed::Embed* embed, Draft::Quote::SharedPtr quote);
    ATProto::AppBskyEmbed::ImagesView::SharedPtr createImagesView(const ATProto::AppBskyEmbed::Images* images);
    ATProto::AppBskyEmbed::ImagesViewImage::SharedPtr createImageView(const QJsonObject& imgJson, const QString& imgSource, const QString& alt);
    ATProto::AppBskyEmbed::VideoView::SharedPtr createVideoView(const ATProto::AppBskyEmbed::Video* video);
    ATProto::AppBskyEmbed::VideoView::SharedPtr createVideoView(const QJsonObject& videoJson, const QString& videoSource, const std::optional<QString>& alt, const std::optional<ATProto::AppBskyEmbed::VideoPresentation>& presentation);
    ATProto::AppBskyEmbed::ExternalView::SharedPtr createExternalView(const ATProto::AppBskyEmbed::External* external) const;
    ATProto::AppBskyEmbed::RecordView::SharedPtr createRecordView(const ATProto::AppBskyEmbed::Record* record, Draft::Quote::SharedPtr quote) const;
    ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr createRecordWithMediaView(
        const ATProto::AppBskyEmbed::RecordWithMedia* record, Draft::Quote::SharedPtr quote);

    QString checkMediaStorage(const ATProto::AppBskyDraft::DraftView& draft, const ATProto::AppBskyFeed::PostFeed& postFeed, int embedImagesCount, int embedVideoCount) const;
    ATProto::AppBskyFeed::PostFeed convertDraftToFeedViewPost(const ATProto::AppBskyDraft::DraftView& draft, const QString& recordUri);
    ATProto::AppBskyFeed::ReplyRef::SharedPtr createReplyRef(const Draft::ReplyToPost::SharedPtr& replyToPost, const ATProto::AppBskyFeed::PostReplyRef::SharedPtr& postReplyRef) const;
    ATProto::AppBskyFeed::PostView::SharedPtr convertReplyToPostView(const Draft::ReplyToPost::SharedPtr& replyToPost, const ATProto::AppBskyFeed::PostReplyRef::SharedPtr& postReplyRef) const;
    ATProto::AppBskyFeed::Record::Post::SharedPtr createReplyToPost(const Draft::ReplyToPost::SharedPtr& replyToPost) const;
    ATProto::AppBskyFeed::PostView::SharedPtr convertDraftToPostView(const ATProto::AppBskyDraft::DraftView& draftView, const ATProto::AppBskyDraft::DraftPost& draftPost, const QString& recordUri);
    ATProto::AppBskyFeed::ViewerState::SharedPtr createViewerState(const ATProto::AppBskyDraft::Draft& draft) const;
    ATProto::AppBskyFeed::ThreadgateView::SharedPtr createThreadgateView(const ATProto::AppBskyFeed::ThreadgateRules& threadgateRules, const QString& recordUri, QDateTime createdAt) const;
    bool hasEmbed(const ATProto::AppBskyDraft::DraftPost& draftPost) const;
    bool hasMediaEmbed(const ATProto::AppBskyDraft::DraftPost& draftPost) const;
    ATProto::AppBskyEmbed::EmbedView::SharedPtr createEmbedView(const ATProto::AppBskyDraft::DraftPost& draftPost);
    ATProto::AppBskyEmbed::ImagesView::SharedPtr createImagesView(const ATProto::AppBskyDraft::DraftEmbedImage::List& images);
    ATProto::AppBskyEmbed::VideoView::SharedPtr createVideoView(const ATProto::AppBskyDraft::DraftEmbedVideo& video);
    ATProto::AppBskyEmbed::ExternalView::SharedPtr createExternalView(const ATProto::AppBskyDraft::DraftEmbedExternal& external);
    ATProto::AppBskyEmbed::RecordView::SharedPtr createRecordView(const ATProto::AppBskyDraft::DraftEmbedRecord& record);
    ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr createRecordWithMediaView(const ATProto::AppBskyEmbed::ImagesView::SharedPtr imagesView, const ATProto::AppBskyDraft::DraftEmbedRecord& record);
    ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr createRecordWithMediaView(const ATProto::AppBskyEmbed::VideoView::SharedPtr videoView, const ATProto::AppBskyDraft::DraftEmbedRecord& record);
    ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr createRecordWithMediaView(const ATProto::AppBskyEmbed::ExternalView::SharedPtr externalView, const ATProto::AppBskyDraft::DraftEmbedRecord& record);

    QUrl getGifUrl(const TenorGif& gif) const;
    void addGifToPost(ATProto::AppBskyFeed::Record::Post& post, const TenorGif& gif) const;
    void addExternalLinkToPost(ATProto::AppBskyFeed::Record::Post& post, const QString& externalLink) const;

    void dropDraftMedia(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread, const QString& baseName);

    // FILE STORAGE
    void loadDraftFeed();
    QStringList getDraftPostFiles(const QString& draftsPath) const;
    Draft::Draft::SharedPtr loadDraft(const QString& fileName, const QString& draftsPath) const;
    bool saveFileDraftPost(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread);
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
                                       bool videoRemoveAudio, int videoNewHeight, const QString& draftsPath, const QString& baseName);
    void dropImages(const QString& draftsPath, const QString& baseName, int count) const;
    void dropImage(const QString& draftsPath, const QString& baseName, int seq) const;
    void dropVideo(const QString& draftsPath, const QString& baseName);
    void dropDraftPostFiles(const QString& draftsPath, const QString& fileName);
    void dropDraftPostFilesByBaseName(const QString& draftsPath, const QString& baseName);
    void dropDraftPost(const QString& fileName);

    // BLUESKY STORAGE
    void loadBlueskyDrafts(const QString& cursor = {});
    void loadBlueskyDraftsNextPage();
    bool saveBlueskyDraftPost(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread);
    ATProto::AppBskyDraft::Draft::SharedPtr createBlueskyDraft(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread, const QString& baseName);
    ATProto::AppBskyDraft::DraftPost::SharedPtr createBlueskyDraftPost(const DraftPostData* draftPost, const QString& baseName, int threadIndex);
    ATProto::AppBskyDraft::DraftEmbedImage::List createDraftEmbedImages(const DraftPostData* draftPost, const QString& baseName);
    ATProto::AppBskyDraft::DraftEmbedVideo::List createDraftEmbedVideos(const DraftPostData* draftPost, const QString& baseName);
    ATProto::AppBskyDraft::DraftEmbedExternal::List createDraftEmbedExternals(const DraftPostData* draftPost);
    ATProto::AppBskyDraft::DraftEmbedRecord::List createDraftEmbedRecords(const DraftPostData* draftPost);
    ATProto::ComATProtoLabel::SelfLabels::SharedPtr createSelfLabels(const DraftPostData* draftPost) const;
    ATProto::AppBskyFeed::ThreadgateRules createThreadgateRules(const DraftPostData* draftPost) const;
    void deleteBlueskyDraft(const QString& draftId, int index);
    void deleteMediaFiles(int index);
    QString getMediaBaseName(const std::vector<Post>& thread) const;

    void updatePostRecord(const Post& post, int index, const ATProto::AppBskyEmbed::Record* record, Draft::Quote::SharedPtr quote) const;
    void failUpdatePostRecord(const Post& post, int index, const QString& error);

    void getPostRecordPost(const Post& post, int index, const QString& postUri);
    void getPostRecordFeed(const Post& post, int index, const QString& feedUri);
    void getPostRecordList(const Post& post, int index, const QString& listUri);
    void getPostRecordStarterPack(const Post& post, int index, const QString& starterPackUri);

    std::unique_ptr<DraftPostsModel> mDraftPostsModel;
    StorageType mStorageType = STORAGE_FILE;
};

}
