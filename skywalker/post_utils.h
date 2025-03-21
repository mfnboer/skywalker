// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "facet_highlighter.h"
#include "generator_view.h"
#include "image_reader.h"
#include "link_card.h"
#include "list_view.h"
#include "post_interaction_settings.h"
#include "postgate.h"
#include "presence.h"
#include "profile.h"
#include "text_differ.h"
#include "video_upload_limits.h"
#include "web_link.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/post_master.h>
#include <QImage>
#include <QQuickTextDocument>
#include <unordered_map>

namespace Skywalker {

class PostUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(QString textWithoutLinks READ getTextWithoutLinks WRITE setTextWithoutLinks NOTIFY textWithoutLinksChanged FINAL)
    Q_PROPERTY(QString editMention READ getEditMention WRITE setEditMention NOTIFY editMentionChanged FINAL)
    Q_PROPERTY(QString editTag READ getEditTag WRITE setEditTag NOTIFY editTagChanged FINAL)
    Q_PROPERTY(QString firstWebLink READ getFirstWebLink WRITE setFirstWebLink NOTIFY firstWebLinkChanged FINAL)
    Q_PROPERTY(bool cursorInFirstWebLink READ isCursorInFirstWebLink WRITE setCursorInFirstWebLink NOTIFY cursorInFirstWebLinkChanged FINAL)
    Q_PROPERTY(QString firstPostLink READ getFirstPostLink WRITE setFirstPostLink NOTIFY firstPostLinkChanged FINAL)
    Q_PROPERTY(bool cursorInFirstPostLink READ isCursorInFirstPostLink WRITE setCursorInFirstPostLink NOTIFY cursorInFirstPostLinkChanged FINAL)
    Q_PROPERTY(QString firstFeedLink READ getFirstFeedLink WRITE setFirstFeedLink NOTIFY firstFeedLinkChanged FINAL)
    Q_PROPERTY(bool cursorInFirstFeedLink READ isCursorInFirstFeedLink WRITE setCursorInFirstFeedLink NOTIFY cursorInFirstFeedLinkChanged FINAL)
    Q_PROPERTY(QString firstListLink READ getFirstListLink WRITE setFirstListLink NOTIFY firstListLinkChanged FINAL)
    Q_PROPERTY(bool cursorInFirstListLink READ isCursorInFirstListLink WRITE setCursorInFirstListLink NOTIFY cursorInFirstListLinkChanged FINAL)
    Q_PROPERTY(WebLink::List webLinks READ getWebLinks WRITE setWebLinks NOTIFY webLinksChanged FINAL)
    Q_PROPERTY(int cursorInWebLink READ getCursorInWebLink WRITE setCursorInWebLink NOTIFY cursorInWebLinkChanged FINAL)
    Q_PROPERTY(WebLink::List embeddedLinks READ getEmbeddedLinks WRITE setEmbeddedLinks NOTIFY embeddedLinksChanged FINAL)
    Q_PROPERTY(int cursorInEmbeddedLink READ getCursorInEmbeddedLink WRITE setCursorInEmbeddedLink NOTIFY cursorInEmbeddedLinkChanged FINAL)
    QML_ELEMENT

public:
    explicit PostUtils(QObject* parent = nullptr);

    Q_INVOKABLE static QString extractDidFromUri(const QString& uri);
    Q_INVOKABLE void checkPostExists(const QString& uri, const QString& cid);
    Q_INVOKABLE void post(const QString& text, const QStringList& imageFileNames, const QStringList& altTexts,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid,
                          const QString& quoteUri, const QString& quoteCid,
                          const QStringList& labels, const QString& language);
    Q_INVOKABLE void post(const QString& text, const LinkCard* card,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid,
                          const QString& quoteUri, const QString& quoteCid,
                          const QStringList& labels, const QString& language);
    Q_INVOKABLE void postVideo(const QString& text, const QString& videoFileName, const QString& videoAltText,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid,
                          const QString& quoteUri, const QString& quoteCid,
                          const QStringList& labels, const QString& language);
    Q_INVOKABLE void addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollower, bool allowFollowing, const QStringList& allowList, bool allowNobody, const QStringList& hiddenReplies);
    Q_INVOKABLE void addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollower, bool allowFollowing, const ListViewBasicList& allowList, bool allowNobody, const QStringList& hiddenReplies);
    Q_INVOKABLE void addPostgate(const QString& uri, bool disableEmbedding, const QStringList& detachedEmbeddingUris);
    Q_INVOKABLE void undoThreadgate(const QString& threadgateUri, const QString& cid);
    Q_INVOKABLE void undoPostgate(const QString& postUri);
    Q_INVOKABLE void detachQuote(const QString& uri, const QString& embeddingUri, const QString& embeddingCid, bool detach);
    Q_INVOKABLE void repost(const QString& uri, const QString& cid);
    Q_INVOKABLE void undoRepost(const QString& repostUri, const QString& origPostCid);
    Q_INVOKABLE void like(const QString& uri, const QString& cid);
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
    Q_INVOKABLE void setHighlightDocument(QQuickTextDocument* doc, const QString& highlightColor,
                                          int maxLength = -1, const QString& lengthExceededColor = {});
    Q_INVOKABLE void setHighLightMaxLength(int maxLength);
    Q_INVOKABLE void extractMentionsAndLinks(const QString& text,const QString& preeditText, int cursor);
    Q_INVOKABLE WebLink makeWebLink(const QString& name, const QString& link, int startIndex, int endIndex) const;
    Q_INVOKABLE void addEmbeddedLink(const WebLink& link);
    Q_INVOKABLE void updatedEmbeddedLink(int linkIndex, const WebLink& link);
    Q_INVOKABLE void removeEmbeddedLink(int linkIndex);

    // Make updates due to cursor moving to a new position
    Q_INVOKABLE void updateCursor(int cursor);

    // Make updates due to changed text
    Q_INVOKABLE void updateText(const QString& prevText, const QString& text);

    Q_INVOKABLE void cacheTags(const QString& text);

    // Returns a new text up to cursor with the last type char transformed into font.
    // Returns null string if last typed char was not a transformable char.
    Q_INVOKABLE QString applyFontToLastTypedChars(const QString& text,const QString& preeditText,
                                                  int cursor, int numChars, QEnums::FontType font);

    Q_INVOKABLE int getEditMentionIndex() const { return mEditMentionIndex; }
    Q_INVOKABLE int getEditTagIndex() const { return mEditTagIndex; }
    Q_INVOKABLE QString linkiFy(const QString& text, const QString& colorName);
    Q_INVOKABLE int getLinkShorteningReduction() const { return mLinkShorteningReduction; }
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

    const QString& getTextWithoutLinks() const { return mTextWithoutLinks; }
    void setTextWithoutLinks(const QString& text);
    const QString& getEditMention() const { return mEditMention; }
    void setEditMention(const QString& mention);
    const QString& getEditTag() const { return mEditTag; }
    void setEditTag(const QString& tag);
    const QString& getFirstWebLink() const { return mFirstWebLink; }
    void setFirstWebLink(const QString& link);
    void setFirstWebLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor);
    bool isCursorInFirstWebLink() const { return mCursorInFirstWebLink; }
    void setCursorInFirstWebLink(bool inLink);
    const QString& getFirstPostLink() const { return mFirstPostLink; }
    void setFirstPostLink(const QString& link);
    void setFirstPostLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor);
    bool isCursorInFirstPostLink() const { return mCursorInFirstPostLink; }
    void setCursorInFirstPostLink(bool inLink);
    const QString& getFirstFeedLink() const { return mFirstFeedLink; }
    void setFirstFeedLink(const QString& link);
    void setFirstFeedLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor);
    bool isCursorInFirstFeedLink() const { return mCursorInFirstFeedLink; }
    void setCursorInFirstFeedLink(bool inLink);
    const QString& getFirstListLink() const { return mFirstListLink; }
    void setFirstListLink(const QString& link);
    void setFirstListLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor);
    bool isCursorInFirstListLink() const { return mCursorInFirstListLink; }
    void setCursorInFirstListLink(bool inLink);
    const WebLink::List& getWebLinks() const { return mWebLinks; }
    void setWebLinks(const WebLink::List& webLinks);
    int getCursorInWebLink() const { return mCursorInWebLink; }
    void setCursorInWebLink(int index);
    const WebLink::List& getEmbeddedLinks() const { return mEmbeddedLinks; }
    void setEmbeddedLinks(const WebLink::List& embeddedLinks);
    int getCursorInEmbeddedLink() const { return mCursorInEmbeddedLink; }
    void setCursorInEmbeddedLink(int index);

signals:
    void checkPostExistsOk(QString uri, QString cid);
    void checkPostExistsFailed(QString uri, QString cid, QString error);
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
    void textWithoutLinksChanged();
    void editMentionChanged();
    void editTagChanged();
    void firstWebLinkChanged();
    void cursorInFirstWebLinkChanged();
    void firstPostLinkChanged();
    void cursorInFirstPostLinkChanged();
    void firstFeedLinkChanged();
    void cursorInFirstFeedLinkChanged();
    void firstListLinkChanged();
    void cursorInFirstListLinkChanged();
    void webLinksChanged();
    void cursorInWebLinkChanged();
    void embeddedLinksChanged();
    void cursorInEmbeddedLinkChanged();
    void quotePost(QString uri, QString cid, QString text, BasicProfile author, QDateTime);
    void quoteFeed(GeneratorView feed);
    void quoteList(ListView list);
    void getPostgateOk(Postgate postgate);
    void getPostgateFailed(QString error);
    void videoUploadLimits(VideoUploadLimits limits);
    void checkVideoLimitsOk(VideoUploadLimits limits);
    void checkVideoLimitsFailed(QString error);
    void languageIdentified(QString languageCode, int index);

private:
    void continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid, const QStringList& labels);
    void continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex = 0);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid, const QStringList& labels);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(const LinkCard* card, QImage thumb, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePostVideo(const QString& videoFileName, const QString& videoAltText, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid, const QStringList& labels);
    void continuePostVideo(const QString& videoFileName, const QString& videoAltText, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continueRepost(const QString& uri, const QString& cid);
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
    void updateEmbeddedLinksInsertedText(const TextDiffer::Result& diff, const QString& text);
    void updateEmbeddedLinksDeletedText(const TextDiffer::Result& diff);
    void updateEmbeddedLinksUpdatedText(const TextDiffer::Result& diff, const QString& text);
    bool facetOverlapsWithEmbeddedLink(const ATProto::RichTextMaster::ParsedMatch& facet) const;

    QNetworkAccessManager* mNetwork;
    ATProto::PostMaster* postMaster();
    ImageReader* imageReader();
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    QString mEditMention; // Mention currently being edited (without @-symbol)
    int mEditMentionIndex = 0;
    QString mEditTag; // Tag currently being edited (without #-symbol)
    int mEditTagIndex = 0;
    QString mFirstPostLink; // HTTPS link to a post
    bool mCursorInFirstPostLink = false;
    QString mFirstFeedLink; // HTTPS link to feed generator
    bool mCursorInFirstFeedLink = false;
    QString mFirstListLink; // HTTPS link to list ivew
    bool mCursorInFirstListLink = false;
    QString mFirstWebLink;
    bool mCursorInFirstWebLink = false;
    int mLinkShorteningReduction = 0;
    QString mTextWithoutLinks;

    WebLink::List mWebLinks;
    int mCursorInWebLink = -1;
    WebLink::List mEmbeddedLinks;
    int mCursorInEmbeddedLink = -1;

    std::unique_ptr<ImageReader> mImageReader;
    FacetHighlighter mFacetHighlighter;
    bool mPickingPhoto = false;

    std::unordered_map<int, int> mIndexLanguageIdentificationRequestIdMap;
    std::unordered_map<int, int> mLanguageIdentificationRequestIdIndexMap;

    static int sNextRequestId;
};

}
