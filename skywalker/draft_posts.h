// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>
#include <QObject>
#include <QtQmlIntegration>
#include <atproto/lib/post_master.h>

namespace Skywalker {

class DraftPosts : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DraftPosts(QObject* parent = nullptr);

    Q_INVOKABLE void saveDraftPost(const QString& text,
                                   const QStringList& imageFileNames, const QStringList& altTexts,
                                   const QString& replyToUri, const QString& replyToCid,
                                   const QString& replyRootUri, const QString& replyRootCid,
                                   const QString& quoteUri, const QString& quoteCid,
                                   const QStringList& labels);

signals:
    void saveDraftPostOk();
    void saveDraftPostFailed(QString error);

private:
    QString createDraftPostFileName(const QString& baseName) const;
    QString createDraftImageFileName(const QString& baseName, int seq) const;
    bool save(const ATProto::AppBskyFeed::Record::Post& post, const QString& draftsPath, const QString& baseName);
    ATProto::Blob::Ptr saveImage(const QString& imgName, const QString& draftsPath,
                                 const QString& baseName, int seq);
    void dropImage(const QString& draftsPath, const QString& baseName, int seq);
};

}
