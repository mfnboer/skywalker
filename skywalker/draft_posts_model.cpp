// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts_model.h"
#include "author_cache.h"
#include "draft_posts.h"
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

void DraftPostsModel::clear()
{
    qDebug() << "Clear feed";

    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        std::visit([](auto&& rawFeed){ rawFeed.clear(); }, mRawFeed);
        endRemoveRows();
    }
}

void DraftPostsModel::setFeed(SkyDrafts feed)
{
    qDebug() << "Set feed:" << feed.size();

    if (!mFeed.empty())
        clear();

    mRawFeed = std::move(feed);
    auto& rawFeed = std::get<SkyDrafts>(mRawFeed);

    if (rawFeed.empty())
        return;

    beginInsertRows({}, 0, rawFeed.size() - 1);

    for (int i = 0; i < (int)rawFeed.size(); ++i)
    {
        Post post(rawFeed[i][0]);
        mFeed.push_back(post);
    }

    if (!mFeed.empty())
        mFeed.back().setEndOfFeed(true);

    endInsertRows();
}

void DraftPostsModel::setFeed(BlueskyDrafts feed)
{
    qDebug() << "Set feed:" << feed.size();

    if (!mFeed.empty())
        clear();

    mRawFeed = std::move(feed);
    auto& rawFeed = std::get<BlueskyDrafts>(mRawFeed);

    if (rawFeed.empty())
        return;

    beginInsertRows({}, 0, rawFeed.size() - 1);

    for (int i = 0; i < (int)rawFeed.size(); ++i)
    {
        const auto& draftView = rawFeed[i];
        const auto& draftPost = draftView->mDraft->mPosts[0];
        const auto postView = toPostView(draftPost, draftView);
        Post post(postView);
        mFeed.push_back(post);
    }

    // TODO: more pages
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
    std::visit([index](auto&& rawFeed){ rawFeed.erase(rawFeed.begin() + index); }, mRawFeed);
    endRemoveRows();

    if (endOfFeed && !mFeed.empty())
    {
        mFeed.back().setEndOfFeed(true);
        changeData({ int(Role::EndOfFeed) });
    }
}

std::vector<Post> DraftPostsModel::getThread(int index) const
{
    const int feedSize = std::visit([](auto&& rawFeed){ return rawFeed.size(); }, mRawFeed);

    if (index < 0 || index >= feedSize)
        return {};

    if (std::holds_alternative<SkyDrafts>(mRawFeed))
        return getSkyDraftThread(index);

    return getBlueskyDraftThread(index);
}

std::vector<Post> DraftPostsModel::getSkyDraftThread(int index) const
{
    std::vector<Post> thread;
    const auto& rawThread = std::get<SkyDrafts>(mRawFeed)[index];

    for (int i = 0; i < (int)rawThread.size(); ++i)
    {
        const auto& feedViewPost = rawThread[i];
        Post post(feedViewPost);
        thread.push_back(post);
    }

    return thread;
}

std::vector<Post> DraftPostsModel::getBlueskyDraftThread(int index) const
{
    std::vector<Post> thread;
    const auto& rawThread = std::get<BlueskyDrafts>(mRawFeed)[index];

    for (int i = 0; i < (int)rawThread->mDraft->mPosts.size(); ++i)
    {
        const auto& draftPost = rawThread->mDraft->mPosts[i];
        const auto postView = toPostView(draftPost, rawThread);
        Post post(postView);
        thread.push_back(post);
    }

    return thread;
}

ATProto::AppBskyFeed::PostView::SharedPtr DraftPostsModel::toPostView(
    const ATProto::AppBskyDraft::DraftPost::SharedPtr& draftPost,
    const ATProto::AppBskyDraft::DraftView::SharedPtr& draftView) const
{
    auto postView = std::make_shared<ATProto::AppBskyFeed::PostView>();
    postView->mUri = draftView->mId;
    auto user = AuthorCache::instance().get(mUserDid);
    postView->mAuthor = user->getProfileBasicView();

    auto postRecord = std::make_shared<ATProto::AppBskyFeed::Record::Post>();
    postRecord->mText = draftPost->mText;
    postRecord->mLabels = draftPost->mLabels;
    postRecord->mLanguages = draftView->mDraft->mLangs;
    postRecord->mCreatedAt = draftView->mCreatedAt;

    postView->mRecord = postRecord;
    postView->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
    postView->mIndexedAt = draftView->mUpdatedAt;

    return postView;
}

static int getThreadLength(const ATProto::AppBskyFeed::PostFeed& postFeed)
{
    return postFeed.size();
}

static int getThreadLength(const ATProto::AppBskyDraft::DraftView::SharedPtr& draftView)
{
    return draftView->mDraft->mPosts.size();
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
    const int threadLength = std::visit([index](auto&& rawFeed){ return getThreadLength(rawFeed[index.row()]); }, mRawFeed);

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

}
