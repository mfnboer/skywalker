// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ContentLabel {
    Q_GADGET
    Q_PROPERTY(QString did READ getDid FINAL)
    Q_PROPERTY(QString text READ getText FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    QML_VALUE_TYPE(contentlabel)

public:
    ContentLabel() = default;

    ContentLabel(const QString& did, const QString& text, const QDateTime& createdAt) :
        mDid(did), mText(text), mCreatedAt(createdAt) {}

    const QString& getDid() const { return mDid; }
    const QString& getText() const { return mText; }
    const QDateTime& getCreatedAt() const { return mCreatedAt; }
    Q_INVOKABLE bool isSystemLabel() const { return mText.startsWith("!"); }

private:
    QString mDid;
    QString mText;
    QDateTime mCreatedAt;
};

using ContentLabelList = QList<ContentLabel>;

}

Q_DECLARE_METATYPE(::Skywalker::ContentLabel)
