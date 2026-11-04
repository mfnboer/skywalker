// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_utils.h"
#include "link_utils.h"
#include "skywalker.h"
#include "ozone_report_list_model.h"
#include "utils.h"

namespace Skywalker {

OzoneUtils::OzoneUtils(QObject* parent) :
    WrappedSkywalker(parent)
{}

QString OzoneUtils::actionToDescription(const QString& action)
{
    if (action.isEmpty())
        return {};

    if (action == "contentRemoved")
        return QObject::tr("Content removed");
    if (action == "contentRestored")
        return QObject::tr("Content restored");
    if (action == "accountSuspended")
        return QObject::tr("Account suspended");
    if (action == "accountTakedown")
        return QObject::tr("Account taken down");
    if (action == "accountRestored")
        return QObject::tr("Account restored");
    if (action == "labelApplied")
        return QObject::tr("Label applied");
    if (action == "labelRemoved")
        return QObject::tr("Label removed");
    if (action == "communicationSent")
        return QObject::tr("Communication sent");
    if (action == "reportingRestricted")
        return QObject::tr("Reporting restricted");
    if (action == "credentialsRevoked")
        return QObject::tr("Credentials revoked");

    qWarning() << "Uknown action:" << action;
    return action;
}

QString OzoneUtils::subjectTypeToString(const OzoneReportView& report, bool adverb)
{
    switch (report.getSubjectType())
    {
    case QEnums::OZONE_REPORT_SUBJECT_REPO_REF:
        return adverb ? QObject::tr("Account") : QObject::tr("account");
    case QEnums::OZONE_REPORT_SUBJECT_STRONG_REF:
        return subjectToString(report, adverb);
    case QEnums::OZONE_REPORT_SUBJECT_CONVO_REF:
        return adverb ? QObject::tr("Chat with") : QObject::tr("chat");
    case QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF:
        return adverb ? QObject::tr("Chat message from") : QObject::tr("chat message");
    case QEnums::OZONE_REPORT_SUBJECT_UNKNOWN:
        break;
    }

    return QObject::tr("Content");
}

QString OzoneUtils::subjectToString(const OzoneReportView& report, bool adverb)
{
    const QVariant subject = report.getSubject();

    if (!subject.canConvert<StrongRef>())
        return QObject::tr("Content");

    const auto& strongRef = subject.value<StrongRef>();
    const auto refType = LinkUtils::getUriRefType(strongRef.getUri());

    switch (refType) {
    case QEnums::REF_TYPE_POST:
        return adverb ? QObject::tr("Post by") : QObject::tr("post");
    case QEnums::REF_TYPE_LIST:
        return adverb ? QObject::tr("List by") : QObject::tr("list");
    case QEnums::REF_TYPE_FEED:
        return adverb ? QObject::tr("Feed by") : QObject::tr("feed");
    case QEnums::REF_TYPE_STARTER_PACK:
        return adverb ? QObject::tr("Starter pack by") : QObject::tr("starter pack");
    case QEnums::REF_TYPE_ACTOR_STATUS:
        return adverb ? QObject::tr("Livestream by") : QObject::tr("livestream");
    case QEnums::REF_TYPE_ACTOR_LINK:
        return adverb ? QObject::tr("Profile link from") : QObject::tr("profile link");
    case QEnums::REF_TYPE_UNKNOWN:
        break;
    }

    return strongRef.getUri();
}

QEnums::RefType OzoneUtils::subjectToRefType(const OzoneReportView& report)
{
    const QVariant subject = report.getSubject();

    if (!subject.canConvert<StrongRef>())
        return QEnums::REF_TYPE_UNKNOWN;

    const auto& strongRef = subject.value<StrongRef>();
    const auto refType = LinkUtils::getUriRefType(strongRef.getUri());
    return refType;
}

void OzoneUtils::getReportList(int modelId, int limit, int maxPages, int minEntries,  const QString& cursor)
{
    qDebug() << "Get report list:" << modelId << "cursor:" << cursor;

    if (!bskyClient())
        return;

    auto* model = mSkywalker->getOzoneReportListModel(modelId);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get author starter pack list still in progress";
        return;
    }

    model->setGetFeedInProgress(true);

    bskyClient()->ozoneInboxListReports(model->getReportsFilter(), {}, {}, {}, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), modelId, limit, maxPages, minEntries, cursor](ATProto::ToolsOzoneInbox::ListReportsOutput::SharedPtr output){
            if (!presence)
                return;

            auto* model = mSkywalker->getOzoneReportListModel(modelId);


            if (!model)
                return; // user has closed the view

            model->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                model->clear();

            const int added = model->addReports(std::move(output->mReports), output->mCursor.value_or(""));
            const int toAdd = minEntries - added;

            if (toAdd < 0)
                getReportListNextPage(modelId, limit, maxPages - 1, toAdd);
        },
        [this, presence=getPresence(), modelId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getReportList failed:" << error << " - " << msg;
            auto* model = mSkywalker->getOzoneReportListModel(modelId);

            if (model)
                model->setGetFeedInProgress(false);

            mSkywalker->statusMessage(mSkywalker->getUserDid(), msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void OzoneUtils::getReportListNextPage(int modelId, int limit, int maxPages, int minEntries)
{
    qDebug() << "Get report list next page:" << modelId;

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    auto* model = mSkywalker->getOzoneReportListModel(modelId);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    const auto& cursor = model->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    getReportList(modelId, limit, maxPages, minEntries, cursor);
}

void OzoneUtils::getReport(int reportId, const BasicProfile& profile)
{
    qDebug() << "Get report:" << reportId;

    if (!bskyClient())
        return;

    bskyClient()->ozoneInboxGetReport(reportId,
        [this, presence=getPresence(), profile](ATProto::ToolsOzoneInbox::GetReportOutput::SharedPtr output){
            if (!presence)
                return;

            emit reportOk(OzoneReportView(output->mReport),
                          OzoneResolutionView(output->mResolution),
                          profile);
        },
        [this, presence=getPresence(), reportId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getReport failed:" << error << " - " << msg;
            emit reportFailed(reportId, msg);
        });
}

}