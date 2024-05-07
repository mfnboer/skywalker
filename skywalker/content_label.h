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
    ContentLabel() = default;
    ContentLabel(const QString& did, const QString& uri, const QString& cid,
                 const QString& labelId, const QDateTime& createdAt) :
        mDid(did),
        mUri(uri),
        mCid(cid),
        mLabelId(labelId),
        mCreatedAt(createdAt)
    {}

    static bool isSystemLabelId(const QString& labelId) { return labelId.startsWith('!'); }

    const QString& getDid() const { return mDid; }
    const QString& getUri() const { return mUri; }
    const QString& getCid() const { return mCid; }
    const QString& getLabelId() const { return mLabelId; }
    const QDateTime& getCreatedAt() const { return mCreatedAt; }
    Q_INVOKABLE bool isSystemLabel() const { return isSystemLabelId(mLabelId); }

    Q_INVOKABLE bool appliesToActor() const;
    Q_INVOKABLE QString getActorDid() const;

private:
    QString mDid;
    QString mUri;
    QString mCid;
    QString mLabelId;
    QDateTime mCreatedAt;
};

using ContentLabelList = QList<ContentLabel>;

}

Q_DECLARE_METATYPE(::Skywalker::ContentLabel)
