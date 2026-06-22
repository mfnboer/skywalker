// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "chat_author_list_model.h"
#include "author_cache.h"

namespace Skywalker {

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

    const auto& chatAuthor = mList[index.row()];
    const auto& author = chatAuthor.getBasicProfile();
    const auto* change = getLocalChange(author.getDid());

    switch (Role(role))
    {
    case Role::Author:
        return QVariant::fromValue(chatAuthor);
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

        AuthorCache::instance().put(profile.getBasicProfile());
        list.push_back(profile);
    }

    return list;
}

void ChatAuthorListModel::setGetFeedInProgress(bool inProgress)
{
    if (inProgress != mGetFeedInProgress) {
        mGetFeedInProgress = inProgress;
        emit getFeedInProgressChanged();
    }
}

QHash<int, QByteArray> ChatAuthorListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Author), "author" },
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
