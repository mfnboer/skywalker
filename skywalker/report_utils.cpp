// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "report_utils.h"
#include "utils.h"

namespace Skywalker {

ReportUtils::ReportUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
    mReportReasons.append(ReportReason(QEnums::REPORT_REASON_TYPE_SPAM));
    mReportReasons.append(ReportReason(QEnums::REPORT_REASON_TYPE_VIOLATION));
    mReportReasons.append(ReportReason(QEnums::REPORT_REASON_TYPE_MISLEADING));
    mReportReasons.append(ReportReason(QEnums::REPORT_REASON_TYPE_SEXUAL));
    mReportReasons.append(ReportReason(QEnums::REPORT_REASON_TYPE_RUDE));
    mReportReasons.append(ReportReason(QEnums::REPORT_REASON_TYPE_OTHER));
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

}
