// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_subject_view.h"
#include "ozone_utils.h"
#include "strong_ref.h"

namespace Skywalker {

OzoneSubjectView::OzoneSubjectView(const ATProto::ToolsOzoneInbox::SubjectView::SharedPtr& subject) :
    mSubject(subject)
{
}

QString OzoneSubjectView::getSubjectUri() const
{
    if (!mSubject)
        return {};

    return OzoneUtils::getSubjectUri(mSubject->mSubject);
}

QEnums::OzoneReportSubjectType OzoneSubjectView::getSubjectType() const
{
    if (!mSubject)
        return QEnums::OZONE_REPORT_SUBJECT_UNKNOWN;

    if (ATProto::holdsNonNull<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mSubject->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_REPO_REF;

    if (ATProto::holdsNonNull<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mSubject->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_STRONG_REF;

    qWarning() << "Unknown subject type";
    return QEnums::OZONE_REPORT_SUBJECT_UNKNOWN;
}

}