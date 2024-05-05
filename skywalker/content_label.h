// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ContentLabel {
    Q_GADGET
    Q_PROPERTY(QString did READ getDid FINAL)
    Q_PROPERTY(QString labelId READ getLabelId FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(bool isSystemLabel READ isSystemLabel FINAL)
    QML_VALUE_TYPE(contentlabel)

public:
    ContentLabel() = default;
    ContentLabel(const QString& did, const QString& labelId, const QDateTime& createdAt) :
        mDid(did), mLabelId(labelId), mCreatedAt(createdAt) {}

    static bool isSystemLabelId(const QString& labelId) { return labelId.startsWith('!'); }

    const QString& getDid() const { return mDid; }
    const QString& getLabelId() const { return mLabelId; }
    const QDateTime& getCreatedAt() const { return mCreatedAt; }
    Q_INVOKABLE bool isSystemLabel() const { return isSystemLabelId(mLabelId); }

private:
    QString mDid;
    QString mLabelId;
    QDateTime mCreatedAt;
};

using ContentLabelList = QList<ContentLabel>;

}

Q_DECLARE_METATYPE(::Skywalker::ContentLabel)
