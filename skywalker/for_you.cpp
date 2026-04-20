// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "for_you.h"

namespace Skywalker {

static constexpr char const* FOR_YOU_BASE_URL = "https://foryou.club/";

QString ForYou::sAvatar;

ForYou::ForYou(QObject* parent) :
    WrappedSkywalker(parent)
{
}

ForYou::ForYou(QNetworkAccessManager* network, QObject* parent) :
    WrappedSkywalker(parent),
    mNetwork(network)
{
    Q_ASSERT(mNetwork);
}

void ForYou::alsoLiked(const QString& postUri, const std::optional<QString>& cursor,
                       const AlsoLikedSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Also liked:" << postUri << "cursor:" << cursor.value_or("");
    Params params{{"format", "json"}, {"limit", "20"}, {"post", postUri}};

    if (cursor)
        params.append({"cursor", *cursor});

    QNetworkRequest request(buildUrl("also-liked", params));
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), postUri, cursor, reply, successCb, errorCb]{
        if (!presence)
            return;

        continueAlsoLiked(reply, postUri, cursor, successCb, errorCb);
    });
}

void ForYou::continueAlsoLiked(QNetworkReply* reply, const QString& postUri, const std::optional<QString>& cursor,
                               const AlsoLikedSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Also liked reply:" << reply->request().url() << reply->error();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Also liked failed:" << reply->request().url() << "error:" << reply->error() << reply->errorString();
        errorCb("Error", reply->errorString());
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    AlsoLikedFeed::SharedPtr feed;

    try {
        feed = AlsoLikedFeed::fromJson(json.object());
    } catch (ATProto::InvalidJsonException& e) {
        qWarning() << e.msg();
        errorCb("InvalidJson", e.msg());
    }

    std::vector<QString> uris;

    if (!cursor)
        uris.push_back(postUri);

    for (const auto& post : feed->mFeed)
        uris.push_back(post->mPost);

    if (uris.empty())
    {
        qDebug() << "No other likes:" << reply->request().url();
        successCb(std::make_shared<ATProto::AppBskyFeed::OutputFeed>());
        return;
    }

    bskyClient()->getPosts(uris,
        [presence=getPresence(), feed, successCb](ATProto::AppBskyFeed::PostView::List posts){
            if (!presence)
                return;

            auto output = std::make_shared<ATProto::AppBskyFeed::OutputFeed>();
            output->mCursor = feed->mCursor;

            for (const auto& post : posts)
            {
                auto feedViewPost = std::make_shared<ATProto::AppBskyFeed::FeedViewPost>();
                feedViewPost->mPost = post;
                output->mFeed.push_back(feedViewPost);
            }

            successCb(output);
        },
        errorCb);
}

QUrl ForYou::buildUrl(const QString& endpoint, const Params& params) const
{
    QUrl url(FOR_YOU_BASE_URL + endpoint);
    QUrlQuery query;

    for (const auto& kv : params)
        query.addQueryItem(kv.first, QUrl::toPercentEncoding(kv.second));

    url.setQuery(query);
    return url;
}

ForYou::AlsoLikedPost::SharedPtr ForYou::AlsoLikedPost::fromJson(const QJsonObject& json)
{
    ATProto::XJsonObject xjson(json);
    auto post = std::make_shared<AlsoLikedPost>();
    post->mPost = xjson.getRequiredString("post");
    return post;
}

ForYou::AlsoLikedFeed::SharedPtr ForYou::AlsoLikedFeed::fromJson(const QJsonObject& json)
{
    ATProto::XJsonObject xjson(json);
    auto feed = std::make_shared<AlsoLikedFeed>();
    feed->mFeed = xjson.getOptionalVector<AlsoLikedPost>("feed");
    feed->mCursor = xjson.getOptionalString("cursor");
    return feed;
}

QString ForYou::getAvatar()
{
    initAvatar();
    return sAvatar;
}

void ForYou::initAvatar()
{
    if (!sAvatar.isEmpty())
        return;

    if (!bskyClient())
        return;

    qDebug() << "Init avatar";
    bskyClient()->getFeedGenerator(FOR_YOU_AT_URI,
        [this, presence=getPresence()](ATProto::AppBskyFeed::GetFeedGeneratorOutput::SharedPtr output){
            if (!presence)
                return;

            if (output->mView->mAvatar)
                sAvatar = *output->mView->mAvatar;

            emit avatarChanged();
        },
        [](const QString& error, const QString& msg){
            qWarning() << "Failed to init avatar:" << error << "-" << msg;
        });
}

}
