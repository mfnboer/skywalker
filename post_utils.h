// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "skywalker.h"
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

    Q_INVOKABLE void post(QString text, const QStringList& imageFileNames);
    Q_INVOKABLE void pickPhoto() const;
    Q_INVOKABLE QString highlightMentionsAndLinks(const QString& text);
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
    ATProto::Client* bskyClient();

    Skywalker* mSkywalker = nullptr;
    QString mFirstWebLink;
    int mLinkShorteningReduction = 0;
};

}
