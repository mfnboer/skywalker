// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "songlink.h"
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <unordered_set>

// API DOC: https://linktree.notion.site/API-d0ebe08a5e304a55928405eb682f6741

namespace Skywalker {

Songlink::Songlink(QObject* parent) :
    QObject(parent)
{
}

void Songlink::init()
{
    if (mNetwork)
        return;

    mNetwork = new QNetworkAccessManager(this);
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);

    QLocale locale;
    mCountryCode = QLocale::territoryToCode(locale.territory());
    qDebug() << "Country code:" << mCountryCode;
}

bool Songlink::isMusicLink(const QString& link)
{
    static std::unordered_set<QString> MUSIC_HOSTS = {
        "music.amazon.com",
        "audiomack.com",
        "www.deezer.com",
        "geo.music.apple.com",
        "soundcloud.com",
        "open.spotify.com",
        "music.youtube.com",
        "listen.tidal.com",
        "tidal.com",
        "play.anghami.com"
    };

    qDebug() << "Is music link:" << link;
    const QUrl url(link);

    if (!url.isValid())
    {
        qDebug() << "Invalid link:" << link;
        return false;
    };

    if (MUSIC_HOSTS.contains(url.host()))
    {
        qDebug() << "Is music:" << url.host();
        return true;
    }

    qDebug() << "Not a music link:" << link;
    return false;
}

QUrl Songlink::buildUrl(const QString& musicLink) const
{
    const QUrl musicUrl(musicLink);

    if (!musicUrl.isValid())
        return {};

    QUrl url("https://api.song.link/v1-alpha.1/links");
    QUrlQuery query;
    query.addQueryItem("url", musicUrl.toEncoded());
    query.addQueryItem("userCountry", mCountryCode);
    url.setQuery(query);
    return url;
}

void Songlink::getLinks(const QString& musicLink)
{
    if (mInProgress)
    {
        qDebug() << "Still in progress";
        return;
    }

    if (!isMusicLink(musicLink))
        return;

    init();
    const QUrl url = buildUrl(musicLink);

    if (!url.isValid())
    {
        qWarning() << "Failed to build url for:" << musicLink;
        emit failure(tr("Could not get song links"));
        return;
    }

    setInProgress(true);
    QNetworkRequest request(url);
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

        setInProgress(false);
        processGetLinksReply(reply);
    });

        connect(reply, &QNetworkReply::errorOccurred, this, [this, presence=getPresence(), reply](auto errCode){
        if (!presence)
            return;

        setInProgress(false);
        qWarning() << "Get links error:" << reply->request().url() << "error:" <<
            errCode << reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

        setInProgress(false);
        qWarning() << "Get links SSL error:" <<  reply->request().url();
    });
}

void Songlink::processGetLinksReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Get links failed:" << reply->request().url() << "error:" <<
            reply->error() << reply->errorString();
        emit failure(reply->errorString());
        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const auto jsonObject = json.object();
    const ATProto::XJsonObject xjson(jsonObject);

    try {
        const auto linksJson = xjson.getRequiredJsonObject("linksByPlatform");
        auto links = SonglinkLinks::fromJson(linksJson);
        emit linksFound(*links);
    }
    catch (ATProto::InvalidJsonException& e) {
        qWarning() << "Invalid JSON:" << e.msg();
        emit failure(tr("Failed to process song links"));
    }
}

void Songlink::setInProgress(bool inProgress)
{
    if (inProgress == mInProgress)
        return;

    mInProgress = inProgress;
    emit inProgressChanged();
}

}
