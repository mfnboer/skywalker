// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "report_utils.h"
#include "utils.h"

namespace Skywalker {

const ReportReasonList ReportUtils::ACCOUNT_REASONS = {
    {
        QEnums::REPORT_REASON_TYPE_MISLEADING,
        tr("Misleading Account"),
        tr("Impersonation or false claims about identity or affiliation")
    },
    {
        QEnums::REPORT_REASON_TYPE_SPAM,
        tr("Frequently Posts Unwanted Content"),
        tr("Spam; excessive mentions or replies")
    },
    {
        QEnums::REPORT_REASON_TYPE_VIOLATION,
        tr("Illegal and Urgent"),
        tr("Glaring violations of law, community standards or terms of service")
    },
    {
        QEnums::REPORT_REASON_TYPE_OTHER,
        tr("Other"),
        tr("An issue not included in these options")
    }
};

const ReportReasonList ReportUtils::POST_REASONS = {
    {
        QEnums::REPORT_REASON_TYPE_SPAM,
        tr("Spam"),
        tr("Excessive mentions or replies")
    },
    {
        QEnums::REPORT_REASON_TYPE_SEXUAL,
        tr("Unwanted Sexual Content"),
        tr("Nudity or adult content not labeled as such")
    },
    {
        QEnums::REPORT_REASON_TYPE_RUDE,
        tr("Anti-Social Behavior"),
        tr("Harassment, trolling, or intolerance")
    },
    {
        QEnums::REPORT_REASON_TYPE_VIOLATION,
        tr("Illegal and Urgent"),
        tr("Glaring violations of law, community standards or terms of service")
    },
    {
        QEnums::REPORT_REASON_TYPE_OTHER,
        tr("Other"),
        tr("An issue not included in these options")
    }
};

const ReportReasonList ReportUtils::FEED_REASONS = {
    {
        QEnums::REPORT_REASON_TYPE_RUDE,
        tr("Anti-Social Behavior"),
        tr("Harassment, trolling, or intolerance")
    },
    {
        QEnums::REPORT_REASON_TYPE_VIOLATION,
        tr("Illegal and Urgent"),
        tr("Glaring violations of law, community standards or terms of service")
    },
    {
        QEnums::REPORT_REASON_TYPE_OTHER,
        tr("Other"),
        tr("An issue not included in these options")
    }
};

const ReportReasonList ReportUtils::LIST_REASONS = {
    {
        QEnums::REPORT_REASON_TYPE_RUDE,
        tr("Anti-Social Behavior"),
        tr("Harassment, trolling, or intolerance")
    },
    {
        QEnums::REPORT_REASON_TYPE_VIOLATION,
        tr("Illegal and Urgent"),
        tr("Glaring violations of law, community standards or terms of service")
    },
    {
        QEnums::REPORT_REASON_TYPE_OTHER,
        tr("Other"),
        tr("An issue not included in these options")
    }
};

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

ReportReasonList ReportUtils::getReportReasons(QEnums::ReportTarget target)
{
    switch (target)
    {
    case QEnums::ReportTarget::REPORT_TARGET_ACCOUNT:
        return ACCOUNT_REASONS;
    case QEnums::ReportTarget::REPORT_TARGET_POST:
        return POST_REASONS;
    case QEnums::ReportTarget::REPORT_TARGET_FEED:
        return FEED_REASONS;
    case QEnums::ReportTarget::REPORT_TARGET_LIST:
        return LIST_REASONS;
    }

    Q_ASSERT(false);
    return POST_REASONS;
}

}
