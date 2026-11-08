// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_subject_view_interface.h"
#include "strong_ref.h"

namespace Skywalker {

QVariant IOzoneSubjectView::getSubject() const
{
    switch (getSubjectType())
    {
    case QEnums::OZONE_REPORT_SUBJECT_REPO_REF:
    {
        const auto subject = getAtprotoSubject();
        auto repoRef = std::get<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(subject);
        return repoRef->mDid;
    }
    case QEnums::OZONE_REPORT_SUBJECT_STRONG_REF:
    {
        const auto subject = getAtprotoSubject();
        auto strongRef = std::get<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(subject);
        return QVariant::fromValue(StrongRef(strongRef));
    }
    case QEnums::OZONE_REPORT_SUBJECT_CONVO_REF:
    case QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF:
    case QEnums::OZONE_REPORT_SUBJECT_UNKNOWN:
        return {};
    }

    qWarning() << "Unknown subject type";
    return {};
}

QList<QEnums::OzoneSubjectAction> IOzoneSubjectView::getAvailableActions() const
{
    const auto availableActions = getAtprotoActions();

    QList<QEnums::OzoneSubjectAction> actions;
    actions.reserve(availableActions.size());

    for (const auto& action : availableActions)
        actions.push_back((QEnums::OzoneSubjectAction)action.value());

    return actions;
}

}