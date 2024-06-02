// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts_model.h"
#include "draft_posts.h"

namespace Skywalker {

DraftPostsModel::DraftPostsModel(const QString& userDid, const IProfileStore& following,
                                 const IProfileStore& mutedReposts,
                                 const IContentFilter& contentFilter, const Bookmarks& bookmarks,
                                 const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                                 HashtagIndex& hashtags,
                                 QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, contentFilter, bookmarks, mutedWords, focusHashtags, hashtags, parent)
{
}

int DraftPostsModel::getMaxDrafts() const
{
    return DraftPosts::MAX_DRAFTS;
}

void DraftPostsModel::clear()
{
    qDebug() << "Clear feed";

    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        mRawFeed.clear();
        endRemoveRows();
    }
}

void DraftPostsModel::setFeed(std::vector<ATProto::AppBskyFeed::PostFeed> feed)
{
    qDebug() << "Set feed:" << feed.size();

    if (!mFeed.empty())
        clear();

    mRawFeed = std::move(feed);

    if (mRawFeed.empty())
        return;

    beginInsertRows({}, 0, mRawFeed.size() - 1);

    for (int i = 0; i < (int)mRawFeed.size(); ++i)
    {
        Post post(mRawFeed[i][0].get(), i);
        mFeed.push_back(post);
    }

    if (!mFeed.empty())
        mFeed.back().setEndOfFeed(true);

    endInsertRows();
}

void DraftPostsModel::deleteDraft(int index)
{
    qDebug() << "Delete draft:" << index;

    if (index < 0 || index >= (int)mFeed.size())
        return;

    const bool endOfFeed = getPost(index).isEndOfFeed();

    beginRemoveRows({}, index, index);
    deletePost(index);
    mRawFeed.erase(mRawFeed.begin() + index);
    endRemoveRows();

    if (endOfFeed && !mFeed.empty())
    {
        mFeed.back().setEndOfFeed(true);
        changeData({ int(Role::EndOfFeed) });
    }
}

std::vector<Post> DraftPostsModel::getThread(int index) const
{
    if (index < 0 || index >= (int)mRawFeed.size())
        return {};

    std::vector<Post> thread;

    for (int i = 0; i < (int)mRawFeed[index].size(); ++i)
    {
        const auto& feedViewPost = mRawFeed[index][i];
        Post post(feedViewPost.get(), i);
        thread.push_back(post);
    }

    return thread;
}

QVariant DraftPostsModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mFeed.size())
        return {};

    QVariant result = AbstractPostFeedModel::data(index, role);
    const int threadLength = mRawFeed[index.row()].size();

    if (threadLength <= 1)
        return result;

    switch (Role(role))
    {
    case Role::PostText:
    {
        auto text = result.toString();
        text += QString("<br>🧵1/%1").arg(threadLength);
        return text;
    }
    case Role::PostPlainText:
    {
        auto text = result.toString();
        text += QString("\n🧵1/%1").arg(threadLength);
        return text;
    }
    default:
        break;
    }

    return result;
}

}
