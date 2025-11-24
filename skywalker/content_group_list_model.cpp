// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_group_list_model.h"
#include <atproto/lib/client.h>

namespace Skywalker {

ContentGroupListModel::ContentGroupListModel(QObject* parent) :
    QAbstractListModel(parent)
{
}

ContentGroupListModel::ContentGroupListModel(ContentFilter& contentFilter, const QString& listUri,
                                             QObject* parent) :
    QAbstractListModel(parent),
    mContentFilter(&contentFilter),
    mListUri(listUri),
    mSubscribed(true) // implicitly subscribed to global labels
{
    init();
}

ContentGroupListModel::ContentGroupListModel(const QString& labelerDid, ContentFilter& contentFilter,
                                             const QString& listUri, QObject* parent) :
    QAbstractListModel(parent),
    mContentFilter(&contentFilter),
    mListUri(listUri),
    mLabelerDid(labelerDid),
    mSubscribed(mContentFilter->isSubscribedToLabeler(mLabelerDid))
{
    init();
}

void ContentGroupListModel::init()
{
    mAdultContent = mContentFilter->getAdultContent();

    if (mSubscribed && !mLabelerDid.isEmpty())
        mNewLabelIds = mContentFilter->getNewLabelIds(mLabelerDid);

    if (mContentFilter->isFixedLabelerSubscription(mLabelerDid))
        mFixedLabelerEnabled = mContentFilter->isFixedLabelerEnabled(mLabelerDid);

    connect(mContentFilter, &ContentFilter::contentGroupsChanged, this,
        [this](const QString& listUri){
            // Adult content can only be changed on the default filter, but applies
            // to the list fitlers as well.
            mAdultContent = mContentFilter->getAdultContent();

            if (listUri == mListUri)
                mChangedVisibility.clear();

            emit dataChanged(createIndex(0, 0), createIndex(mContentGroupList.size() - 1, 0));
        });
}

void ContentGroupListModel::setGlobalContentGroups()
{
    clear();

    beginInsertRows({}, 0, ContentFilter::USER_CONTENT_GROUP_LIST.size() - 1);

    for (const auto& group : ContentFilter::USER_CONTENT_GROUP_LIST)
        mContentGroupList.push_back(group);

    endInsertRows();
}

void ContentGroupListModel::setContentGroups(std::vector<ContentGroup> groups)
{
    clear();

    beginInsertRows({}, 0, groups.size() - 1);
    mContentGroupList = std::move(groups);
    endInsertRows();
}

void ContentGroupListModel::clear()
{
    if (!mContentGroupList.empty())
    {
        beginRemoveRows({}, 0, mContentGroupList.size() - 1);
        mContentGroupList.clear();
        endRemoveRows();
    }
}

int ContentGroupListModel::rowCount(const QModelIndex&) const
{
    return mContentGroupList.size();
}

QVariant ContentGroupListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mContentGroupList.size())
        return {};

    const auto& group = mContentGroupList[index.row()];

    switch (Role(role))
    {
    case Role::ContentGroup:
        return QVariant::fromValue(group);
    case Role::ContentPrefVisibility:
    {
        auto it = mChangedVisibility.find(index.row());

        if (it != mChangedVisibility.end())
            return it->second;

        return mContentFilter->getGroupPrefVisibility(group, mListUri);
    }
    case Role::IsNewLabel:
        return mNewLabelIds.contains(group.getLabelId());
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

bool ContentGroupListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= (int)mContentGroupList.size())
        return {};

    const auto& group = mContentGroupList[index.row()];

    switch (Role(role))
    {
    case Role::ContentPrefVisibility:
    {
        const auto visibility = QEnums::ContentPrefVisibility(value.toInt());
        const auto origVisibility = mContentFilter->getGroupPrefVisibility(group, mListUri);

        if (visibility != origVisibility)
            mChangedVisibility[index.row()] = visibility;
        else
            mChangedVisibility.erase(index.row());

        return true;
    }
    default:
        Q_ASSERT(false);
        break;
    }

    return false;
}

QHash<int, QByteArray> ContentGroupListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::ContentGroup), "contentGroup" },
        { int(Role::ContentPrefVisibility), "contentPrefVisibility" },
        { int(Role::IsNewLabel), "isNewLabel" }
    };

    return roles;
}

void ContentGroupListModel::setAdultContent(bool adultContent)
{
    if (adultContent != mAdultContent)
    {
        mAdultContent = adultContent;
        emit adultContentChanged();
    }
}

void ContentGroupListModel::setSubscribed(bool subscribed)
{
    if (subscribed != mSubscribed)
    {
        if (subscribed && mContentFilter->numLabelers() >= ATProto::Client::MAX_LABELERS)
        {
            emit error(tr("Already subscribed to maximum number of labelers: %1").arg(mContentFilter->numLabelers()));
            return;
        }

        mSubscribed = subscribed;
        emit subscribedChanged();
    }
}

bool ContentGroupListModel::isFixedLabelerEnabled() const
{
    if (!isFixedSubscription())
        return false;

    return mFixedLabelerEnabled;
}

void ContentGroupListModel::setFixedLabelerEnabled(bool enabled)
{
    if (!isFixedSubscription())
        return;

    if (enabled != mFixedLabelerEnabled)
    {
        if (!enabled && !mAdultContent)
        {
            emit error(tr("You can only unsubscribe from this labeler when adult content is enabled."));
            return;
        }

        mFixedLabelerEnabled = enabled;
        emit fixedLabelerEnabledChanged();
    }
}

bool ContentGroupListModel::isFixedSubscription() const
{
    if (mLabelerDid.isEmpty())
        return true;

    return mContentFilter->isFixedLabelerSubscription(mLabelerDid);
}

bool ContentGroupListModel::isModified(const ATProto::UserPreferences& userPreferences) const
{
    if (!mListUri.isEmpty())
        return !mChangedVisibility.empty();

    return mAdultContent != userPreferences.getAdultContent() ||
           !mChangedVisibility.empty() ||
           (!mLabelerDid.isEmpty() && mSubscribed != mContentFilter->isSubscribedToLabeler(mLabelerDid)) ||
           (mContentFilter->isFixedLabelerSubscription(mLabelerDid) && mFixedLabelerEnabled != mContentFilter->isFixedLabelerEnabled(mLabelerDid));
}

void ContentGroupListModel::saveTo(ATProto::UserPreferences& userPreferences) const
{
    Q_ASSERT(mListUri.isEmpty());
    qDebug() << "Save preferences";
    userPreferences.setAdultContent(mAdultContent);

    for (const auto& [index, visibility] : mChangedVisibility)
    {
        const auto& contentGroup = mContentGroupList.at(index);
        const auto& labelerDid = contentGroup.getLabelerDid();
        const auto& label = contentGroup.getLabelId();
        const auto labelVisibility = ATProto::UserPreferences::LabelVisibility(visibility);

        qDebug() << "Changed label:" << label << "did:" << labelerDid << "visibitlity:" << (int)labelVisibility;

        Q_ASSERT(contentGroup.isGlobal() == ContentFilter::isGlobalLabel(label) || ContentFilter::isOverridableSytemLabelId(label));
        const auto defaultVisibility = QEnums::toContentPrefVisibility(contentGroup.getUnconditionalDefaultVisibility());

        if (visibility == defaultVisibility)
        {
            qDebug() << "Label:" << label << "did:" << labelerDid << "visibility is default:" << visibility;
            userPreferences.removeLabelVisibility(labelerDid, label);

            for (const auto& legacyId : contentGroup.getLegacyLabelIds())
                userPreferences.removeLabelVisibility(labelerDid, legacyId);
        }
        else
        {
            userPreferences.setLabelVisibility(labelerDid, label, labelVisibility);

            for (const auto& legacyId : contentGroup.getLegacyLabelIds())
                userPreferences.setLabelVisibility(labelerDid, legacyId, labelVisibility);
        }
    }

    if (mContentFilter->isFixedLabelerSubscription(mLabelerDid) && mFixedLabelerEnabled != mContentFilter->isFixedLabelerEnabled(mLabelerDid))
        mContentFilter->enableFixedLabeler(mLabelerDid, mFixedLabelerEnabled);

    if (mLabelerDid.isEmpty() || mSubscribed == mContentFilter->isSubscribedToLabeler(mLabelerDid))
        return;

    auto prefs = userPreferences.getLabelersPref();
    ATProto::AppBskyActor::LabelerPrefItem item;
    item.mDid = mLabelerDid;

    if (mSubscribed)
    {
        qDebug() << "Subscribe to labeler:" << mLabelerDid;
        prefs.mLabelers.insert(item);
        mContentFilter->addContentGroups(mLabelerDid, mContentGroupList);
    }
    else
    {
        qDebug() << "Unsubscribe from labeler:" << mLabelerDid;
        prefs.mLabelers.erase(item);
        userPreferences.removeContentLabelPrefs(mLabelerDid);
        mContentFilter->removeContentGroups(mLabelerDid);
    }

    userPreferences.setLabelersPref(prefs);
}

void ContentGroupListModel::saveToContentFilter()
{
    Q_ASSERT(!mListUri.isEmpty());
    qDebug() << "Save preferences for list:" << mListUri;

    for (const auto& [index, visibility] : mChangedVisibility)
    {
        const auto& contentGroup = mContentGroupList.at(index);
        const auto& labelerDid = contentGroup.getLabelerDid();
        const auto& label = contentGroup.getLabelId();

        qDebug() << "Changed label:" << label << "did:" << labelerDid << "visibitlity:" << visibility;

        Q_ASSERT(contentGroup.isGlobal() == ContentFilter::isGlobalLabel(label) || ContentFilter::isOverridableSytemLabelId(label));
        mContentFilter->setListPref(mListUri, labelerDid, label, visibility);
    }
}

}
