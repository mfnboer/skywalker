// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/at_uri.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ContentLabel {
    Q_GADGET
    Q_PROPERTY(QString did READ getDid FINAL)
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString cid READ getCid FINAL)
    Q_PROPERTY(QString labelId READ getLabelId FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    QML_VALUE_TYPE(contentlabel)

public:
    ContentLabel() : mPrivate{std::make_shared<PrivateData>()} {}
    ContentLabel(const QString& did, const QString& uri, const QString& cid,
                 const QString& labelId, const QDateTime& createdAt) :
        mPrivate{std::make_shared<PrivateData>(
            did,
            uri,
            cid,
            labelId,
            createdAt)}
    {}

    static const std::unordered_set<QString>& getOverridableSystemLabelIds();
    static bool isSystemLabelId(const QString& labelId) { return labelId.startsWith('!'); }
    static bool isOverridableSytemLabelId(const QString& labelId);

    const QString& getDid() const { return mPrivate->mDid; }
    const QString& getUri() const { return mPrivate->mUri; }
    const QString& getCid() const { return mPrivate->mCid; }
    const QString& getLabelId() const { return mPrivate->mLabelId; }
    const QDateTime& getCreatedAt() const { return mPrivate->mCreatedAt; }
    Q_INVOKABLE bool isSystemLabel() const { return isSystemLabelId(mPrivate->mLabelId); }
    Q_INVOKABLE bool isOverridableSytemLabel() const { return isOverridableSytemLabelId(mPrivate->mLabelId); }

    Q_INVOKABLE bool appliesToActor() const;
    Q_INVOKABLE const QString& getActorDid() const;

private:
    struct PrivateData
    {
        QString mDid;
        QString mUri;
        QString mCid;
        QString mLabelId;
        QDateTime mCreatedAt;
    };
    std::shared_ptr<PrivateData> mPrivate;
};

using ContentLabelList = QList<ContentLabel>;

}

Q_DECLARE_METATYPE(::Skywalker::ContentLabel)
