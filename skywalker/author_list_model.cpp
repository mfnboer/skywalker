// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "author_list_model.h"
#include "author_cache.h"

namespace Skywalker {

AuthorListModel::ListEntry::ListEntry(const Profile& profile, const QString& listItemUri) :
    mProfile(profile),
    mListItemUri(listItemUri)
{
}

AuthorListModel::AuthorListModel(Type type, const QString& atId, const IProfileStore& mutedReposts,
                                 const ContentFilter& contentFilter, QObject* parent) :
    QAbstractListModel(parent),
    mType(type),
    mAtId(atId),
    mMutedReposts(mutedReposts),
    mContentFilter(contentFilter)
{
    qDebug() << "New author list model type:" << type << "atId:" << atId;
}

int AuthorListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mList.size();
}

QVariant AuthorListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& entry = mList[index.row()];
    const auto& author = entry.mProfile;
    const auto* change = getLocalChange(author.getDid());

    switch (Role(role))
    {
    case Role::Author:
        return QVariant::fromValue(author);
    case Role::FollowingUri:
        return change && change->mFollowingUri ? *change->mFollowingUri : author.getViewer().getFollowing();
    case Role::BlockingUri:
        return change && change->mBlockingUri ? *change->mBlockingUri : author.getViewer().getBlocking();
    case Role::ListItemUri:
        return entry.mListItemUri;
    case Role::AuthorMuted:
        return change && change->mMuted ? *change->mMuted : author.getViewer().isMuted();
    case Role::MutedReposts:
        return change && change->mMutedReposts ? *change->mMutedReposts : mMutedReposts.contains(author.getDid());
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void AuthorListModel::clear()
{
    if (!mList.empty())
    {
        beginRemoveRows({}, 0, mList.size() - 1);
        mList.clear();
        clearLocalChanges();
        endRemoveRows();
    }

    mCursor.clear();
    mRawLists.clear();
    mRawItemLists.clear();
}

void AuthorListModel::addAuthors(ATProto::AppBskyActor::ProfileViewList authors, const QString& cursor)
{
    qDebug() << "Add authors:" << authors.size() << "cursor:" << cursor;
    mCursor = cursor;

    const auto list = filterAuthors(authors);
    const size_t newRowCount = mList.size() + list.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), list.begin(), list.end());
    endInsertRows();

    mRawLists.push_back(std::forward<ATProto::AppBskyActor::ProfileViewList>(authors));
    qDebug() << "New list size:" << mList.size();
}

void AuthorListModel::addAuthors(ATProto::AppBskyGraph::ListItemViewList listItems, const QString& cursor)
{
    qDebug() << "Add list item authors:" << listItems.size() << "cursor:" << cursor;
    mCursor = cursor;
    const size_t newRowCount = mList.size() + listItems.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);

    for (const auto& item : listItems)
    {
        ListEntry entry(Profile(item->mSubject.get()), item->mUri);
        AuthorCache::instance().put(entry.mProfile);
        mList.push_back(entry);
    }

    endInsertRows();

    mRawItemLists.push_back(std::forward<ATProto::AppBskyGraph::ListItemViewList>(listItems));
    qDebug() << "New list size:" << mList.size();
}

void AuthorListModel::prependAuthor(const Profile& author, const QString& listItemUri)
{
    qDebug() << "Preprend author:" << author.getHandle();
    AuthorCache::instance().put(author);

    beginInsertRows({}, 0, 0);
    mList.push_front(ListEntry(author, listItemUri));
    endInsertRows();

    qDebug() << "New list size:" << mList.size();
}

void AuthorListModel::deleteEntry(int index)
{
    qDebug() << "Delete entry:" << index;

    if (index < 0 || (size_t)index >= mList.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mList.size();
        return;
    }

    beginRemoveRows({}, index, index);
    mList.erase(mList.begin() + index);
    endRemoveRows();
}

AuthorListModel::AuthorList AuthorListModel::filterAuthors(const ATProto::AppBskyActor::ProfileViewList& authors) const
{
    AuthorList list;

    for (const auto& author : authors)
    {
        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(author->mLabels);

        if (visibility == QEnums::CONTENT_VISIBILITY_HIDE_POST || visibility == QEnums::CONTENT_VISIBILITY_HIDE_MEDIA)
        {
            qDebug() << "Filter content of author:" << author->mHandle << author->mDid << warning;
            continue;
        }

        if (mType != Type::AUTHOR_LIST_MUTES && author->mViewer->mMuted)
        {
            qDebug() << "Muted author:" << author->mHandle << author->mDid;
            continue;
        }

        const ListEntry entry(Profile(author.get()));
        AuthorCache::instance().put(entry.mProfile);
        list.push_back(entry);
    }

    return list;
}

QHash<int, QByteArray> AuthorListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Author), "author" },
        { int(Role::FollowingUri), "followingUri" },
        { int(Role::BlockingUri), "blockingUri" },
        { int(Role::ListItemUri), "listItemUri" },
        { int(Role::AuthorMuted), "authorMuted" },
        { int(Role::MutedReposts), "mutedReposts" }
    };

    return roles;
}

void AuthorListModel::blockingUriChanged()
{
    changeData({ int(Role::BlockingUri) });
}

void AuthorListModel::followingUriChanged()
{
    changeData({ int(Role::FollowingUri) });
}

void AuthorListModel::mutedChanged()
{
    changeData({ int(Role::AuthorMuted) });
}

void AuthorListModel::mutedRepostsChanged()
{
    changeData({ int(Role::MutedReposts) });
}

void AuthorListModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mList.size() - 1, 0), roles);
}

}
