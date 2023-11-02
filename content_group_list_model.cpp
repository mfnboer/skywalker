// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_group_list_model.h"

namespace Skywalker {

ContentGroupListModel::ContentGroupListModel(const ContentFilter& contentFilter, QObject* parent) :
    QAbstractListModel(parent),
    mContentFilter(contentFilter)
{
    initContentGroups();
}

void ContentGroupListModel::initContentGroups()
{
    clear();

    beginInsertRows({}, 0, ContentFilter::CONTENT_GROUP_LIST.size() - 1);

    for (const auto& group : ContentFilter::CONTENT_GROUP_LIST)
        mContentGroupList.push_back(group);

    endInsertRows();
}

void ContentGroupListModel::clear()
{
    beginRemoveRows({}, 0, mContentGroupList.size() - 1);
    mContentGroupList.clear();
    endRemoveRows();
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

        return QEnums::toContentPrefVisibility(mContentFilter.getGroupVisibility(group.mId));
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
        const auto origVisibility = QEnums::toContentPrefVisibility(mContentFilter.getGroupVisibility(group.mId));

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

bool ContentGroupListModel::isModified(const ATProto::UserPreferences& userPreferences) const
{
    return mAdultContent != userPreferences.getAdultContent() || !mChangedVisibility.empty();
}

void ContentGroupListModel::saveTo(ATProto::UserPreferences& userPreferences) const
{
    userPreferences.setAdultContent(mAdultContent);

    for (const auto& [index, visibility] : mChangedVisibility)
    {
        const auto& contentGoup = mContentGroupList.at(index);
        const auto& label = contentGoup.mId;
        const auto labelVisibility = ATProto::UserPreferences::LabelVisibility(visibility);
        userPreferences.setLabelVisibility(label, labelVisibility);
    }
}


}
