// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_filter.h"
#include "user_settings.h"

namespace Skywalker {

// We are implicitly subscribed to the Bluesky moderator
#define BLUESKY_MODERATOR_DID QStringLiteral("did:plc:ar7c4by46qjdydhdevvrndac")

const std::vector<ContentGroup> ContentFilter::SYSTEM_CONTENT_GROUP_LIST = {
    {
        "!hide",
        QObject::tr("Content Blocked"),
        QObject::tr("This content has been hidden by the moderators."),
        {},
        false,
        QEnums::CONTENT_VISIBILITY_HIDE_POST,
        QEnums::LABEL_TARGET_CONTENT,
        QEnums::LABEL_SEVERITY_ALERT,
        ""
    },
    {
        "!warn",
        QObject::tr("Content Warning"),
        QObject::tr("This content has received a general warning from the moderators."),
        {},
        false,
        QEnums::CONTENT_VISIBILITY_WARN_POST,
        QEnums::LABEL_TARGET_CONTENT,
        QEnums::LABEL_SEVERITY_ALERT,
        ""
    }
};

const std::vector<ContentGroup> ContentFilter::USER_CONTENT_GROUP_LIST = {
    {
        "porn",
        QObject::tr("Adult Content"),
        QObject::tr("Explicit sexual images"),
        {"nsfw"},
        true,
        QEnums::CONTENT_VISIBILITY_WARN_MEDIA,
        QEnums::LABEL_TARGET_MEDIA,
        QEnums::LABEL_SEVERITY_ALERT,
        ""
    },
    {
        "sexual",
        QObject::tr("Sexually Suggestive"),
        QObject::tr("Does not include nudity"),
        {"suggestive"},
        true,
        QEnums::CONTENT_VISIBILITY_WARN_MEDIA,
        QEnums::LABEL_TARGET_MEDIA,
        QEnums::LABEL_SEVERITY_NONE,
        ""
    },
    {
        "nudity",
        QObject::tr("Non-sexual Nudity"),
        QObject::tr("E.g. artistic nudes"),
        {},
        true,
        QEnums::CONTENT_VISIBILITY_WARN_MEDIA,
        QEnums::LABEL_TARGET_MEDIA,
        QEnums::LABEL_SEVERITY_NONE,
        ""
    },
    {
        "graphic-media",
        QObject::tr("Graphic Media"),
        QObject::tr("Explicit or potentially disturbing media"),
        {"gore", "corpse"},
        true,
        QEnums::CONTENT_VISIBILITY_HIDE_MEDIA,
        QEnums::LABEL_TARGET_MEDIA,
        QEnums::LABEL_SEVERITY_ALERT,
        ""
    }
};

ContentFilter::GlobalContentGroupMap ContentFilter::CONTENT_GROUPS;

void ContentFilter::initContentGroups()
{
    for (const auto& group : SYSTEM_CONTENT_GROUP_LIST)
    {
        CONTENT_GROUPS[group.getLabelId()] = &group;

        for (const auto& legacyId : group.getLegacyLabelIds())
            CONTENT_GROUPS[legacyId] = &group;
    }

    for (const auto& group : USER_CONTENT_GROUP_LIST)
    {
        CONTENT_GROUPS[group.getLabelId()] = &group;

        for (const auto& legacyId : group.getLegacyLabelIds())
            CONTENT_GROUPS[legacyId] = &group;
    }
}

const ContentFilter::GlobalContentGroupMap& ContentFilter::getGlobalContentGroups()
{
    if (CONTENT_GROUPS.empty())
        initContentGroups();

    return CONTENT_GROUPS;
}

const ContentGroup* ContentFilter::getGlobalContentGroup(const QString& labelId)
{
    const auto& groups = getGlobalContentGroups();
    auto it = groups.find(labelId);
    return it != groups.end() ? it->second : nullptr;
}

bool ContentFilter::isGlobalLabel(const QString& labelId)
{
    return getGlobalContentGroup(labelId) != nullptr;
}

ContentFilter::ContentFilter(const ATProto::UserPreferences& userPreferences, UserSettings* userSettings, QObject* parent) :
    QObject(parent),
    mUserPreferences(userPreferences),
    mUserSettings(userSettings)
{
}

const ContentGroup* ContentFilter::getContentGroup(const QString& did, const QString& labelId) const
{
    auto* group = getGlobalContentGroup(labelId);

    if (group)
        return group;

    auto itDid = mLabelerGroupMap.find(did);

    if (itDid == mLabelerGroupMap.end())
        return nullptr;

    const ContentGroupMap& groupMap = itDid->second;
    auto itLabel = groupMap.find(labelId);

    if (itLabel == groupMap.end())
        return nullptr;

    return &itLabel->second;
}

ContentLabelList ContentFilter::getContentLabels(const LabelList& labels)
{
    ContentLabelList contentLabels;
    addContentLabels(contentLabels, labels);
    return contentLabels;
}

void ContentFilter::addContentLabels(ContentLabelList& contentLabels, const LabelList& labels)
{
    for (const auto& label : labels)
    {
        const ContentLabel contentLabel(label->mSrc, label->mUri, label->mCid.value_or(""),
                                        label->mVal, label->mCreatedAt);

        if (!label->mNeg)
        {
            contentLabels.append(contentLabel);
        }
        else
        {
            contentLabels.removeIf([&contentLabel](const ContentLabel& l){
                return l.getLabelId() == contentLabel.getLabelId() && l.getDid() == contentLabel.getDid();
            });
        }
    }
}

QEnums::ContentPrefVisibility ContentFilter::getGroupPrefVisibility(const ContentGroup& group) const
{
    if (group.isAdult() && !getAdultContent())
        return QEnums::CONTENT_PREF_VISIBILITY_HIDE;

    auto visibility = mUserPreferences.getLabelVisibility(group.getLabelerDid(), group.getLabelId());

    if (visibility == ATProto::UserPreferences::LabelVisibility::UNKNOWN)
    {
        for (const auto& legacyId : group.getLegacyLabelIds())
        {
            visibility = mUserPreferences.getLabelVisibility(group.getLabelerDid(), legacyId);

            if (visibility != ATProto::UserPreferences::LabelVisibility::UNKNOWN)
                break;
        }
    }

    if (visibility != ATProto::UserPreferences::LabelVisibility::UNKNOWN)
        return QEnums::ContentPrefVisibility(visibility);

    return QEnums::toContentPrefVisibility(group.getUnconditionalDefaultVisibility());
}

QEnums::ContentVisibility ContentFilter::getGroupVisibility(const ContentGroup& group) const
{
    if (group.isAdult() && !getAdultContent())
        return QEnums::CONTENT_VISIBILITY_HIDE_MEDIA;

    auto visibility = mUserPreferences.getLabelVisibility(group.getLabelerDid(), group.getLabelId());

    if (visibility == ATProto::UserPreferences::LabelVisibility::UNKNOWN)
    {
        for (const auto& legacyId : group.getLegacyLabelIds())
        {
            visibility = mUserPreferences.getLabelVisibility(group.getLabelerDid(), legacyId);

            if (visibility != ATProto::UserPreferences::LabelVisibility::UNKNOWN)
                break;
        }
    }

    if (visibility != ATProto::UserPreferences::LabelVisibility::UNKNOWN)
        return group.getContentVisibility(visibility);

    return group.getDefaultVisibility();
}

QEnums::ContentVisibility ContentFilter::getVisibility(const ContentLabel& label) const
{
    const auto* group = getContentGroup(label.getDid(), label.getLabelId());

    if (group)
        return getGroupVisibility(*group);

    qDebug() << "Undefined label:" << label.getLabelId() << "labeler:" << label.getDid();
    return QEnums::CONTENT_VISIBILITY_SHOW;
}

bool ContentFilter::mustShowBadge(const ContentLabel& label) const
{
    const auto* group = getContentGroup(label.getDid(), label.getLabelId());

    if (!group || !group->isBadge())
        return true;

    auto visibility = mUserPreferences.getLabelVisibility(group->getLabelerDid(), group->getLabelId());
    return group->mustShowBadge(visibility);
}

QString ContentFilter::getGroupWarning(const ContentGroup& group) const
{
    if (group.isAdult() && !getAdultContent())
        return QObject::tr("Adult content");

    return group.getTitleWithSeverity();
}

QString ContentFilter::getWarning(const ContentLabel& label) const
{
    const auto* group = getContentGroup(label.getDid(), label.getLabelId());

    if (group)
        return getGroupWarning(*group);

    qDebug() << "Undefined label:" << label.getLabelId() << "labeler:" << label.getDid();
    return QObject::tr("Unknown label") + QString(": %1").arg(label.getLabelId());
}

std::tuple<QEnums::ContentVisibility, QString> ContentFilter::getVisibilityAndWarning(const ATProto::ComATProtoLabel::LabelList& labels) const
{
    const auto contentLabels = getContentLabels(labels);
    return getVisibilityAndWarning(contentLabels);
}

std::tuple<QEnums::ContentVisibility, QString> ContentFilter::getVisibilityAndWarning(const ContentLabelList& contentLabels) const
{
    QEnums::ContentVisibility visibility = QEnums::CONTENT_VISIBILITY_SHOW;
    QString warning;

    for (const auto& label : contentLabels)
    {
        const auto v = getVisibility(label);

        if (v <= visibility)
            continue;

        visibility = v;
        warning = getWarning(label);
    }

    return {visibility, warning};
}

bool ContentFilter::isSubscribedToLabeler(const QString& did) const
{
    if (isFixedLabelerSubscription(did))
        return true;

    ATProto::AppBskyActor::LabelerPrefItem item;
    item.mDid = did;
    const auto& prefs = mUserPreferences.getLabelersPref();
    return prefs.mLabelers.contains(item);
}

std::unordered_set<QString> ContentFilter::getSubscribedLabelerDids() const
{
    auto dids = mUserPreferences.getLabelerDids();
    dids.insert(BLUESKY_MODERATOR_DID);
    return dids;
}

std::vector<QString> ContentFilter::getSubscribedLabelerDidsOrdered() const
{
    std::vector<QString> dids;
    dids.push_back(BLUESKY_MODERATOR_DID);

    auto subscribedDids = mUserPreferences.getLabelerDids();

    for (const auto& did : subscribedDids)
        dids.push_back(did);

    return dids;
}

size_t ContentFilter::numLabelers() const
{
    return mUserPreferences.numLabelers() + 1; // +1 for Bluesky labeler
}

QStringList ContentFilter::getLabelIds(const QString& labelerDid) const
{
    if (!mLabelerGroupMap.contains(labelerDid))
        return {};

    QStringList labelIds;
    const ContentGroupMap& contentGroups = mLabelerGroupMap.at(labelerDid);

    for (const auto& [labelId, _] : contentGroups)
        labelIds.push_back(labelId);

    return labelIds;
}

bool ContentFilter::isFixedLabelerSubscription(const QString& did)
{
    return did == BLUESKY_MODERATOR_DID;
}

void ContentFilter::addContentGroupMap(const QString& did, const ContentGroupMap& contentGroupMap)
{
    Q_ASSERT(!did.isEmpty());
    qDebug() << "Add content group map for did:" << did;
    mLabelerGroupMap[did] = contentGroupMap;
}

void ContentFilter::addContentGroups(const QString& did, const std::vector<ContentGroup>& contentGroups)
{
    Q_ASSERT(!did.isEmpty());
    qDebug() << "Add content groups for did:" << did;
    auto& groupMap = mLabelerGroupMap[did];

    for (const auto& group : contentGroups)
        groupMap[group.getLabelId()] = group;

    saveLabelIdsToSettings(did);
}

void ContentFilter::removeContentGroups(const QString& did)
{
    Q_ASSERT(!did.isEmpty());
    mLabelerGroupMap.erase(did);
    removeLabelIdsFromSettings(did);
}

void ContentFilter::saveLabelIdsToSettings(const QString& labelerDid)
{
    qDebug() << "Save label ids:" << labelerDid;
    const QString did = mUserSettings->getActiveUserDid();
    Q_ASSERT(!did.isEmpty());

    if (did.isEmpty())
    {
        qWarning() << "No DID";
        return;
    }

    const auto labels = getLabelIds(labelerDid);
    mUserSettings->addLabels(did, labelerDid, labels);
}

void ContentFilter::removeLabelIdsFromSettings(const QString &labelerDid)
{
    qDebug() << "Remove label ids:" << labelerDid;
    const QString did = mUserSettings->getActiveUserDid();
    Q_ASSERT(!did.isEmpty());

    if (did.isEmpty())
    {
        qWarning() << "No DID";
        return;
    }

    mUserSettings->removeLabels(did, labelerDid);
}

std::unordered_set<QString> ContentFilter::checkNewLabelIds(const QString& labelerDid) const
{
    const QString did = mUserSettings->getActiveUserDid();
    Q_ASSERT(!did.isEmpty());

    if (did.isEmpty())
    {
        qWarning() << "No DID";
        return {};
    }

    if (!mUserSettings->containsLabeler(did, labelerDid))
    {
        qDebug() << "No labels saved for:" << labelerDid;
        return {};
    }

    std::unordered_set<QString> newLabels;
    const QStringList savedLabels = mUserSettings->getLabels(did, labelerDid); // TODO store as set?
    std::unordered_set<QString> savedLabelSet(savedLabels.begin(), savedLabels.end());
    const QStringList labels = getLabelIds(labelerDid);

    for (const auto& label : labels)
    {
        if (!savedLabelSet.contains(label))
            newLabels.insert(label);
    }

    return newLabels;
}

bool ContentFilter::hasNewLabels(const QString& labelerDid) const
{
    const auto newLabels = checkNewLabelIds(labelerDid);
    return !newLabels.empty();
}

}
