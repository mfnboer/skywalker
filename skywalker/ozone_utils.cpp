// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_utils.h"
#include "link_utils.h"
#include "skywalker.h"
#include "ozone_report_list_model.h"
#include "ozone_subject_list_model.h"
#include "ozone_policy_view.h"
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
    return getLatestActionSummaryImpl(subject);
}

OzoneActionSummary OzoneUtils::getLatestActionSummary(const OzoneSubjectViewDetail& subject)
{
    return getLatestActionSummaryImpl(subject);
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

OzoneActionSummary OzoneUtils::getLatestActionSummaryImpl(const IOzoneSubjectView& subject)
{
    const OzoneActionView action = subject.getLatestAction();

    if (action.isNull())
        return {};

    const auto subjectType = subject.getSubjectType();

    if (subjectType != QEnums::OZONE_REPORT_SUBJECT_REPO_REF && subjectType != QEnums::OZONE_REPORT_SUBJECT_STRONG_REF)
        return {};

    const bool isAccount = subjectType == QEnums::OZONE_REPORT_SUBJECT_CONVO_REF;
    const QEnums::RefType refType = subjectVariantToRefType(subject.getSubject());
    const QString contentName = isAccount ? tr("account") : QEnums::refTypeToString(refType);

    // TODO: where defined?
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

}