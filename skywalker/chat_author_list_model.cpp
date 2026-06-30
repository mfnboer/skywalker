// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "chat_author_list_model.h"

namespace Skywalker {

ChatAuthorListModel::ListEntry::ListEntry(const ChatBasicProfile& profile, QDateTime joinRequestedAt) :
    mProfile(profile),
    mJoinRequestedAt(joinRequestedAt)
{
}

ChatAuthorListModel::ChatAuthorListModel(Type type,
                                 const IProfileStore& mutedReposts,
                                 const IProfileStore& timelineHide,
                                 const ContentFilter& contentFilter,
                                 QObject* parent) :
    QAbstractListModel(parent),
    mType(type),
    mMutedReposts(mutedReposts),
    mTimelineHide(timelineHide),
    mContentFilter(contentFilter)
{
    qDebug() << "New chat author list model type:" << type;
}

int ChatAuthorListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mList.size();
}

QVariant ChatAuthorListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& listEntry = mList[index.row()];
    const auto& chatAuthor = listEntry.mProfile;
    const auto& author = chatAuthor.getBasicProfile();
    const auto* change = getLocalChange(author.getDid());

    switch (Role(role))
    {
    case Role::ChatAuthor:
        return QVariant::fromValue(chatAuthor);
    case Role::JoinRequestedAt:
        return listEntry.mJoinRequestedAt;
    case Role::FollowingUri:
        return change && change->mFollowingUri ? *change->mFollowingUri : author.getViewer().getFollowing();
    case Role::BlockingUri:
        return change && change->mBlockingUri ? *change->mBlockingUri : author.getViewer().getBlocking();
    case Role::AuthorMuted:
        return change && change->mMuted ? *change->mMuted : author.getViewer().isMuted();
    case Role::MutedReposts:
        return change && change->mMutedReposts ? *change->mMutedReposts : mMutedReposts.contains(author.getDid());
    case Role::HideFromTimeline:
        return mTimelineHide.contains(author.getDid());
    case Role::EndOfList:
        return isEndOfList() && index.row() == (int)mList.size() - 1;
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void ChatAuthorListModel::clear()
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
    mRawRequestLists.clear();
}

void ChatAuthorListModel::addAuthors(ATProto::ChatBskyActor::ProfileViewBasic::List authors, const QString& cursor)
{
    qDebug() << "Add authors:" << authors.size() << "cursor:" << cursor;
    mCursor = cursor;

    const auto list = filterAuthors(authors);
    const size_t newRowCount = mList.size() + list.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), list.begin(), list.end());
    endInsertRows();

    mRawLists.push_back(std::forward<ATProto::ChatBskyActor::ProfileViewBasic::List>(authors));
    qDebug() << "New list size:" << mList.size();
    setEndOfList();
}

void ChatAuthorListModel::addJoinRequests(ATProto::ChatBskyGroup::JoinRequestView::List joinRequests, const QString& cursor)
{
    qDebug() << "Add join requests:" << joinRequests.size() << "cursor:" << cursor;
    mCursor = cursor;

    const size_t newRowCount = mList.size() + joinRequests.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);

    for (const auto& request : joinRequests)
    {
        ListEntry entry(ChatBasicProfile(*request->mRequestedBy), request->mRequestedAt);
        mList.push_back(entry);
    }

    endInsertRows();

    mRawRequestLists.push_back(std::forward<ATProto::ChatBskyGroup::JoinRequestView::List>(joinRequests));
    qDebug() << "New list size:" << mList.size();
    setEndOfList();
}

void ChatAuthorListModel::prependAuthor(const ATProto::ChatBskyActor::ProfileViewBasic& author)
{
    qDebug() << "Preprend author:" << author.mHandle;
    const ChatBasicProfile profile(author);

    beginInsertRows({}, 0, 0);
    mList.push_front(ListEntry{profile});
    endInsertRows();

    qDebug() << "New list size:" << mList.size();

    if (mList.size() == 1)
        setEndOfList();
}

void ChatAuthorListModel::deleteAuthor(const QString& did)
{
    qDebug() << "Delete author:" << did;
    int index = -1;

    for (int i = 0; i < (int)mList.size(); ++ i)
    {
        const ChatBasicProfile& chatProfile = mList[i].mProfile;

        if (chatProfile.getBasicProfile().getDid() == did)
        {
            qDebug() << "Author found:" << chatProfile.getBasicProfile().getName();
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        qDebug() << "Author not found:" << did;
        return;
    }

    deleteEntry(index);
}

void ChatAuthorListModel::deleteEntry(int index)
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

    if (index == (int)mList.size())
        setEndOfList();
}

void ChatAuthorListModel::setEndOfList()
{
    if (isEndOfList() && !mList.empty())
    {
        const auto index = createIndex(mList.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfList) });
    }
}

ChatAuthorListModel::AuthorList ChatAuthorListModel::filterAuthors(const ATProto::ChatBskyActor::ProfileViewBasic::List& authors) const
{
    AuthorList list;

    for (const auto& author : authors)
    {
        const ChatBasicProfile profile(*author);
        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(
            profile.getBasicProfile(), author->mLabels);

        if (visibility == QEnums::CONTENT_VISIBILITY_HIDE_POST || visibility == QEnums::CONTENT_VISIBILITY_HIDE_MEDIA)
        {
            qDebug() << "Filter content of author:" << author->mHandle << author->mDid << warning;
            continue;
        }

        list.push_back(ListEntry{profile});
    }

    return list;
}

void ChatAuthorListModel::setGetFeedInProgress(bool inProgress)
{
    if (inProgress != mGetFeedInProgress) {
        mGetFeedInProgress = inProgress;
        emit getFeedInProgressChanged();

        if (mGetFeedInProgress)
            clearFeedError();
    }
}

void ChatAuthorListModel::setFeedError(const QString& error)
{
    if (error != mFeedError)
    {
        mFeedError = error;
        emit feedErrorChanged();
    }
}

QHash<int, QByteArray> ChatAuthorListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::ChatAuthor), "chatAuthor" },
        { int(Role::JoinRequestedAt), "joinRequestedAt" },
        { int(Role::FollowingUri), "followingUri" },
        { int(Role::BlockingUri), "blockingUri" },
        { int(Role::AuthorMuted), "authorMuted" },
        { int(Role::MutedReposts), "mutedReposts" },
        { int(Role::HideFromTimeline), "hideFromTimeline" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

void ChatAuthorListModel::blockingUriChanged()
{
    changeData({ int(Role::BlockingUri) });
}

void ChatAuthorListModel::followingUriChanged()
{
    changeData({ int(Role::FollowingUri) });
}

void ChatAuthorListModel::mutedChanged()
{
    changeData({ int(Role::AuthorMuted) });
}

void ChatAuthorListModel::mutedRepostsChanged()
{
    changeData({ int(Role::MutedReposts) });
}

void ChatAuthorListModel::hideFromTimelineChanged()
{
    changeData({ int(Role::HideFromTimeline) });
}

void ChatAuthorListModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mList.size() - 1, 0), roles);
}

}
