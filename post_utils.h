// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "image_reader.h"
#include "link_card.h"
#include "skywalker.h"
#include <QImage>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class PostUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Skywalker* skywalker READ getSkywalker WRITE setSkywalker NOTIFY skywalkerChanged FINAL REQUIRED)
    Q_PROPERTY(QString firstWebLink READ getFirstWebLink WRITE setFirstWebLink NOTIFY firstWebLinkChanged FINAL)
    QML_ELEMENT

public:
    explicit PostUtils(QObject* parent = nullptr);

    Q_INVOKABLE void post(QString text, const QStringList& imageFileNames,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid);
    Q_INVOKABLE void post(QString text, const LinkCard* card,
                          const QString& replyToUri, const QString& replyToCid,
                          const QString& replyRootUri, const QString& replyRootCid);
    Q_INVOKABLE void pickPhoto() const;
    Q_INVOKABLE QString highlightMentionsAndLinks(const QString& text, const QString& preeditText, int cursor);
    Q_INVOKABLE int graphemeLength(const QString& text) const;
    Q_INVOKABLE int getLinkShorteningReduction() const { return mLinkShorteningReduction; };

    Skywalker* getSkywalker() const { return mSkywalker; }
    void setSkywalker(Skywalker* skywalker);

    const QString& getFirstWebLink() const { return mFirstWebLink; }
    void setFirstWebLink(const QString& link);

signals:
    void skywalkerChanged();
    void postOk();
    void postFailed(QString error);
    void postProgress(QString msg);
    void photoPicked(QString filename);
    void firstWebLinkChanged();

private:
    void continuePost(const QStringList& imageFileNames, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex = 0);
    void continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(const LinkCard* card, QImage thumb, ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    void continuePost(ATProto::AppBskyFeed::Record::Post::SharedPtr post);
    ATProto::Client* bskyClient();
    ImageReader* imageReader();

    Skywalker* mSkywalker = nullptr;
    QString mFirstWebLink;
    int mLinkShorteningReduction = 0;
    std::unique_ptr<ImageReader> mImageReader;
};

}
