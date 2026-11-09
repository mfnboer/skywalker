// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_utils.h"
#include "link_utils.h"
#include "skywalker.h"
#include "ozone_report_list_model.h"
#include "ozone_subject_list_model.h"
#include "ozone_policy_view.h"
#include "report_utils.h"
#include "utils.h"

namespace Skywalker {

OzoneUtils::OzoneUtils(QObject* parent) :
    WrappedSkywalker(parent)
{}

RecordContent OzoneUtils::getRecordContent(const ATProto::ToolsOzoneInbox::ReportView::RecordType& record)
{
    if (ATProto::holdsNonNull<ATProto::AppBskyFeed::Record::Post::SharedPtr>(record))
    {
        auto post = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(record);
        return RecordContent{post->mText, post->mCreatedAt};
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyGraph::List::SharedPtr>(record))
    {
        auto list = std::get<ATProto::AppBskyGraph::List::SharedPtr>(record);
        return RecordContent{list->mName, list->mCreatedAt};
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyFeed::Generator::SharedPtr>(record))
    {
        auto feed = std::get<ATProto::AppBskyFeed::Generator::SharedPtr>(record);
        return RecordContent{feed->mDisplayName, feed->mCreatedAt};
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyGraph::StarterPack::SharedPtr>(record))
    {
        auto starterPack = std::get<ATProto::AppBskyGraph::StarterPack::SharedPtr>(record);
        return RecordContent{starterPack->mName, starterPack->mCreatedAt};
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyActor::Status::SharedPtr>(record))
    {
        auto status = std::get<ATProto::AppBskyActor::Status::SharedPtr>(record);

        if (!status->mEmbed || !ATProto::holdsNonNull<ATProto::AppBskyEmbed::External::SharedPtr>(*status->mEmbed))
            return {};

        auto external = std::get<ATProto::AppBskyEmbed::External::SharedPtr>(*status->mEmbed);
        return RecordContent{external->mExternal->mTitle + "\n" + external->mExternal->mUri, status->mCreatedAt};
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyActor::Link::SharedPtr>(record))
    {
        auto link = std::get<ATProto::AppBskyActor::Link::SharedPtr>(record);

        if (link->mTitle)
            return RecordContent{*link->mTitle + "\n" + link->mUrl, link->mCreatedAt};

        return RecordContent{link->mUrl, link->mCreatedAt};
    }

    if (ATProto::holdsNonNull<ATProto::UnknownVariant::SharedPtr>(record))
    {
        auto unknown = std::get<ATProto::UnknownVariant::SharedPtr>(record);
        return RecordContent{unknown->mType, {}};
    }

    return {};
}

QString OzoneUtils::actionToDescription(const QString& action)
{
    if (action.isEmpty())
        return {};

    switch(QEnums::labelToOzoneAction(action))
    {
    case QEnums::OZONE_ACTION_CONTENT_REMOVED:
        return QObject::tr("Content removed");
    case QEnums::OZONE_ACTION_CONTENT_RESTORED:
        return QObject::tr("Content restored");
    case QEnums::OZONE_ACTION_ACCOUNT_SUSPENDED:
        return QObject::tr("Account suspended");
    case QEnums::OZONE_ACTION_ACCOUNT_TAKEDOWN:
        return QObject::tr("Account taken down");
    case QEnums::OZONE_ACTION_ACCOUNT_RESTORED:
        return QObject::tr("Account restored");
    case QEnums::OZONE_ACTION_LABEL_APPLIED:
        return QObject::tr("Label applied");
    case QEnums::OZONE_ACTION_LABEL_REMOVED:
        return QObject::tr("Label removed");
    case QEnums::OZONE_ACTION_COMMUNICATION_SENT:
        return QObject::tr("Communication sent");
    case QEnums::OZONE_ACTION_REPORTING_RESTRICTED:
        return QObject::tr("Reporting restricted");
    case QEnums::OZONE_ACTION_CREDENTIALS_REVOKED:
        return QObject::tr("Credentials revoked");
    case QEnums::OZONE_ACTION_UNKNOWN:
        break;
    }

    qWarning() << "Uknown action:" << action;
    return action;
}

template<class Entity>
static QString subjectToStringImpl(const Entity& entity, bool adverb)
{
    const QVariant subject = entity.getSubject();

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

template<class Entity>
static QString subjectTypeToStringImpl(const Entity& entity, bool adverb)
{
    switch (entity.getSubjectType())
    {
    case QEnums::OZONE_REPORT_SUBJECT_REPO_REF:
        return adverb ? QObject::tr("Account") : QObject::tr("account");
    case QEnums::OZONE_REPORT_SUBJECT_STRONG_REF:
        return subjectToStringImpl(entity, adverb);
    case QEnums::OZONE_REPORT_SUBJECT_CONVO_REF:
        return adverb ? QObject::tr("Chat with") : QObject::tr("chat");
    case QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF:
        return adverb ? QObject::tr("Chat message from") : QObject::tr("chat message");
    case QEnums::OZONE_REPORT_SUBJECT_UNKNOWN:
        break;
    }

    return QObject::tr("Content");
}

QString OzoneUtils::subjectTypeToString(const OzoneReportView& report, bool adverb)
{
    return subjectTypeToStringImpl(report, adverb);
}

QString OzoneUtils::subjectTypeToString(const OzoneSubjectViewDetail& subject, bool adverb)
{
    return subjectTypeToStringImpl(subject, adverb);
}


QEnums::RefType OzoneUtils::subjectVariantToRefType(const QVariant& subject)
{
    if (!subject.canConvert<StrongRef>())
        return QEnums::REF_TYPE_UNKNOWN;

    const auto& strongRef = subject.value<StrongRef>();
    const auto refType = LinkUtils::getUriRefType(strongRef.getUri());
    return refType;
}

QEnums::RefType OzoneUtils::subjectToRefType(const OzoneReportView& report)
{
    return subjectVariantToRefType(report.getSubject());
}

QEnums::RefType OzoneUtils::subjectToRefType(const OzoneSubjectViewDetail& subject)
{
    return subjectVariantToRefType(subject.getSubject());
}

OzoneActionSummary OzoneUtils::getLatestActionSummary(const OzoneSubjectView& subject)
{
    return getActionSummaryImpl(subject, subject.getLatestAction());
}

OzoneActionSummary OzoneUtils::getLatestActionSummary(const OzoneSubjectViewDetail& subject)
{
    return getActionSummaryImpl(subject, subject.getLatestAction());
}

OzoneActionSummary OzoneUtils::getActionSummary(const OzoneSubjectViewDetail& subject, int actionIndex)
{
    const auto actions = subject.getActions();

    if (actions.size() <= actionIndex)
        return {};

    return getActionSummaryImpl(subject, actions.at(actionIndex));
}

static OzonePolicySummary toPolicySummary(const OzonePolicyView& policyView, const QString& templateText)
{
    const QString text = templateText.arg(policyView.getDisplayName());
    return OzonePolicySummary{text, policyView.getLink()};
}

static OzonePolicySummary::List toPolicySummaryList(const OzonePolicyView::List& policyViewList, const QString& templateText = QObject::tr("Community guidelines: %1"))
{
    OzonePolicySummary::List summaryList;
    summaryList.reserve(policyViewList.size());

    for (const auto& policy : policyViewList)
        summaryList.push_back(toPolicySummary(policy, templateText));

    return summaryList;
}

OzoneActionSummary OzoneUtils::getActionSummaryImpl(const IOzoneSubjectView& subject, const OzoneActionView& action)
{
    const auto subjectType = subject.getSubjectType();

    if (subjectType != QEnums::OZONE_REPORT_SUBJECT_REPO_REF && subjectType != QEnums::OZONE_REPORT_SUBJECT_STRONG_REF)
        return {};

    if (action.isNull())
        return getEnforcementSummaryImpl(subject);

    const bool isAccount = subjectType == QEnums::OZONE_REPORT_SUBJECT_CONVO_REF;
    const QEnums::RefType refType = subjectVariantToRefType(subject.getSubject());
    const QString contentName = isAccount ? tr("account") : QEnums::refTypeToString(refType);
    const auto actionType = QEnums::labelToOzoneAction(action.getType());

    switch (actionType)
    {
    case QEnums::OZONE_ACTION_CONTENT_REMOVED:
        return OzoneActionSummary{
            tr("Your %1 was removed").arg(contentName),
            toPolicySummaryList(action.getPolicies())
        };
    case QEnums::OZONE_ACTION_CONTENT_RESTORED:
        return OzoneActionSummary{
            tr("Your %1 was restored").arg(contentName)
        };
    case QEnums::OZONE_ACTION_ACCOUNT_SUSPENDED:
    {
        OzonePolicySummary::List policies;

        if (action.getExpiresAt().isValid())
        {
            const auto duration = (action.getExpiresAt() - action.getCreatedAt()) / 1h;

            if (action.getExpiresAt() <= action.getCreatedAt())
                policies = toPolicySummaryList(action.getPolicies(), tr("%1 - %2h, now expired").arg("%1", duration));
            else
                policies = toPolicySummaryList(action.getPolicies(), tr("%1 - %2h").arg("%1", duration));
        }
        else
        {
            policies = toPolicySummaryList(action.getPolicies());
        }

        return OzoneActionSummary{
            tr("Your account was suspended"),
            policies
        };
    }
    case QEnums::OZONE_ACTION_ACCOUNT_TAKEDOWN:
        return OzoneActionSummary{
            tr("Your account was suspended"),
            toPolicySummaryList(action.getPolicies())
        };
    case QEnums::OZONE_ACTION_ACCOUNT_RESTORED:
        return OzoneActionSummary{
            tr("Your account was restored")
        };
    case QEnums::OZONE_ACTION_LABEL_APPLIED:
    case QEnums::OZONE_ACTION_LABEL_REMOVED:
    {
        auto labels = action.getLabels();
        auto expiresAt = action.getExpiresAt();

        if (labels.empty())
        {
            const auto& enforcement = subject.getEnforcment();
            labels = enforcement.getLabels();
            expiresAt = enforcement.getExpiresAt();
        }

        ContentLabel::List contentLabels;

        for (const auto& label : labels)
            contentLabels.push_back(ContentLabel(subject.getSource(), {}, {}, label, action.getCreatedAt(), expiresAt));

        return OzoneActionSummary{
            (actionType == QEnums::OZONE_ACTION_LABEL_APPLIED ?
                tr("A label was added to your %1").arg(contentName) :
                tr("A label was removed from your %1").arg(contentName)),
            toPolicySummaryList(action.getPolicies()),
            contentLabels
        };
    }
    case QEnums::OZONE_ACTION_COMMUNICATION_SENT:
        return OzoneActionSummary{
            tr("We sent you an email about your account"),
            toPolicySummaryList(action.getPolicies())
        };
    case QEnums::OZONE_ACTION_REPORTING_RESTRICTED:
        return OzoneActionSummary{
            tr("Your ability to submit reports was restricted"),
            toPolicySummaryList(action.getPolicies())
        };
    case QEnums::OZONE_ACTION_CREDENTIALS_REVOKED:
        return OzoneActionSummary{
            tr("Your account credentials were revoked"),
            toPolicySummaryList(action.getPolicies())
        };
    case QEnums::OZONE_ACTION_UNKNOWN:
        return OzoneActionSummary{
            action.getType(),
            toPolicySummaryList(action.getPolicies())
        };
    }

    qWarning() << "Uknown action:" << action.getType();
    Q_ASSERT(false);
    return OzoneActionSummary{
        "UKNOWN"
    };
}

OzoneActionSummary OzoneUtils::getEnforcementSummaryImpl(const IOzoneSubjectView& subject)
{
    const auto subjectType = subject.getSubjectType();

    if (subjectType != QEnums::OZONE_REPORT_SUBJECT_REPO_REF && subjectType != QEnums::OZONE_REPORT_SUBJECT_STRONG_REF)
        return {};

    const bool isAccount = subjectType == QEnums::OZONE_REPORT_SUBJECT_CONVO_REF;
    const QEnums::RefType refType = subjectVariantToRefType(subject.getSubject());
    const QString contentName = isAccount ? tr("account") : QEnums::refTypeToString(refType);
    const OzoneEnforcementView enforcement = subject.getEnforcment();

    switch (enforcement.getState())
    {
    case QEnums::OZONE_ENFORCEMENT_STATE_LABELED:
    {
        const auto labels = enforcement.getLabels();
        const auto expiresAt = enforcement.getExpiresAt();

        ContentLabel::List contentLabels;

        for (const auto& label : labels)
            contentLabels.push_back(ContentLabel(subject.getSource(), {}, {}, label, subject.getCreatedAt(), expiresAt));

        return OzoneActionSummary{
            tr("A label was added to your %1").arg(contentName), {}, contentLabels
        };
    }
    case QEnums::OZONE_ENFORCEMENT_STATE_SUSPENDED:
    case QEnums::OZONE_ENFORCEMENT_STATE_TAKENDOWN:
        return OzoneActionSummary{ tr("Your account was suspended") };
    default:
        break;
    }

    return {};
}

QString OzoneUtils::getAppealStatus(const OzoneAppealView& appeal)
{
    if (appeal.isNull())
        return {};

    const QDateTime resolvedAt = appeal.getResolvedAt();
    const QString resolvedAtText = resolvedAt.isValid() ? resolvedAt.toString(" dd-MM-yyyy") : "";

    switch (appeal.getOutcome())
    {
    case QEnums::OZONE_APPEAL_OUTCOME_APPROVED:
        return QObject::tr("Appeal approved%1").arg(resolvedAtText);
    case QEnums::OZONE_APPEAL_OUTCOME_DENIED:
        return QObject::tr("Appeal denied%1").arg(resolvedAtText);
    case QEnums::OZONE_APPEAL_OUTCOME_UNKNOWN:
        break;
    }

    switch (appeal.getState())
    {
    case QEnums::OZONE_APPEAL_STATE_PENDING:
    {
        const QDateTime appealedAt = appeal.getAppealedAt();
        const QString appealedAtText = appealedAt.isValid() ? appealedAt.toString(" dd-MM-yyyy") : "";
        return QObject::tr("Appeal pending%1").arg(appealedAtText);
    }
    case QEnums::OZONE_APPEAL_STATE_RESOLVED:
        qWarning() << "Appeal was resolved but outcome was unknown";
        return QObject::tr("Appeal resolved%1").arg(resolvedAtText);
    case QEnums::OZONE_APPEAL_STATE_SUPERSEDED:
        return QObject::tr("Appeal superseded%1").arg(resolvedAtText); // What does this mean?
    case QEnums::OZONE_APPEAL_STATE_EXPIRED:
    {
        const QDateTime appealableUntil = appeal.getAppealableUntil();
        const QString appealableUntilText = appealableUntil.isValid() ? appealableUntil.toString(" dd-MM-yyyy") : "";
        return QObject::tr("Possibility to appeal expired%1").arg(appealableUntilText);
    }
    case QEnums::OZONE_APPEAL_STATE_NONE:
    case QEnums::OZONE_APPEAL_STATE_UNKNOWN:
        break;
    }

    return {};
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

            if (toAdd > 0)
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

void OzoneUtils::getSubjectList(int modelId, int limit, int maxPages, int minEntries,  const QString& cursor)
{
    qDebug() << "Get subject list:" << modelId << "cursor:" << cursor;

    if (!bskyClient())
        return;

    auto* model = mSkywalker->getOzoneSubjectListModel(modelId);
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

    bskyClient()->ozoneInboxListActionedSubjects(model->getReportsFilter(), {}, {}, {}, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), modelId, limit, maxPages, minEntries, cursor](ATProto::ToolsOzoneInbox::ListActionedSubjectsOutput::SharedPtr output){
            if (!presence)
                return;

            auto* model = mSkywalker->getOzoneSubjectListModel(modelId);


            if (!model)
                return; // user has closed the view

            model->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                model->clear();

            const int added = model->addSubjects(std::move(output->mSubjects), output->mCursor.value_or(""));
            const int toAdd = minEntries - added;

            if (toAdd > 0)
                getReportListNextPage(modelId, limit, maxPages - 1, toAdd);
        },
        [this, presence=getPresence(), modelId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getSubjectList failed:" << error << " - " << msg;
            auto* model = mSkywalker->getOzoneSubjectListModel(modelId);

            if (model)
                model->setGetFeedInProgress(false);

            mSkywalker->statusMessage(mSkywalker->getUserDid(), msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void OzoneUtils::getSubjectListNextPage(int modelId, int limit, int maxPages, int minEntries)
{
    qDebug() << "Get subject list next page:" << modelId;

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    auto* model = mSkywalker->getOzoneSubjectListModel(modelId);
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

    getSubjectList(modelId, limit, maxPages, minEntries, cursor);
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

void OzoneUtils::getSubject(const QString& subject, const QString& cursor)
{
    qDebug() << "Get subject:" << subject << "cursor:" << cursor;

    if (!bskyClient())
        return;

    bskyClient()->ozoneInboxGetActionedSubject(subject, {}, Utils::makeOptionalString(cursor),
        [this, presence=getPresence()](ATProto::ToolsOzoneInbox::SubjectViewDetail::SharedPtr detail){
            if (!presence)
                return;

            emit subjectOk(OzoneSubjectViewDetail{detail});
        },
        [this, presence=getPresence(), subject](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getSubject failed:" << error << " - " << msg;
            emit subjectFailed(subject, msg);
        });
}

void OzoneUtils::updateSeen(const QString& section, QDateTime seenAt)
{
    qDebug() << "Update seen:" << section << seenAt;

    const auto lastSeen = mSeenAt[section];

    if (lastSeen.isValid() && seenAt <= lastSeen)
    {
        qDebug() << "Already updated:" << lastSeen;
        return;
    }

    if (!bskyClient())
        return;

    bskyClient()->ozoneInboxUpdateSeen({section}, seenAt,
        [this, presence=getPresence(), section, seenAt]{
            if (!presence)
                return;

            mSeenAt[section] = seenAt;
        },
        {});
}

void OzoneUtils::appeal(const OzoneSubjectViewDetail& subject, const QString& reason)
{
    qDebug() << "Appeal:" << subject.getSubjectUri() << "reason:" << reason;

    if (mAppealInProgress)
    {
        qDebug() << "Appeal still in progress";
        return;
    }

    if (!bskyClient())
        return;

    auto atprotoSubject = subject.getAtprotoPtr();
    ATProto::ToolsOzoneInbox::AppealSubjectType subjectParam;

    if (ATProto::holdsNonNull<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(atprotoSubject->mSubject))
    {
        subjectParam = *std::get<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(atprotoSubject->mSubject);
    }
    else if (ATProto::holdsNonNull<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(atprotoSubject->mSubject))
    {
        subjectParam = *std::get<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(atprotoSubject->mSubject);
    }
    else
    {
        qWarning() << "Invalid subject:" << subject.getSubjectUri();
        return;
    }

    ATProto::ToolsOzoneInbox::AppealActionType actionParam;

    const OzoneActionView action = subject.getLatestAction();

    if (!action.isNull())
    {
        ATProto::ToolsOzoneInbox::AppealActionRef actionRef;
        actionRef.mId = action.getId();
        actionParam = actionRef;
    }
    else
    {
        const OzoneEnforcementView enforcement = subject.getEnforcment();

        switch (enforcement.getState())
        {
        case QEnums::OZONE_ENFORCEMENT_STATE_LABELED:
        {
            const auto labels = enforcement.getLabels();

            if (!labels.empty())
            {
                ATProto::ToolsOzoneInbox::AppealLabelRef labelRef;
                labelRef.mVal = labels.front();
                actionParam = labelRef;
            }
            else
            {
                qWarning() << "Label missing:" << subject.getSubjectUri();
                return;
            }

            break;
        }
        case QEnums::OZONE_ENFORCEMENT_STATE_TAKENDOWN:
        case QEnums::OZONE_ENFORCEMENT_STATE_SUSPENDED:
        {
            ATProto::ToolsOzoneInbox::AppealTakedownRef takedownRef;
            actionParam = takedownRef;
            break;
        }
        default:
            qWarning() << "No state to appeal:" << enforcement.getState() << "subjectUri:" << subject.getSubjectUri();
            return;
        }
    }

    setAppealInProgress(true);

    bskyClient()->ozoneInboxAppealActionedSubject(
        subjectParam, actionParam, Utils::makeOptionalString(reason), ReportUtils::getModToolName(),
        [this, presence=getPresence()](ATProto::ToolsOzoneInbox::SubjectView::SharedPtr subjectView){
            if (!presence)
                return;

            setAppealInProgress(false);
            emit appealOk(OzoneSubjectView{subjectView});
        },
        [this, presence=getPresence(), subject](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "appealLatestAction failed:" << error << " - " << msg;
            setAppealInProgress(false);
            emit appealFailed(msg);
        });
}

void OzoneUtils::getAccountStatus()
{
    if (!bskyClient())
        return;

    bskyClient()->ozoneInboxGetAccountStatus(
        [this, presence=getPresence()](ATProto::ToolsOzoneInbox::GetAccountStatusOutput::SharedPtr output){
            if (!presence)
                return;

            const OzoneAccountStatus status(output);
            emit accountStatus(status);
        },
        {});
}

void OzoneUtils::setAppealInProgress(bool inProgress)
{
    if (mAppealInProgress != inProgress)
    {
        mAppealInProgress = inProgress;
        emit appealInProgressChanged();
    }
}

}