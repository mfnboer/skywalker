// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_filter.h"
#include "list_store.h"
#include "profile_store.h"
#include "user_settings.h"

namespace Skywalker {

// We are implicitly subscribed to the Bluesky moderator
#define BLUESKY_MODERATOR_DID QStringLiteral("did:plc:ar7c4by46qjdydhdevvrndac")

const std::vector<ContentGroup> ContentFilter::SYSTEM_CONTENT_GROUP_LIST = {
    {
        "!hide",
        QObject::tr("Moderator Blocked"),
        QObject::tr("This content has been blocked by the moderator."),
        {},
        false,
        QEnums::CONTENT_VISIBILITY_HIDE_POST,
        QEnums::LABEL_TARGET_CONTENT,
        QEnums::LABEL_SEVERITY_ALERT,
        ""
    },
    {
        "!warn",
        QObject::tr("Moderator Warning"),
        QObject::tr("This content has received a general warning from the moderator."),
        {},
        false,
        QEnums::CONTENT_VISIBILITY_WARN_POST,
        QEnums::LABEL_TARGET_CONTENT,
        QEnums::LABEL_SEVERITY_ALERT,
        ""
    },
    {
        "!no-unauthenticated",
        QObject::tr("Logged out visibility"),
        QObject::tr("Only show content to logged in users."),
        {},
        false,
        QEnums::CONTENT_VISIBILITY_SHOW,
        QEnums::LABEL_TARGET_CONTENT,
        QEnums::LABEL_SEVERITY_NONE,
        ""
    }
};

const std::unordered_set<QString> ContentFilter::OVERRIDABLE_SYSTEM_LABELS_IDS = {
    "!hide",
    "!warn"
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
        "graphic-media",
        QObject::tr("Graphic Media"),
        QObject::tr("Explicit or potentially disturbing media"),
        {"gore", "corpse"},
        true,
        QEnums::CONTENT_VISIBILITY_HIDE_MEDIA,
        QEnums::LABEL_TARGET_MEDIA,
        QEnums::LABEL_SEVERITY_ALERT,
        ""
    },
    {
        "nudity",
        QObject::tr("Non-sexual Nudity"),
        QObject::tr("E.g. artistic nudes"),
        {},
        false,
        QEnums::CONTENT_VISIBILITY_SHOW,
        QEnums::LABEL_TARGET_MEDIA,
        QEnums::LABEL_SEVERITY_NONE,
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

void ContentFilter::removeListPrefs(const QString& listUri)
{
    qDebug() << "Remove list prefs:" << listUri;
    mListPrefs.erase(listUri);
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

bool ContentFilter::isSystemLabelId(const QString& labelId)
{
    return labelId.startsWith('!');
}

bool ContentFilter::isOverridableSytemLabelId(const QString& labelId)
{
    return OVERRIDABLE_SYSTEM_LABELS_IDS.contains(labelId);
}

ContentFilter::ContentFilter(const QString& userDid,
                             const IProfileStore& following,
                             const ListStore& policies,
                             const ATProto::UserPreferences& userPreferences,
                             UserSettings* userSettings, QObject* parent) :
    QObject(parent),
    mUserDid(userDid),
    mFollowing(following),
    mListsWithPolicies(policies),
    mUserPreferences(userPreferences),
    mUserSettings(userSettings)
{
    connect(&mListsWithPolicies, &ListStore::listRemoved, this,
            [this](const QString& uri){ removeListPrefs(uri); });
}

void ContentFilter::clear()
{
    mLabelerGroupMap.clear();
    clearFollowingPrefs();
    mListPrefs.clear();
}

void ContentFilter::initListPrefs()
{
    qDebug() << "Initialize list prefs:" << mUserDid;
    const auto keys = mUserSettings->getContentLabelPrefKeys(mUserDid);

    for (const auto& [listUri, labelerDid, labelId] : keys)
    {
        const auto pref = mUserSettings->getContentLabelPref(mUserDid, listUri, labelerDid, labelId);
        qDebug() << "list:" << listUri << "labeler:" << labelerDid << "label:" << labelId << "pref:" << pref;

        if (listUri == "following")
            setFollowingPref(labelerDid, labelId, pref);
        else
            mListPrefs[listUri][labelerDid][labelId] = (ATProto::UserPreferences::LabelVisibility)pref;
    }
}

const ContentGroup* ContentFilter::getContentGroup(const QString& did, const QString& labelId) const
{
    const auto* globalGroup = getGlobalContentGroup(labelId);

    if (globalGroup && !isOverridableSytemLabelId(labelId))
        return globalGroup;

    auto itDid = mLabelerGroupMap.find(did);

    if (itDid == mLabelerGroupMap.end())
        return globalGroup;

    const ContentGroupMap& groupMap = itDid->second;
    auto itLabel = groupMap.find(labelId);

    if (itLabel == groupMap.end())
        return globalGroup;

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

QEnums::ContentPrefVisibility ContentFilter::getGroupPrefVisibility(const ContentGroup& group, const QString& listUri) const
{
    if (group.isAdult() && !getAdultContent())
        return QEnums::CONTENT_PREF_VISIBILITY_HIDE;

    ATProto::UserPreferences::LabelVisibility visibility = ATProto::UserPreferences::LabelVisibility::UNKNOWN;

    if (listUri == "following")
    {
        visibility = getLabelVisibility(mFollowingPrefs, group);

        if (visibility == ATProto::UserPreferences::LabelVisibility::UNKNOWN)
            visibility = getVisibilityDefaultPrefs(group);
    }
    else if (!listUri.isEmpty() && mListPrefs.contains(listUri))
    {
        visibility = getLabelVisibility(mListPrefs.at(listUri), group);

        if (visibility == ATProto::UserPreferences::LabelVisibility::UNKNOWN)
            visibility = getVisibilityDefaultPrefs(group);
    }
    else
    {
        visibility = getVisibilityDefaultPrefs(group);
    }


    if (visibility != ATProto::UserPreferences::LabelVisibility::UNKNOWN)
        return QEnums::ContentPrefVisibility(visibility);

    return QEnums::toContentPrefVisibility(group.getUnconditionalDefaultVisibility());
}

QEnums::ContentVisibility ContentFilter::getGroupVisibility(
    const QString& authorDid,
    const ContentGroup& group,
    std::optional<QEnums::ContentVisibility> adultOverrideVisibility) const
{
    if (group.isAdult())
    {
        if (!getAdultContent())
            return QEnums::CONTENT_VISIBILITY_HIDE_MEDIA;

        if (adultOverrideVisibility)
            return *adultOverrideVisibility;
    }

    auto visibility = getVisibilityAuthorPrefs(authorDid, group);
    qDebug() << "Author prefs:" << authorDid << "label:" << group.getTitle() << "visibility:" << (int)visibility;

    if (visibility == ATProto::UserPreferences::LabelVisibility::UNKNOWN)
        visibility = getVisibilityDefaultPrefs(group);

    if (visibility != ATProto::UserPreferences::LabelVisibility::UNKNOWN)
        return group.getContentVisibility(visibility);

    return group.getDefaultVisibility();
}

ATProto::UserPreferences::LabelVisibility ContentFilter::getVisibilityDefaultPrefs(
    const ContentGroup& group) const
{
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

    return visibility;
}

ATProto::UserPreferences::LabelVisibility ContentFilter::getVisibilityAuthorPrefs(
    const QString& authorDid, const ContentGroup& group) const
{
    auto visibility = ATProto::UserPreferences::LabelVisibility::UNKNOWN;

    if (authorDid.isEmpty())
        return visibility;

    if (!mFollowingPrefs.empty() && mFollowing.contains(authorDid))
        visibility = getLabelVisibility(mFollowingPrefs, group);

    for (const auto& [listUri, prefs] : mListPrefs)
    {
        if (visibility == ATProto::UserPreferences::LabelVisibility::SHOW)
            break;

        if (mListsWithPolicies.contains(listUri, authorDid))
        {
            const auto v = getLabelVisibility(prefs, group);

            if (v < visibility)
                visibility = v;
        }
    }

    return visibility;
}

ATProto::UserPreferences::LabelVisibility ContentFilter::getLabelVisibility(
    const ATProto::UserPreferences::ContentLabelPrefs& labelPrefs, const ContentGroup& group) const
{
    auto itDid = labelPrefs.find(group.getLabelerDid());

    if (itDid == labelPrefs.end())
        return ATProto::UserPreferences::LabelVisibility::UNKNOWN;

    const auto& labelVisibilityMap = itDid->second;
    auto itLabel = labelVisibilityMap.find(group.getLabelId());

    return itLabel != labelVisibilityMap.end() ?
               itLabel->second :
               ATProto::UserPreferences::LabelVisibility::UNKNOWN;
}

QEnums::ContentVisibility ContentFilter::getVisibility(
    const QString& authorDid,
    const ContentLabel& label,
    std::optional<QEnums::ContentVisibility> adultOverrideVisibility) const
{
    const auto* group = getContentGroup(label.getDid(), label.getLabelId());

    if (group)
        return getGroupVisibility(authorDid, *group, adultOverrideVisibility);

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

std::tuple<QEnums::ContentVisibility, QString> ContentFilter::getVisibilityAndWarning(
    const QString& authorDid,
    const ATProto::ComATProtoLabel::Label::List& labels,
    std::optional<QEnums::ContentVisibility> adultOverrideVisibility) const
{
    const auto contentLabels = getContentLabels(labels);
    const auto [visibility, warning, _] = getVisibilityAndWarning(authorDid, contentLabels, adultOverrideVisibility);
    return { visibility, warning };
}

std::tuple<QEnums::ContentVisibility, QString, int> ContentFilter::getVisibilityAndWarning(
    const QString& authorDid,
    const ContentLabelList& contentLabels,
    std::optional<QEnums::ContentVisibility> adultOverrideVisibility) const
{
    QEnums::ContentVisibility visibility = QEnums::CONTENT_VISIBILITY_SHOW;
    QString warning;
    int labelIndex = -1;

    for (int i = 0; i < contentLabels.size(); ++i)
    {
        const auto& label = contentLabels[i];
        const auto v = getVisibility(authorDid, label, adultOverrideVisibility);

        if (v <= visibility)
            continue;

        visibility = v;
        warning = getWarning(label);
        labelIndex = i;

        if (visibility == QEnums::CONTENT_VISIBILITY_LAST)
            break;
    }

    return {visibility, warning, labelIndex};
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

bool ContentFilter::isFixedLabelerEnabled(const QString& did) const
{
    if (!isFixedLabelerSubscription(did))
    {
        qWarning() << "Not a fixed labeler:" << did;
        return false;
    }

    return mUserSettings->getFixedLabelerEnabled(mUserDid, did);
}

void ContentFilter::enableFixedLabeler(const QString& did, bool enabled)
{
    if (!isFixedLabelerSubscription(did))
    {
        qWarning() << "Not a fixed labeler:" << did;
        return;
    }

    mUserSettings->setFixedLabelerEnabled(mUserDid, did, enabled);
}

std::unordered_set<QString> ContentFilter::getSubscribedLabelerDids(bool includeDisabledFixedLabelers) const
{
    auto dids = mUserPreferences.getLabelerDids();

    if (includeDisabledFixedLabelers || mUserSettings->getFixedLabelerEnabled(mUserDid, BLUESKY_MODERATOR_DID))
        dids.insert(BLUESKY_MODERATOR_DID);

    return dids;
}

std::vector<QString> ContentFilter::getSubscribedLabelerDidsOrdered() const
{
    std::vector<QString> dids;

    if (mUserSettings->getFixedLabelerEnabled(mUserDid, BLUESKY_MODERATOR_DID))
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

    for (const auto& [labelId, group] : contentGroups)
    {
        if (!group.isSystem())
            labelIds.push_back(labelId);
    }

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

void ContentFilter::saveLabelIdsToSettings(const QString& labelerDid) const
{
    qDebug() << "Save label ids:" << labelerDid;
    Q_ASSERT(!mUserDid.isEmpty());

    if (mUserDid.isEmpty())
    {
        qWarning() << "No DID";
        return;
    }

    const auto labels = getLabelIds(labelerDid);
    mUserSettings->setLabels(mUserDid, labelerDid, labels);
}

void ContentFilter::removeLabelIdsFromSettings(const QString &labelerDid) const
{
    qDebug() << "Remove label ids:" << labelerDid;
    Q_ASSERT(!mUserDid.isEmpty());

    if (mUserDid.isEmpty())
    {
        qWarning() << "No DID";
        return;
    }

    mUserSettings->removeLabels(mUserDid, labelerDid);
}

std::unordered_set<QString> ContentFilter::getNewLabelIds(const QString& labelerDid) const
{
    Q_ASSERT(!mUserDid.isEmpty());

    if (mUserDid.isEmpty())
    {
        qWarning() << "No DID";
        return {};
    }

    if (!mLabelerGroupMap.contains(labelerDid))
    {
        qWarning() << "Unknown labeler:" << labelerDid;
        return {};
    }

    if (!mUserSettings->containsLabeler(mUserDid, labelerDid))
    {
        qDebug() << "No labels saved for:" << labelerDid;
        saveLabelIdsToSettings(labelerDid);
        return {};
    }

    std::unordered_set<QString> newLabels;
    const QStringList savedLabels = mUserSettings->getLabels(mUserDid, labelerDid);
    const std::unordered_set<QString> savedLabelSet(savedLabels.begin(), savedLabels.end());
    const ContentGroupMap& labelMap = mLabelerGroupMap.at(labelerDid);

    for (const auto& [label, group] : labelMap)
    {
        if (group.isSystem())
            continue;

        if (!savedLabelSet.contains(label))
            newLabels.insert(label);
    }

    return newLabels;
}

bool ContentFilter::hasNewLabels(const QString& labelerDid) const
{
    const auto newLabels = getNewLabelIds(labelerDid);
    return !newLabels.empty();
}

std::unordered_set<QString> ContentFilter::getLabelerDidsWithNewLabels() const
{
    std::unordered_set<QString> labelers;

    for (const auto& [labelerDid, _] : mLabelerGroupMap)
    {
        if (hasNewLabels(labelerDid))
            labelers.insert(labelerDid);
    }

    return labelers;
}

bool ContentFilter::hasFollowingPrefs() const
{
    return !mFollowingPrefs.empty();
}

void ContentFilter::clearFollowingPrefs()
{
    if (!mFollowingPrefs.empty())
    {
        mFollowingPrefs.clear();
        emit hasFollowingPrefsChanged();
    }
}

void ContentFilter::createFollowingPrefs()
{
    if (!mFollowingPrefs.empty())
    {
        qWarning() << "Following prefs already created";
        return;
    }

    // Insert empty prefs for the global labels
    mFollowingPrefs[""] = {};
    emit hasFollowingPrefsChanged();
}

void ContentFilter::setFollowingPref(const QString& labelerDid, const QString& labelId, QEnums::ContentPrefVisibility pref)
{
    const bool wasEmpty = mFollowingPrefs.empty();
    mFollowingPrefs[labelerDid][labelId] = (ATProto::UserPreferences::LabelVisibility)pref;
    mUserSettings->setContentLabelPref(mUserDid, "following", labelerDid, labelId, pref);

    if (wasEmpty)
        emit hasFollowingPrefsChanged();
}

void ContentFilter::removeFollowingPref(const QString& labelerDid, const QString& labelId)
{
    mFollowingPrefs[labelerDid].erase(labelId);
    mUserSettings->removeContentLabelPref(mUserDid, "following", labelerDid, labelId);
}

}
