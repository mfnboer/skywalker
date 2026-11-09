// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "profile.h"
#include "ozone_account_status.h"
#include "ozone_action_summary.h"
#include "ozone_report_view.h"
#include "ozone_resolution_view.h"
#include "ozone_subject_view.h"
#include "ozone_subject_view_detail.h"
#include "ozone_subject_view_interface.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class OzoneUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool appealInProgress READ getAppealInProgress NOTIFY appealInProgressChanged FINAL)
    QML_ELEMENT

public:
    explicit OzoneUtils(QObject* parent = nullptr);

    template<class SubjectType>
    static QString getSubjectUri(const SubjectType& subject);

    static RecordContent getRecordContent(const ATProto::ToolsOzoneInbox::ReportView::RecordType& record);

    Q_INVOKABLE static QString actionToDescription(const QString& action);
    Q_INVOKABLE static QString subjectTypeToString(const OzoneReportView& report, bool adverb = true);
    Q_INVOKABLE static QString subjectTypeToString(const OzoneSubjectViewDetail& subject, bool adverb = true);
    Q_INVOKABLE static QEnums::RefType subjectToRefType(const OzoneReportView& report);
    Q_INVOKABLE static QEnums::RefType subjectToRefType(const OzoneSubjectViewDetail& subject);
    Q_INVOKABLE OzoneActionSummary getLatestActionSummary(const OzoneSubjectView& subject);
    Q_INVOKABLE OzoneActionSummary getLatestActionSummary(const OzoneSubjectViewDetail& subject);
    Q_INVOKABLE OzoneActionSummary getActionSummary(const OzoneSubjectViewDetail& subject, int actionIndex);
    Q_INVOKABLE static QString getAppealStatus(const OzoneAppealView& appeal);

    Q_INVOKABLE void getReportList(int modelId, int limit = 25, int maxPages = 1, int minEntries = 0,  const QString& cursor = {});
    Q_INVOKABLE void getReportListNextPage(int modelId, int limit = 25, int maxPages = 1, int minEntries = 0);
    Q_INVOKABLE void getSubjectList(int modelId, int limit = 25, int maxPages = 1, int minEntries = 0,  const QString& cursor = {});
    Q_INVOKABLE void getSubjectListNextPage(int modelId, int limit = 25, int maxPages = 1, int minEntries = 0);
    Q_INVOKABLE void getReport(int reportId, const BasicProfile& profile);
    Q_INVOKABLE void getSubject(const QString& subject, const QString& cursor = {});
    Q_INVOKABLE void updateSeen(const QString& section, QDateTime seenAt);
    Q_INVOKABLE void appeal(const OzoneSubjectViewDetail& subject, const QString& reason);
    Q_INVOKABLE void getAccountStatus();

    bool getAppealInProgress() const { return mAppealInProgress; }

signals:
    // resolution can be null
    void reportOk(OzoneReportView report, OzoneResolutionView resolution, BasicProfile profile);
    void reportFailed(int reportId, QString error);
    void subjectOk(OzoneSubjectViewDetail subject);
    void subjectFailed(QString subject, QString error);
    void appealOk(OzoneSubjectView subject);
    void appealFailed(QString error);
    void appealInProgressChanged();
    void accountStatus(OzoneAccountStatus status);

private:
    OzoneActionSummary getActionSummaryImpl(const IOzoneSubjectView& subject, const OzoneActionView& action);
    OzoneActionSummary getEnforcementSummaryImpl(const IOzoneSubjectView& subject);
    static QEnums::RefType subjectVariantToRefType(const QVariant& subject);
    void setAppealInProgress(bool inProgress);

    std::unordered_map<QString, QDateTime> mSeenAt;
    bool mAppealInProgress = false;
};

template<class SubjectType>
QString OzoneUtils::getSubjectUri(const SubjectType& subject)
{
    if (ATProto::holdsNonNull<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(subject))
    {
        auto repoRef = std::get<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(subject);
        return repoRef->mDid;
    }

    if (ATProto::holdsNonNull<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(subject))
    {
        auto strongRef = std::get<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(subject);
        return strongRef->mUri;
    }

    return {};
}

}