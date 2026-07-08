// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "web_service_base.h"
#include <QJsonObject>

namespace Skywalker {

// See https://constellation.microcosm.blue/
class Constellation : public WebServiceBase
{
public:
    struct Record
    {
        QString mDid;
        QString mCollection;
        QString mRkey;

        using SharedPtr = std::shared_ptr<Record>;
        using List = std::vector<SharedPtr>;
        static SharedPtr fromJson(const QJsonObject& json);
    };

    struct Backlinks
    {
        int mTotal = 0;
        Record::List mRecords;

        using SharedPtr = std::shared_ptr<Backlinks>;
        static SharedPtr fromJson(const QJsonObject& json);
    };

    using BacklinksCb = std::function<void(Backlinks::SharedPtr)>;
    using HasBacklinksCb = std::function<void(bool hasLinks)>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    explicit Constellation(QNetworkAccessManager* network);

    void getBackLinks(const QString& subject, const QString& source, const std::vector<QString>& filterDids,
                      std::optional<int> limit,
                      const BacklinksCb& successCb, const ErrorCb& errorCb);

    void hasBackLinks(const QString& subject, const QString& source, const std::vector<QString>& filterDids,
                      const HasBacklinksCb& successCb, const ErrorCb& errorCb);
};

}
