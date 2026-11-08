// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_report_view.h"
#include "convo_ref.h"
#include "convo_message_ref.h"
#include "ozone_utils.h"
#include "strong_ref.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

OzoneReportView::OzoneReportView(const ATProto::ToolsOzoneInbox::ReportView::SharedPtr& report) :
    mReport(report)
{
}

QString OzoneReportView::getSubjectUri() const
{
    if (!mReport)
        return {};

    return OzoneUtils::getSubjectUri(mReport->mSubject);
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
    {
        auto convoRef = std::get<ATProto::ChatBskyConvo::ConvoRef::SharedPtr>(mReport->mSubject);
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

RecordContent OzoneReportView::getRecordContent() const
{
    if (!mReport || !mReport->mRecord)
        return {};

    return OzoneUtils::getRecordContent(*mReport->mRecord);
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

}