// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "image_reader.h"
#include "link_card.h"
#include "presence.h"
#include "skywalker.h"
#include <atproto/lib/post_master.h>
#include <QImage>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class PostUtils : public QObject, public Presence
{
    Q_OBJECT
    Q_PROPERTY(Skywalker* skywalker READ getSkywalker WRITE setSkywalker NOTIFY skywalkerChanged FINAL REQUIRED)
    Q_PROPERTY(QString firstWebLink READ getFirstWebLink WRITE setFirstWebLink NOTIFY firstWebLinkChanged FINAL)
    Q_PROPERTY(QString firstPostLink READ getFirstPostLink WRITE setFirstPostLink NOTIFY firstPostLinkChanged FINAL)
    QML_ELEMENT

public:
    explicit PostUtils(QObject* parent = nullptr);

    Q_INVOKABLE void post(QString text, const QStringList& imageFileNames,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid,
                          const QString& quoteUri, const QString& quoteCid);
    Q_INVOKABLE void post(QString text, const LinkCard* card,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid,
                          const QString& quoteUri, const QString& quoteCid);
    Q_INVOKABLE void repost(const QString& uri, const QString& cid);
    Q_INVOKABLE void undoRepost(const QString& repostUri, const QString& origPostCid);
    Q_INVOKABLE void pickPhoto() const;
    Q_INVOKABLE QString highlightMentionsAndLinks(const QString& text,const QString& preeditText,
                                                  int cursor, const QString& color);
    Q_INVOKABLE int graphemeLength(const QString& text) const;
    Q_INVOKABLE int getLinkShorteningReduction() const { return mLinkShorteningReduction; };
    Q_INVOKABLE void getQuotePost(const QString& httpsUri);

    Skywalker* getSkywalker() const { return mSkywalker; }
    void setSkywalker(Skywalker* skywalker);

    const QString& getFirstWebLink() const { return mFirstWebLink; }
    void setFirstWebLink(const QString& link);
    const QString& getFirstPostLink() const { return mFirstPostLink; }
    void setFirstPostLink(const QString& link);

signals:
    void skywalkerChanged();
    void postOk();
    void postFailed(QString error);
    void postProgress(QString msg);
    void repostOk();
    void repostFailed(QString error);
    void repostProgress(QString msg);
    void undoRepostOk();
    void undoRepostFailed(QString error);
    void photoPicked(QString filename);
    void firstWebLinkChanged();
    void firstPostLinkChanged();
    void quotePost(QString uri, QString cid, QString text, BasicProfile author, QDateTime);

private:
    void continuePost(const QStringList& imageFileNames, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid);
    void continuePost(const QStringList& imageFileNames, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex = 0);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                      const QString& quoteUri, const QString& quoteCid);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(const LinkCard* card, QImage thumb, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continueRepost(const QString& uri, const QString& cid);
    ATProto::Client* bskyClient();
    ATProto::PostMaster* postMaster();
    ImageReader* imageReader();

    Skywalker* mSkywalker = nullptr;
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    QString mFirstPostLink; // HTTPS link to a post
    QString mFirstWebLink;
    int mLinkShorteningReduction = 0;
    std::unique_ptr<ImageReader> mImageReader;
};

}
