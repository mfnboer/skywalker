// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "report_utils.h"
#include "utils.h"

namespace Skywalker {

ReportUtils::ReportUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void ReportUtils::reportAuthor(const QString& did, QEnums::ReportReasonType reasonType,
                               const QString& details, const QString& labelerDid)
{
    qDebug() << "Report author:" << did << reasonType << details;

    bskyClient()->reportAuthor(did, ATProto::ComATProtoModeration::ReasonType(reasonType),
        details, Utils::makeOptionalString(labelerDid),
        [this, presence=getPresence()]{
            if (!presence)
                return;

            qDebug() << "Report author succeeded";
            emit reportOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Report author failed:" << error << " - " << msg;
            emit reportFailed(msg);
        });
}

void ReportUtils::reportPostOrFeed(const QString& uri, const QString& cid, QEnums::ReportReasonType reasonType,
                                   const QString& details, const QString& labelerDid)
{
    qDebug() << "Report post or feed:" << uri << cid << reasonType << details;
    
    bskyClient()->reportPostOrFeed(uri, cid, ATProto::ComATProtoModeration::ReasonType(reasonType),
        details, Utils::makeOptionalString(labelerDid),
        [this, presence=getPresence()]{
            if (!presence)
                return;

            qDebug() << "Report post or feed succeeded";
            emit reportOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Report post failed:" << error << " - " << msg;
            emit reportFailed(msg);
        });
}

void ReportUtils::reportDirectMessage(const QString& did, const QString& convoId, const QString& messageId,
                                      QEnums::ReportReasonType reasonType, const QString& details)
{
    qDebug() << "Report direct message:" << did << convoId << messageId << reasonType << details;

    bskyClient()->reportDirectMessage(did, convoId, messageId,
        ATProto::ComATProtoModeration::ReasonType(reasonType), details,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            qDebug() << "Report direct message succeeded";
            emit reportOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Report direct message failed:" << error << " - " << msg;
            emit reportFailed(msg);
        });
}

ReportCategory::List ReportUtils::getReportCategories()
{
    ReportCategory::List categories;

    for (int cat = QEnums::REPORT_CATEGORY_FIRST; cat <= QEnums::REPORT_CATEGORY_LAST; ++cat)
    {
        const QString title = ATProto::ComATProtoModeration::categoryTypeToTitle(ATProto::ComATProtoModeration::CategoryType(cat));
        const QString description = ATProto::ComATProtoModeration::categoryTypeToDescription(ATProto::ComATProtoModeration::CategoryType(cat));
        const ReportCategory category(QEnums::ReportCategoryType(cat), title, description);
        categories.push_back(category);
    }

    return categories;
}

ReportReason::List ReportUtils::getReportReasons(QEnums::ReportCategoryType category)
{
    const auto reasons = ATProto::ComATProtoModeration::getCategoryReasons(ATProto::ComATProtoModeration::CategoryType(category));
    ReportReason::List reasonList;
    reasonList.reserve(reasons.size());

    for (const auto r : reasons)
    {
        const ReportReason reason{ QEnums::ReportReasonType(r), ATProto::ComATProtoModeration::reasonTypeToTitle(r) };
        reasonList.push_back(reason);
    }

    return reasonList;
}

}
