// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "songlink.h"
#include "songlink_cache.h"
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

// API DOC: https://linktree.notion.site/API-d0ebe08a5e304a55928405eb682f6741
static constexpr char const* SONGLINK_BASE_URL = "https://api.song.link/v1-alpha.1/";

namespace Skywalker {

Songlink::Songlink(QObject* parent) :
    QObject(parent),
    WebServiceBase(SONGLINK_BASE_URL, nullptr)
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
    static std::unordered_map<QString, QStringList> MUSIC_HOSTS = SonglinkLinks::getPlatformHosts();

    qDebug() << "Is music link:" << link;
    const QUrl url(link);

    if (!url.isValid())
    {
        qDebug() << "Invalid link:" << link;
        return false;
    };

    if (!MUSIC_HOSTS.contains(url.host()))
    {
        qDebug() << "Not a music link:" << link;
        return false;
    }

    const auto& paths = MUSIC_HOSTS[url.host()];

    if (paths.empty())
    {
        qDebug() << "Is music:" << link;
        return true;
    }

    for (const auto& path : paths)
    {
        if (url.path().startsWith(path))
        {
            qDebug() << "Is music:" << link;
            return true;
        }
    }

    qDebug() << "Not a music link:" << link;
    return false;
}

bool Songlink::isCached(const QString& link)
{
    return SonglinkCache::instance().contains(QUrl(link));
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

    const SonglinkLinks* links = SonglinkCache::instance().get(QUrl(musicLink));

    if (links)
    {
        emit linksFound(*links);
        return;
    }

    const QUrl musicUrl(musicLink);

    if (!musicUrl.isValid())
    {
        qWarning() << "Invalid URL:" << musicLink;
        emit failure(tr("Could not get song links"));
        return;
    }

    init();
    setInProgress(true);

    Params params {
        { "url", musicUrl.toEncoded() },
        { "userCountry", mCountryCode }
    };

    sendRequest("links", params,
        [this, musicLink](QNetworkReply* reply){
            setInProgress(false);
            processGetLinksReply(reply, musicLink);
        },
        [this](QNetworkReply* reply, auto errCode){
            setInProgress(false);
            qWarning() << "Get links error:" << reply->request().url() << "error:" <<
                errCode << reply->errorString();
        },
        [this](QNetworkReply* reply){
            setInProgress(false);
            qWarning() << "Get links SSL error:" <<  reply->request().url();
        });
}

void Songlink::processGetLinksReply(QNetworkReply* reply, const QString& musicLink)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << "Get links failed:" << reply->request().url() << "code:" << statusCode << "error:" <<
            reply->error() << reply->errorString();

        if (statusCode >= 400 && statusCode < 500)
        {
            const SonglinkLinks noLinks;
            SonglinkCache::instance().put(QUrl(musicLink), noLinks);
            emit linksFound(noLinks);
        }
        else
        {
            emit failure(reply->errorString());
        }

        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const auto jsonObject = json.object();
    const ATProto::XJsonObject xjson(jsonObject);

    try {
        const auto linksJson = xjson.getRequiredJsonObject("linksByPlatform");
        auto links = SonglinkLinks::fromJson(linksJson);
        SonglinkCache::instance().put(QUrl(musicLink), *links);
        emit linksFound(*links);
    }
    catch (ATProto::InvalidJsonException& e) {
        qDebug() << QString::fromUtf8(data);
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
