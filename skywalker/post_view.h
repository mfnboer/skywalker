// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
#include <QObject>

namespace Skywalker {

class PostView : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString uri READ getUri CONSTANT FINAL)
    Q_PROPERTY(QString likeUri READ getLikeUri NOTIFY likeUriChanged FINAL)
    Q_PROPERTY(bool notFound READ isNotFound CONSTANT FINAL)
    Q_PROPERTY(bool blocked READ isBlocked CONSTANT FINAL)
    Q_PROPERTY(QString error READ getError CONSTANT FINAL)
    QML_ELEMENT

public:
    using Ptr = std::unique_ptr<PostView>;

    explicit PostView(Post::Ptr post, QObject* parent = nullptr);
    PostView(const QString& uri, const QString& error, QObject* parent = nullptr);

    const QString& getUri() const { return mUri; }
    const QString getCid() const;

    QString getLikeUri() const;
    void setLikeUri(const QString& likeUri);

    bool isNotFound() const;
    bool isBlocked() const;
    Q_INVOKABLE bool hasError() const { return !mError.isEmpty(); }
    const QString& getError() const { return mError; }

    Q_INVOKABLE bool isGood() const { return !isNotFound() && !isBlocked() && !hasError(); }

signals:
    void likeUriChanged();

private:
    Post::Ptr mPost;
    QString mUri;
    std::optional<QString> mLikeUri;
    QString mError;
};

}
