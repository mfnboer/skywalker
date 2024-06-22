// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "facet_highlighter.h"
#include "generator_view.h"
#include "image_reader.h"
#include "link_card.h"
#include "list_view.h"
#include "presence.h"
#include "profile.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/post_master.h>
#include <QImage>
#include <QQuickTextDocument>

namespace Skywalker {

class PostUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
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
    QML_ELEMENT

public:
    explicit PostUtils(QObject* parent = nullptr);

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
    Q_INVOKABLE void addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollowing, const QStringList& allowList);
    Q_INVOKABLE void addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollowing, const ListViewBasicList& allowList);
    Q_INVOKABLE void undoThreadgate(const QString& threadgateUri, const QString& cid);
    Q_INVOKABLE void repost(const QString& uri, const QString& cid);
    Q_INVOKABLE void undoRepost(const QString& repostUri, const QString& origPostCid);
    Q_INVOKABLE void like(const QString& uri, const QString& cid);
    Q_INVOKABLE void undoLike(const QString& likeUri, const QString& cid);
    Q_INVOKABLE void deletePost(const QString& postUri, const QString& cid);
    Q_INVOKABLE void batchDeletePosts(const QStringList& postUris);
    Q_INVOKABLE bool pickPhoto();
    Q_INVOKABLE void savePhoto(const QString& sourceUrl);
    Q_INVOKABLE void dropPhoto(const QString& source);
    Q_INVOKABLE QString cutPhotoRect(const QString& source, const QRect& rect, const QSize& scaledSize);
    Q_INVOKABLE void setHighlightDocument(QQuickTextDocument* doc, const QString& highlightColor,
                                          int maxLength = -1, const QString& lengthExceededColor = {});
    Q_INVOKABLE void setHighLightMaxLength(int maxLength);
    Q_INVOKABLE void extractMentionsAndLinks(const QString& text,const QString& preeditText,
                                             int cursor);
    Q_INVOKABLE void cacheTags(const QString& text);

    // Returns a new text up to cursor with the last type char transformed into font.
    // Returns null string if last typed char was not a transformable char.
    Q_INVOKABLE QString applyFontToLastTypedChars(const QString& text,const QString& preeditText,
                                                  int cursor, int numChars, QEnums::FontType font);

    Q_INVOKABLE int getEditMentionIndex() const { return mEditMentionIndex; }
    Q_INVOKABLE int getEditTagIndex() const { return mEditTagIndex; }
    Q_INVOKABLE QString linkiFy(const QString& text, const QString& colorName);
    Q_INVOKABLE int getLinkShorteningReduction() const { return mLinkShorteningReduction; };
    Q_INVOKABLE void getQuotePost(const QString& httpsUri);
    Q_INVOKABLE void getQuoteFeed(const QString& httpsUri);
    Q_INVOKABLE void getQuoteList(const QString& httpsUri);

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

signals:
    void postOk(QString uri, QString cid);
    void postFailed(QString error);
    void threadgateOk();
    void threadgateFailed(QString error);
    void undoThreadgateOk();
    void undoThreadgateFailed(QString error);
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
    void postDeletedOk();
    void postDeletedFailed(QString error);
    void photoPicked(QString imgSource);
    void photoPickFailed(QString error);
    void photoPickCanceled();
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
    void quotePost(QString uri, QString cid, QString text, BasicProfile author, QDateTime);
    void quoteFeed(GeneratorView feed);
    void quoteList(ListView list);

private:
    void continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid, const QStringList& labels);
    void continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex = 0);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid, const QStringList& labels);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(const LinkCard* card, QImage thumb, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continueRepost(const QString& uri, const QString& cid);
    void sharePhoto(int fd);
    void cancelPhotoPicking();

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
    std::unique_ptr<ImageReader> mImageReader;
    FacetHighlighter mFacetHighlighter;
    bool mPickingPhoto = false;
};

}
