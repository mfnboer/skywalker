// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_subject_view_detail.h"
#include "ozone_utils.h"
#include "strong_ref.h"

namespace Skywalker {

OzoneSubjectViewDetail::OzoneSubjectViewDetail(const ATProto::ToolsOzoneInbox::SubjectViewDetail::SharedPtr& subject) :
    mSubject(subject)
{
}

QString OzoneSubjectViewDetail::getSubjectUri() const
{
    if (!mSubject)
        return {};

    return OzoneUtils::getSubjectUri(mSubject->mSubject);
}

QEnums::OzoneReportSubjectType OzoneSubjectViewDetail::getSubjectType() const
{
    if (!mSubject)
        return {};

    if (ATProto::holdsNonNull<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mSubject->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_REPO_REF;

    if (ATProto::holdsNonNull<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mSubject->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_STRONG_REF;

    qWarning() << "Unknown subject type";
    return QEnums::OZONE_REPORT_SUBJECT_UNKNOWN;
}

RecordContent OzoneSubjectViewDetail::getRecordContent() const
{
    if (!mSubject || !mSubject->mRecord)
        return {};

    return OzoneUtils::getRecordContent(*mSubject->mRecord);
}

OzoneActionView OzoneSubjectViewDetail::getLatestAction() const
{
    const auto actions = getActions();

    if (actions.empty())
        return {};

    return actions.front();
}

bool OzoneSubjectViewDetail::canAppeal() const
{
    const auto actions = getAvailableActions();
    const auto it = std::find(actions.begin(), actions.end(), QEnums::OZONE_SUBJECT_ACTION_APPEAL);
    return it != actions.end();
}

}