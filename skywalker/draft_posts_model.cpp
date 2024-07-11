// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts_model.h"
#include "draft_posts.h"
#include "meme_maker.h"

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
        Post post(mRawFeed[i][0]);
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
        Post post(feedViewPost);
        thread.push_back(post);
    }

    return thread;
}

QVariant DraftPostsModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mFeed.size())
        return {};

    const auto& post = mFeed[index.row()];

    switch (Role(role))
    {
    case Role::PostImages:
        return QVariant::fromValue(createDraftImages(post));
    default:
        break;
    }

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

QList<ImageView> DraftPostsModel::createDraftImages(const Post& post) const
{
    QList<ImageView> imageViews = post.getDraftImages();

    if (imageViews.empty())
        return {};

    if (mPostUriDraftImagesMap.contains(post.getUri()))
        return mPostUriDraftImagesMap.at(post.getUri());

    QList<ImageView> draftViews;

    for (const auto& view : imageViews)
    {
        if (view.getMemeTopText().isEmpty() && view.getMemeBottomText().isEmpty())
        {
            draftViews.push_back(view);
        }
        else
        {
            MemeMaker memeMaker;

            if (!memeMaker.setOrigImage(view.getFullSizeUrl()))
            {
                qWarning() << "Cannot load image:" << view.getFullSizeUrl();
                draftViews.push_back(view);
                continue;
            }

            auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
            memeMaker.setTopText(view.getMemeTopText());
            memeMaker.setBottomText(view.getMemeBottomText());
            ImageView draftView(memeMaker.getMemeImgSource(), view.getAlt(), view.getMemeTopText(), view.getMemeBottomText());
            draftViews.push_back(draftView);
            auto source = std::make_unique<SharedImageSource>(memeMaker.getMemeImgSource(), imgProvider);
            const_cast<DraftPostsModel*>(this)->mMemeSources.emplace_back(std::move(source));
            memeMaker.releaseMemeOwnership();
        }
    }

    Q_ASSERT(draftViews.size() == imageViews.size());
    const_cast<DraftPostsModel*>(this)->mPostUriDraftImagesMap[post.getUri()] = draftViews;
    return draftViews;
}

}
