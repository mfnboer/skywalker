// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "constellation.h"
#include "skywalker.h"
#include <atproto/lib/xjson.h>
#include <QJsonDocument>

namespace Skywalker {

static constexpr char const* CONSTELLATION_BASE_URL = "https://constellation.microcosm.blue/xrpc/";

Constellation::Constellation(QNetworkAccessManager* network) :
    WebServiceBase(CONSTELLATION_BASE_URL, network)
{
    Q_ASSERT(network);
    setUserAgent(QString("%1/%2 (@skywalker.thereforeiam.eu)").arg(Skywalker::APP_NAME, Skywalker::VERSION));
}

void Constellation::getBackLinks(const QString& subject, const QString& source, const std::vector<QString>& filterDids,
                                 std::optional<int> limit,
                                 const BacklinksCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get backlinks:" << subject << "source:" << source << "filterDids:" << filterDids.size() << "limit:" << limit;

    if (subject.isEmpty())
    {
        qWarning() << "No subject, source:" << source;
        return;
    }

    if (source.isEmpty())
    {
        qWarning() << "No source, subcject:" << subject;
        return;
    }

    Params params{{"subject", subject}, {"source", source}};
    addOptionalIntParam(params, "limit", limit);
    addStringListParam(params, "did", filterDids);

    sendRequest("blue.microcosm.links.getBacklinks", params,
        [successCb, errorCb](QNetworkReply* reply){
            qDebug() << "Get backlinks reply:" << reply->request().url() << reply->error();
            const auto data = reply->readAll();

            if (reply->error() != QNetworkReply::NoError)
            {
                QString msg(data);
                qWarning() << "Get backlinks failed:" << reply->request().url() << "error:" << reply->error() << reply->errorString() << "msg:" << msg;
                errorCb("Error", reply->errorString());
                return;
            }


            const QJsonDocument json(QJsonDocument::fromJson(data));
            Backlinks::SharedPtr backlinks;

            try {
                backlinks = Backlinks::fromJson(json.object());
            } catch (ATProto::InvalidJsonException& e) {
                qWarning() << e.msg();
                errorCb("InvalidJson", e.msg());
                return;
            }

            successCb(backlinks);
        });
}

void Constellation::hasBackLinks(const QString& subject, const QString& source, const std::vector<QString>& filterDids,
                                 const HasBacklinksCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Has backlinks:" << subject << "source:" << source << "filterDids:" << filterDids.size();

    if (subject.isEmpty())
    {
        qWarning() << "No subject, source:" << source;
        return;
    }

    if (source.isEmpty())
    {
        qWarning() << "No source, subcject:" << subject;
        return;
    }

    Params params{{"subject", subject}, {"source", source}, {"limit", "1"}};
    addStringListParam(params, "did", filterDids);

    sendRequest("blue.microcosm.links.getBacklinks", params,
        [successCb, errorCb](QNetworkReply* reply){
            qDebug() << "Has backlinks reply:" << reply->request().url() << reply->error();
            const auto data = reply->readAll();

            if (reply->error() != QNetworkReply::NoError)
            {
                QString msg(data);
                qWarning() << "Has backlinks failed:" << reply->request().url() << "error:" << reply->error() << reply->errorString() << "msg:" << msg;
                errorCb("Error", reply->errorString());
                return;
            }


            const QJsonDocument json(QJsonDocument::fromJson(data));
            const int total = json.object().value("total").toInt(0);
            successCb(total > 0);
        });
}

Constellation::Record::SharedPtr Constellation::Record::fromJson(const QJsonObject& json)
{
    ATProto::XJsonObject xjson(json);
    auto record = std::make_shared<Constellation::Record>();
    record->mDid = xjson.getRequiredString("did");
    record->mCollection = xjson.getRequiredString("collection");
    record->mRkey = xjson.getRequiredString("rkey");
    return record;
}

Constellation::Backlinks::SharedPtr Constellation::Backlinks::fromJson(const QJsonObject& json)
{
    ATProto::XJsonObject xjson(json);
    auto backlinks = std::make_shared<Constellation::Backlinks>();
    backlinks->mTotal = xjson.getRequiredInt("total");
    backlinks->mRecords = xjson.getRequiredVector<Record>("records");
    return backlinks;
}

}
