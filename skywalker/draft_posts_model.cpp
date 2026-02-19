// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts_model.h"
#include "draft_posts.h"
#include "lexicon/lexicon.h"
#include "list_store.h"
#include "meme_maker.h"
#include "unicode_fonts.h"

namespace Skywalker {

DraftPostsModel::DraftPostsModel(const QString& userDid,
                                 const IProfileStore& mutedReposts,
                                 const IContentFilter& contentFilter,
                                 const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                                 HashtagIndex& hashtags,
                                 QObject* parent) :
    AbstractPostFeedModel(userDid, mutedReposts, ListStore::NULL_STORE,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent)
{
}

void DraftPostsModel::setDraftPosts(DraftPosts* draftPosts)
{
    mDraftPosts = draftPosts;
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

void DraftPostsModel::setFeed(std::vector<ATProto::AppBskyFeed::PostFeed> feed, const QString& cursor)
{
    qDebug() << "Set feed:" << feed.size() << "cursor:" << cursor;

    mCursor = cursor;

    if (!mFeed.empty())
        clear();

    mRawFeed = std::move(feed);

    if (!mRawFeed.empty())
    {
        beginInsertRows({}, 0, mRawFeed.size() - 1);

        for (int i = 0; i < (int)mRawFeed.size(); ++i)
        {
            Post post(mRawFeed[i][0]);
            mFeed.push_back(post);
        }

        endInsertRows();
    }

    if (!mFeed.empty() && mCursor.isEmpty())
    {
        mFeed.back().setEndOfFeed(true);
        changeData({ int(Role::EndOfFeed) });
    }
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
    const auto& rawThread = mRawFeed[index];

    for (int i = 0; i < (int)rawThread.size(); ++i)
    {
        const auto& feedViewPost = rawThread[i];
        Post post(feedViewPost);
        thread.push_back(post);
    }

    return thread;
}

QString DraftPostsModel::getStoredMediaWarning(int index) const
{
    if (index < 0 || index >= (int)mFeed.size())
        return {};

    const auto& post = mFeed[index];

    // NOTE: feedContext is abused to store the media storage warning
    return post.getFeedContext();
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
    case Role::PostExternal:
    {
        auto externalView = post.getExternalView();

        if (externalView && externalView->getTitle().isEmpty() && externalView->getDescription().isEmpty())
            getPostExternal(index.row());

        break;
    }
    case Role::PostRecord:
    {
        auto recordView = post.getRecordView();

        if (recordView && recordView->getNotFound())
            getPostRecord(index.row());

        break;
    }
    case Role::PostRecordWithMedia:
    {
        auto recordWithMediaView = post.getRecordWithMediaView();

        if (recordWithMediaView)
        {
            const auto& recordView = recordWithMediaView->getRecord();

            if (!recordView.isNull() && recordView.getNotFound())
                getPostRecord(index.row());
        }

        break;
    }
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
        text += QString("<br>%011/%02").arg(UnicodeFonts::THREAD_SYMBOL).arg(threadLength);
        return text;
    }
    case Role::PostPlainText:
    {
        auto text = result.toString();
        text += QString("\n%011/%02").arg(UnicodeFonts::THREAD_SYMBOL).arg(threadLength);
        return text;
    }
    default:
        break;
    }

    return result;
}

QList<ImageView> DraftPostsModel::createDraftImages(const Post& post) const
{
    const QList<ImageView> imageViews = post.getDraftImages();

    if (imageViews.empty())
        return {};

    if (mPostUriDraftImagesMap.contains(post.getUri()))
        return mPostUriDraftImagesMap.at(post.getUri());

    QList<ImageView> draftViews;
    draftViews.reserve(imageViews.size());

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

void DraftPostsModel::getPostExternal(int index) const
{
    if (!mDraftPosts)
    {
        qWarning() << "Draft posts not set";
        return;
    }

    const auto& post = mFeed[index];

    if (mGettingPostExternal.contains(post.getUri()))
        return;

    const_cast<DraftPostsModel*>(this)->mGettingPostExternal.insert(post.getUri());
    mDraftPosts->getPostExternal(post, index);
}

void DraftPostsModel::updatePostExternal(const Post& post, int index)
{
    qDebug() << "Update external:" << post.getUri() << "index:" << index;

    if (!mGettingPostExternal.contains(post.getUri()))
    {
        qWarning() << "External getting not in progress:" << post.getUri();
        return;
    }

    mGettingPostExternal.erase(post.getUri());

    if (index < 0 || index >= (int)mFeed.size())
    {
        // This may happen if a draft gets delete while getting the external was still in progress
        qWarning() << "Invalid index:" << index;
        return;
    }

    const auto& oldPost = mFeed[index];

    if (oldPost.getUri() != post.getUri())
    {
        qWarning() << "Post URI does not match:" << post.getUri() << "index:" << index;
        return;
    }

    mFeed[index] = post;
    changeData({ int(Role::PostExternal) });
}

void DraftPostsModel::getPostRecord(int index) const
{
    if (!mDraftPosts)
    {
        qWarning() << "Draft posts not set";
        return;
    }

    const auto& post = mFeed[index];

    if (mGettingPostRecord.contains(post.getUri()))
        return;

    const_cast<DraftPostsModel*>(this)->mGettingPostRecord.insert(post.getUri());
    mDraftPosts->getPostRecord(post, index);
}

void DraftPostsModel::updatePostRecord(const Post& post, int index)
{
    qDebug() << "Update record:" << post.getUri() << "index:" << index;

    if (!mGettingPostRecord.contains(post.getUri()))
    {
        qWarning() << "Record getting not in progress:" << post.getUri();
        return;
    }

    mGettingPostRecord.erase(post.getUri());

    if (index < 0 || index >= (int)mFeed.size())
    {
        // This may happen if a draft gets delete while getting the record was still in progress
        qWarning() << "Invalid index:" << index;
        return;
    }

    const auto& oldPost = mFeed[index];

    if (oldPost.getUri() != post.getUri())
    {
        qWarning() << "Post URI does not match:" << post.getUri() << "index:" << index;
        return;
    }

    mFeed[index] = post;
    changeData({ int(Role::PostRecord), int(Role::PostRecordWithMedia) });
}

void DraftPostsModel::updatePostRecordFailed(const Post& post, int index)
{
    qDebug() << "Update record failed:" << post.getUri() << "index:" << index;
    mGettingPostRecord.erase(post.getUri());
}

}
