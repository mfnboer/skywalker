// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "web_service_base.h"
#include "wrapped_skywalker.h"
#include <QNetworkAccessManager>

namespace Skywalker {

class ForYou : public WrappedSkywalker, public WebServiceBase
{
    Q_OBJECT
    Q_PROPERTY(QString uri READ getUri CONSTANT FINAL)
    Q_PROPERTY(QString avatar READ getAvatar NOTIFY avatarChanged FINAL)
    QML_ELEMENT

public:
    using AlsoLikedSuccessCb = ATProto::Client::GetFeedSuccessCb;
    using ErrorCb = ATProto::Client::ErrorCb;

    explicit ForYou(QObject* parent = nullptr);
    explicit ForYou(QNetworkAccessManager* network, QObject* parent = nullptr);

    // The post with postUri will be returned as first post of the first page.
    void alsoLiked(const QString& postUri, const std::optional<QString>& cursor,
                   const AlsoLikedSuccessCb& successCb, const ErrorCb& errorCb);

    QString getUri() const { return FOR_YOU_AT_URI; }
    QString getAvatar();

signals:
    void avatarChanged();

private:
    struct AlsoLikedPost
    {
        QString mPost;

        using SharedPtr = std::shared_ptr<AlsoLikedPost>;
        using List = std::vector<SharedPtr>;
        static SharedPtr fromJson(const QJsonObject& json);
    };

    struct AlsoLikedFeed
    {
        AlsoLikedPost::List mFeed;
        std::optional<QString> mCursor;

        using SharedPtr = std::shared_ptr<AlsoLikedFeed>;
        static SharedPtr fromJson(const QJsonObject& json);
    };

    void continueAlsoLiked(QNetworkReply* reply, const QString& postUri, const std::optional<QString>& cursor,
                           const AlsoLikedSuccessCb& successCb, const ErrorCb& errorCb);
    void initAvatar();

    static QString sAvatar;
    static constexpr char const* FOR_YOU_AT_URI = "at://did:plc:3guzzweuqraryl3rdkimjamk/app.bsky.feed.generator/for-you";
};

}
