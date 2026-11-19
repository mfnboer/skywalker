// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once

#include "profile.h"
#include <QObject>

namespace Skywalker {

class BasicProfileView : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString did READ getDid CONSTANT FINAL)
    Q_PROPERTY(QString followingUri READ getFollowingUri NOTIFY followingUriChanged FINAL)
    Q_PROPERTY(QString error READ getError CONSTANT FINAL)
    QML_ELEMENT

public:
    using Ptr = std::unique_ptr<BasicProfileView>;

    BasicProfileView(QObject* parent = nullptr);
    explicit BasicProfileView(const BasicProfile& profile, QObject* parent = nullptr);
    BasicProfileView(const QString& did, const QString& error, QObject* parent = nullptr);

    const QString& getDid() const { return mDid; }

    QString getFollowingUri() const;
    void setFollowingUri(const QString& followingUri);

    Q_INVOKABLE bool hasError() const { return !mError.isEmpty(); }
    const QString& getError() const { return mError; }

    Q_INVOKABLE bool isGood() const { return !hasError(); }

signals:
    void followingUriChanged();

private:
    BasicProfile mProfile;
    QString mDid;
    std::optional<QString> mFollowingUri;
    QString mError;
};

}