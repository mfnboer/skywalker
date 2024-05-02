// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_group_list_model.h"

namespace Skywalker {

ContentGroupListModel::ContentGroupListModel(const ContentFilter& contentFilter, QObject* parent) :
    QAbstractListModel(parent),
    mContentFilter(contentFilter),
    mSubscribed(true) // implicitly subscribed to global labels
{
}

ContentGroupListModel::ContentGroupListModel(const QString& labelerDid, const ContentFilter& contentFilter, QObject* parent) :
    QAbstractListModel(parent),
    mContentFilter(contentFilter),
    mLabelerDid(labelerDid),
    mSubscribed(mContentFilter.isSubscribedToLabeler(mLabelerDid))
{
}

void ContentGroupListModel::setGlobalContentGroups()
{
    clear();

    beginInsertRows({}, 0, ContentFilter::CONTENT_GROUP_LIST.size() - 1);

    for (const auto& group : ContentFilter::CONTENT_GROUP_LIST)
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

        return QEnums::toContentPrefVisibility(mContentFilter.getGroupVisibility(group));
    }
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
        const auto origVisibility = QEnums::toContentPrefVisibility(mContentFilter.getGroupVisibility(group));

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
        { int(Role::ContentPrefVisibility), "contentPrefVisibility" }
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
        mSubscribed = subscribed;
        emit subscribedChanged();
    }
}

bool ContentGroupListModel::isFixedSubscription() const
{
    if (mLabelerDid.isEmpty())
        return true;

    return mContentFilter.isFixedLabelerSubscription(mLabelerDid);
}

bool ContentGroupListModel::isModified(const ATProto::UserPreferences& userPreferences) const
{
    return mAdultContent != userPreferences.getAdultContent() ||
           !mChangedVisibility.empty() ||
           (!mLabelerDid.isEmpty() && mSubscribed != mContentFilter.isSubscribedToLabeler(mLabelerDid));
}

void ContentGroupListModel::saveTo(ATProto::UserPreferences& userPreferences) const
{
    userPreferences.setAdultContent(mAdultContent);

    for (const auto& [index, visibility] : mChangedVisibility)
    {
        const auto& contentGroup = mContentGroupList.at(index);
        const auto& label = contentGroup.getLabelId();
        const auto labelVisibility = ATProto::UserPreferences::LabelVisibility(visibility);

        qDebug() << "Changed label:" << label << "visibitlity:" << (int)labelVisibility;

        Q_ASSERT(contentGroup.isGlobal() == ContentFilter::isGlobalLabel(label));
        userPreferences.setLabelVisibility(contentGroup.getLabelerDid(), label, labelVisibility);

        if (contentGroup.getLegacyLabelId())
            userPreferences.setLabelVisibility(contentGroup.getLabelerDid(), *contentGroup.getLegacyLabelId(), labelVisibility);
    }

    if (mLabelerDid.isEmpty() || mSubscribed == mContentFilter.isSubscribedToLabeler(mLabelerDid))
        return;

    auto prefs = userPreferences.getLabelersPref();

    if (mSubscribed)
    {
        qDebug() << "Subscribe to labeler:" << mLabelerDid;
        ATProto::AppBskyActor::LabelerPrefItem item;
        item.mDid = mLabelerDid;
        prefs.mLabelers.push_back(item);
    }
    else
    {
        qDebug() << "Unsubscribe from labeler:" << mLabelerDid;
        std::erase_if(prefs.mLabelers, [this](const auto& item){ return item.mDid == mLabelerDid; });
        userPreferences.removeContentLabelPrefs(mLabelerDid);
    }

    userPreferences.setLabelersPref(prefs);
}

}
