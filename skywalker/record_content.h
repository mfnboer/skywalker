// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QQmlEngine>

namespace Skywalker {

class RecordContent
{
    Q_GADGET
    Q_PROPERTY(QString text READ getText FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    QML_VALUE_TYPE(recordcontent)

public:
    RecordContent() = default;
    RecordContent(const QString& text, const QDateTime& createdAt);

    Q_INVOKABLE bool isNull() const { return mText.isEmpty() && !mCreatedAt.isValid(); }
    const QString& getText() const { return mText; }
    const QDateTime& getCreatedAt() const { return mCreatedAt; }

private:
    QString mText;
    QDateTime mCreatedAt;
};

}