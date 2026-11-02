// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_report_view.h"
#include "convo_ref.h"
#include "convo_message_ref.h"
#include "strong_ref.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

OzoneReportView::OzoneReportView(const ATProto::ToolsOzoneInbox::ReportView::SharedPtr& report) :
    mReport(report)
{
}

QEnums::OzoneReportSubjectType OzoneReportView::getSubjectType() const
{
    if (!mReport)
        return QEnums::OZONE_REPORT_SUBJECT_UNKNOWN;

    if (ATProto::holdsNonNull<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mReport->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_REPO_REF;

    if (ATProto::holdsNonNull<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mReport->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_STRONG_REF;

    if (ATProto::holdsNonNull<ATProto::ChatBskyConvo::ConvoRef::SharedPtr>(mReport->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_CONVO_REF;

    if (ATProto::holdsNonNull<ATProto::ChatBskyConvo::MessageRef::SharedPtr>(mReport->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF;

    qWarning() << "Unknown subject type";
    return QEnums::OZONE_REPORT_SUBJECT_UNKNOWN;
}

QVariant OzoneReportView::getSubject() const
{
    switch (getSubjectType())
    {
    case QEnums::OZONE_REPORT_SUBJECT_REPO_REF:
    {
        auto repoRef = std::get<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mReport->mSubject);
        return repoRef->mDid;
    }
    case QEnums::OZONE_REPORT_SUBJECT_STRONG_REF:
    {
        auto strongRef = std::get<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mReport->mSubject);
        return QVariant::fromValue(StrongRef(strongRef));
    }
    case QEnums::OZONE_REPORT_SUBJECT_CONVO_REF:
    {    auto convoRef = std::get<ATProto::ChatBskyConvo::ConvoRef::SharedPtr>(mReport->mSubject);
        return QVariant::fromValue(ConvoRef(convoRef));

    }
    case QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF:
    {
        auto messageRef = std::get<ATProto::ChatBskyConvo::MessageRef::SharedPtr>(mReport->mSubject);
        return QVariant::fromValue(ConvoMessageRef(messageRef));
    }
    case QEnums::OZONE_REPORT_SUBJECT_UNKNOWN:
        return {};
    }

    qWarning() << "Unknown subject type";
    return {};
}

QString OzoneReportView::getSubjectDid() const
{
    switch (getSubjectType())
    {
    case QEnums::OZONE_REPORT_SUBJECT_REPO_REF:
    {
        auto repoRef = std::get<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mReport->mSubject);
        return repoRef->mDid;
    }
    case QEnums::OZONE_REPORT_SUBJECT_STRONG_REF:
    {
        auto strongRef = std::get<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mReport->mSubject);
        ATProto::ATUri atUri(strongRef->mUri);
        return atUri.getAuthority();
    }
    case QEnums::OZONE_REPORT_SUBJECT_CONVO_REF:
    {    auto convoRef = std::get<ATProto::ChatBskyConvo::ConvoRef::SharedPtr>(mReport->mSubject);
        return convoRef->mDid;

    }
    case QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF:
    {
        auto messageRef = std::get<ATProto::ChatBskyConvo::MessageRef::SharedPtr>(mReport->mSubject);
        return messageRef->mDid;
    }
    case QEnums::OZONE_REPORT_SUBJECT_UNKNOWN:
        return {};
    }

    qWarning() << "Unknown subject type";
    return {};
}

QString OzoneReportView::getLastActionTaken() const
{
    if (!mReport)
        return {};

    if (!mReport->mLastActionTaken)
        return {};

    return actionToDescription(mReport->mLastActionTaken.value());
}

QString OzoneReportView::actionToDescription(const QString& action) const
{
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

}