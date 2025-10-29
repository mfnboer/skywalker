// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include "image_reader.h"
#include "link_card.h"
#include "list_view.h"
#include "m3u8_reader.h"
#include "post.h"
#include "post_interaction_settings.h"
#include "postgate.h"
#include "presence.h"
#include "profile.h"
#include "video_upload_limits.h"
#include "web_link.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/post_master.h>
#include <QImage>
#include <unordered_map>

namespace Skywalker {

class AbstractPostFeedModel;
class DraftPostData;

class PostUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit PostUtils(QObject* parent = nullptr);

    Q_INVOKABLE static bool isPostUri(const QString& uri);
    Q_INVOKABLE static QString extractDidFromUri(const QString& uri);
    Q_INVOKABLE void checkPostExists(const QString& uri, const QString& cid);
    Q_INVOKABLE void canQuotePost(const QString& uri);
    Q_INVOKABLE void post(const QString& text, const QStringList& imageFileNames, const QStringList& altTexts,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid,
                          const QString& quoteUri, const QString& quoteCid,
                          const WebLink::List& embeddedLinks,
                          const QStringList& labels, const QString& language);
    Q_INVOKABLE void post(const QString& text, const LinkCard* card,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid,
                          const QString& quoteUri, const QString& quoteCid,
                          const WebLink::List& embeddedLinks,
                          const QStringList& labels, const QString& language);
    Q_INVOKABLE void postVideo(const QString& text, const QString& videoFileName,
                          const QString& videoAltText, int videoWidth, int videoHeight,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid,
                          const QString& quoteUri, const QString& quoteCid,
                          const WebLink::List& embeddedLinks,
                          const QStringList& labels, const QString& language);
    Q_INVOKABLE void addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollower, bool allowFollowing, const QStringList& allowList, bool allowNobody, const QStringList& hiddenReplies);
    Q_INVOKABLE void addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollower, bool allowFollowing, const ListViewBasicList& allowList, bool allowNobody, const QStringList& hiddenReplies);
    Q_INVOKABLE void addPostgate(const QString& uri, bool disableEmbedding, const QStringList& detachedEmbeddingUris);
    Q_INVOKABLE void undoThreadgate(const QString& threadgateUri, const QString& cid);
    Q_INVOKABLE void undoPostgate(const QString& postUri);
    Q_INVOKABLE void detachQuote(const QString& uri, const QString& embeddingUri, const QString& embeddingCid, bool detach);
    Q_INVOKABLE void repost(const QString& uri, const QString& cid, const QString& viaUri = {}, const QString& viaCid = {});
    Q_INVOKABLE void undoRepost(const QString& repostUri, const QString& origPostCid);
    Q_INVOKABLE void like(const QString& uri, const QString& cid, const QString& viaUri = {}, const QString& viaCid = {});
    Q_INVOKABLE void undoLike(const QString& likeUri, const QString& cid);
    Q_INVOKABLE void muteThread(const QString& uri);
    Q_INVOKABLE void unmuteThread(const QString& uri);
    Q_INVOKABLE void deletePost(const QString& postUri, const QString& cid);
    Q_INVOKABLE void batchDeletePosts(const QStringList& postUris);
    Q_INVOKABLE bool pickPhoto(bool pickVideo = false);
    Q_INVOKABLE void savePhoto(const QString& sourceUrl);
    Q_INVOKABLE void sharePhotoToApp(const QString& sourceUrl);
    Q_INVOKABLE static void dropPhoto(const QString& source);
    Q_INVOKABLE static void dropVideo(const QString& source);
    Q_INVOKABLE QString cutPhotoRect(const QString& source, const QRect& rect, const QSize& scaledSize);

    Q_INVOKABLE void cacheTags(const QString& text);
    Q_INVOKABLE static QString linkiFy(const QString& text, const QString& colorName);

    Q_INVOKABLE void getQuotePost(const QString& httpsUri);
    Q_INVOKABLE void getQuoteFeed(const QString& httpsUri);
    Q_INVOKABLE void getQuoteList(const QString& httpsUri);
    Q_INVOKABLE void getPostgate(const QString& postUri);
    Q_INVOKABLE void getVideoUploadLimits();
    Q_INVOKABLE void checkVideoUploadLimits();
    Q_INVOKABLE PostInteractionSettings getPostInteractionSettings() const;
    Q_INVOKABLE void savePostInteractionSettings(bool allowMention, bool allowFollower, bool allowFollowing,
            const QStringList& allowList, bool allowNobody, bool disableEmbedding);

    Q_INVOKABLE void identifyLanguage(QString text, int index);

    Q_INVOKABLE void getEditPostData(AbstractPostFeedModel* model, int postIndex);
    Q_INVOKABLE void dropEditPostData(QList<DraftPostData*> draftPostData);

signals:
    void checkPostExistsOk(QString uri, QString cid);
    void checkPostExistsFailed(QString uri, QString cid, QString error);
    void canQuotePostOk(QString uri, bool canQuote);
    void canQuotePostFailed(QString uri, QString error);
    void postOk(QString uri, QString cid);
    void postFailed(QString error);
    void threadgateOk();
    void threadgateFailed(QString error);
    void undoThreadgateOk();
    void undoThreadgateFailed(QString error);
    void postgateOk();
    void postgateFailed(QString error);
    void undoPostgateOk();
    void undoPostgateFailed(QString error);
    void detachQuoteOk(bool detached);
    void detachQuoteFailed(QString error);
    void postProgress(QString msg);
    void repostOk();
    void repostFailed(QString error);
    void repostProgress(QString msg);
    void undoRepostOk();
    void undoRepostFailed(QString error);
    void likeOk();
    void likeFailed(QString error);
    void undoLikeOk();
    void undoLikeFailed(QString error);
    void muteThreadOk();
    void muteThreadFailed(QString error);
    void unmuteThreadOk();
    void unmuteThreadFailed(QString error);
    void postDeletedOk();
    void postDeletedFailed(QString error);
    void photoPicked(QString imgSource, QString gifTempFileName);
    void photoPickFailed(QString error);
    void photoPickCanceled();
    void videoPicked(QUrl url);
    void videoPickedFailed(QString error);
    void quotePost(QString uri, QString cid, QString text, BasicProfile author, QDateTime);
    void quoteFeed(GeneratorView feed);
    void quoteList(ListView list);
    void getPostgateOk(Postgate postgate);
    void getPostgateFailed(QString error);
    void videoUploadLimits(VideoUploadLimits limits);
    void checkVideoLimitsOk(VideoUploadLimits limits);
    void checkVideoLimitsFailed(QString error);
    void languageIdentified(QString languageCode, int index);
    void editPostData(QList<DraftPostData*> draft);
    void editPostDataProgress(QString msg);

private:
    void continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid, const QStringList& labels);
    void continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex = 0);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid, const QStringList& labels);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(const LinkCard* card, QImage thumb, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePostVideo(const QString& videoFileName, const QString& videoAltText, int videoWidth, int videoHeight, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid, const QStringList& labels);
    void continuePostVideo(const QString& videoFileName, const QString& videoAltText, int videoWidth, int videoHeight, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continueRepost(const QString& uri, const QString& cid, const QString& viaUri = {}, const QString& viaCid = {});
    void continueReAttachQuote(const QString& embeddingUri, int retries =1);
    void shareMedia(int fd, const QString& mimeType);
    void sharePhoto(int fd);
    void shareVideo(int fd, const QString& mimeType);
    void cancelPhotoPicking();
    void getVideoUploadLimits(const std::function<void(const VideoUploadLimits&)>& cb);
    void continueSharePhotoToApp(const QString& fileName);
    void handleLanguageIdentified(const QString& languageCode, int requestId);
    void addIndexLanguageIdentificationRequestId(int index, int requestId);
    void removeIndexLanguageIdentificationRequestId(int index, int requestId);

    void loadEditPostImages(DraftPostData* data, const Post& post, int imageIndex = 0);
    void loadEditPostVideo(DraftPostData* data, const Post& post);
    void loadEditPostVideo(DraftPostData* data, const Post& post, const QString& videoStream);
    void finishedLoadingEditPost(DraftPostData* data);

    QNetworkAccessManager* mNetwork;
    ATProto::PostMaster* postMaster();
    ImageReader* imageReader();
    M3U8Reader* m3u8Reader();
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    std::unique_ptr<ImageReader> mImageReader;
    std::unique_ptr<M3U8Reader> mM3U8Reader;
    bool mPickingPhoto = false;
    std::unordered_map<int, int> mIndexLanguageIdentificationRequestIdMap;
    std::unordered_map<int, int> mLanguageIdentificationRequestIdIndexMap;

    static int sNextRequestId;
};

}
