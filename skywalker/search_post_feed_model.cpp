// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "search_post_feed_model.h"
#include "list_store.h"
#include <ranges>

namespace Skywalker {

SearchPostFeedModel::SearchPostFeedModel(const QString& feedName, const QString& userDid,
                                         const IProfileStore& mutedReposts,
                                         const ContentFilter& contentFilter,
                                         const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                                         HashtagIndex& hashtags,
                                         QObject* parent) :
    AbstractPostFeedModel(userDid, mutedReposts, ListStore::NULL_STORE,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent),
    mFeedName(feedName)
{
}

void SearchPostFeedModel::setReverseFeed(bool reverse)
{
    qDebug() << "Reverse feed:" << reverse << mModelId << mFeedName;
    AbstractPostFeedModel::setReverseFeed(reverse);
    setReverseFeedFilteredPostModels(reverse);
}

void SearchPostFeedModel::setChronological(bool chronological)
{
    qDebug() << "Set chronological:" << chronological << mModelId << mFeedName;
    AbstractPostFeedModel::setChronological(chronological);
    setChronologicalFilteredPostModels(chronological);
}

void SearchPostFeedModel::clear()
{
    clearFilteredPostModels();

    if (!mFeed.empty())
    {
        beginRemoveRowsPhysical(0, mFeed.size() - 1);
        clearFeed();
        endRemoveRows();
    }

    mCursorNextPage.clear();
    qDebug() << "All posts removed";
}

void SearchPostFeedModel::setGetFeedInProgress(bool inProgress)
{
    AbstractPostFeedModel::setGetFeedInProgress(inProgress);

    for (auto& filterModel : mFilteredPostFeedModels)
        filterModel->setGetFeedInProgress(inProgress);
}

void SearchPostFeedModel::setFeedError(const QString& error)
{
    AbstractPostFeedModel::setFeedError(error);

    for (auto& filterModel : mFilteredPostFeedModels)
        filterModel->setFeedError(error);
}

void SearchPostFeedModel::getFeed(IFeedPager*)
{
    emit firstPage();
}

void SearchPostFeedModel::getFeedNextPage(IFeedPager*)
{
    emit nextPage();
}

int SearchPostFeedModel::setFeed(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed)
{
    if (!mFeed.empty())
        clear();

    return addFeed(std::forward<ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr>(feed));
}

int SearchPostFeedModel::addFeed(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mPosts.size();
    auto page = createPage(std::forward<ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr>(feed));

    mCursorNextPage = feed->mCursor.value_or("");

    if (page->mFeed.empty())
    {
        qDebug() << "All posts have been filtered from page";

        if (mCursorNextPage.isEmpty() && !mFeed.empty())
        {
            mFeed.back().setEndOfFeed(true);
            const auto index = createIndex(mFeed.size() - 1, 0);
            emit dataChanged(index, index, { int(Role::EndOfFeed) });

            setEndOfFeed(true);
            setEndOfFeedFilteredPostModels(true);
        }

        return 0;
    }

    const size_t newRowCount = mFeed.size() + page->mFeed.size();

    beginInsertRowsPhysical(mFeed.size(), newRowCount - 1);
    mFeed.insert(mFeed.end(), page->mFeed.begin(), page->mFeed.end());

    if (mCursorNextPage.isEmpty())
        mFeed.back().setEndOfFeed(true);

    endInsertRows();

    addPageToFilteredPostModels(*page, page->mFeed.size());

    if (mCursorNextPage.isEmpty())
    {
        setEndOfFeed(true);
        setEndOfFeedFilteredPostModels(true);
    }

    qDebug() << "New feed size:" << mFeed.size();
    return page->mFeed.size();
}

FilteredSearchPostFeedModel* SearchPostFeedModel::addVideoFilter(int rowSize)
{
    auto filter = std::make_unique<VideoPostFilter>();
    return addFilteredPostFeedModel(std::move(filter), rowSize);
}

FilteredSearchPostFeedModel* SearchPostFeedModel::addMediaFilter(int rowSize)
{
    auto filter = std::make_unique<MediaPostFilter>();
    return addFilteredPostFeedModel(std::move(filter), rowSize);
}

void SearchPostFeedModel::deleteFilteredPostFeedModel(FilteredSearchPostFeedModel* postFeedModel)
{
    removeFilteredPostFeedModel(postFeedModel);
}

FilteredSearchPostFeedModel::Ptr SearchPostFeedModel::removeFilteredPostFeedModel(FilteredSearchPostFeedModel* postFeedModel)
{
    const int index = findFilteredPostFeedModel(postFeedModel);

    if (index < 0)
    {
        qWarning() << "Could not delete filtered post feed model:" << postFeedModel->getFeedName();
        return nullptr;
    }

    // Keep unique_ptr alive till the filteredPostFeedModelsChanged has been emitted.
    // Deleting it before causes a crash when you delete a filter that is not the
    // the last, in the QML engine on Android only!
    auto deletedModel = std::move(mFilteredPostFeedModels[index]);
    mFilteredPostFeedModels.erase(mFilteredPostFeedModels.begin() + index);

    emit filteredPostFeedModelsChanged();

    return deletedModel;
}

int SearchPostFeedModel::findFilteredPostFeedModel(FilteredSearchPostFeedModel* postFeedModel) const
{
    for (auto it = mFilteredPostFeedModels.begin(); it != mFilteredPostFeedModels.end(); ++it)
    {
        if (it->get() == postFeedModel)
            return it - mFilteredPostFeedModels.begin();
    }

    return -1;
}

QList<FilteredSearchPostFeedModel*> SearchPostFeedModel::getFilteredPostFeedModels() const
{
    auto models = mFilteredPostFeedModels | std::views::transform([](auto& m){ return m.get(); });
    return QList<FilteredSearchPostFeedModel*>(models.begin(), models.end());
}

FilteredSearchPostFeedModel* SearchPostFeedModel::getFilteredPostFeedModel(QEnums::ContentMode contentMode) const
{
    auto filterMode = QEnums::contentModeToFilterMode(contentMode);

    for (auto& model : mFilteredPostFeedModels)
    {
        if (model->getContentMode() == filterMode)
            return model.get();
    }

    return nullptr;
}

AbstractPostFeedModel& SearchPostFeedModel::getViewModel(QEnums::ContentMode contentMode)
{
    if (contentMode == getContentMode())
        return *this;

    auto* model = getFilteredPostFeedModel(contentMode);

    if (model)
        return *model;

    qDebug() << "No filter model for content mode:" << contentMode;
    return *this;
}

void SearchPostFeedModel::refreshAllData()
{
    AbstractPostFeedModel::refreshAllData();
    refreshAllFilteredModels();
}

void SearchPostFeedModel::refreshAllFilteredModels()
{
    for (auto& model : mFilteredPostFeedModels)
        model->refreshAllData();
}

void SearchPostFeedModel::makeLocalFilteredModelChange(const std::function<void(LocalProfileChanges*)>& update)
{
    for (auto& model : mFilteredPostFeedModels)
        update(model.get());
}

void SearchPostFeedModel::makeLocalFilteredModelChange(const std::function<void(LocalPostModelChanges*)>& update)
{
    for (auto& model : mFilteredPostFeedModels)
        update(model.get());
}

SearchPostFeedModel::Page::Ptr SearchPostFeedModel::createPage(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed)
{
    auto page = std::make_unique<Page>();

    for (size_t i = 0; i < feed->mPosts.size(); ++i)
    {
        const auto& feedEntry = feed->mPosts[i];

        if (feedEntry->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        {
            Post post(feedEntry);
            mContentFilterStats.reportChecked(post);

            if (auto reason = mustHideContent(post); reason.first != QEnums::HIDE_REASON_NONE)
            {
                mContentFilterStats.report(post, reason.first, reason.second);
                continue;
            }

            preprocess(post);
            page->pushPost(post);
        }
        else
        {
            qWarning() << "Unsupported post record type:" << int(feedEntry->mRecordType);
            page->pushPost(Post::createNotSupported(feedEntry->mRawRecordType));
        }
    }

    page->chronoCheck();

    qDebug() << "Created page:" << page->mFeed.size() << "posts";
    return page;
}

void SearchPostFeedModel::addPageToFilteredPostModels(const Page& page, int pageSize)
{
    for (auto& model : mFilteredPostFeedModels)
        model->addPosts(page.mFeed, pageSize, mCursorNextPage.isEmpty());
}

void SearchPostFeedModel::clearFilteredPostModels()
{
    for (auto& model : mFilteredPostFeedModels)
        model->clear();
}

void SearchPostFeedModel::setReverseFeedFilteredPostModels(bool reverse)
{
    qDebug() << "Reverse feed filtered post models:" << reverse << mModelId << mFeedName;

    for (auto& model : mFilteredPostFeedModels)
        model->setReverseFeed(reverse);
}

void SearchPostFeedModel::setEndOfFeedFilteredPostModels(bool endOfFeed)
{
    for (auto& model : mFilteredPostFeedModels)
        model->setEndOfFeed(endOfFeed);
}

void SearchPostFeedModel::setChronologicalFilteredPostModels(bool chronological)
{
    for (auto& model : mFilteredPostFeedModels)
        model->setChronological(chronological);
}

FilteredSearchPostFeedModel* SearchPostFeedModel::addFilteredPostFeedModel(IPostFilter::Ptr postFilter, int rowSize)
{
    Q_ASSERT(postFilter);
    qDebug() << "Add filtered post feed model:" << postFilter->getName();
    auto model = std::make_unique<FilteredSearchPostFeedModel>(
        std::move(postFilter), this, mUserDid, mMutedReposts, mContentFilter,
        mMutedWords, mFocusHashtags, mHashtags, this);

    model->setRowSize(rowSize);
    model->setModelId(mModelId);
    model->setReverseFeed(mReverseFeed);
    model->setChronological(isChronological());
    model->setPosts(mFeed, mFeed.size(), mCursorNextPage.isEmpty());
    model->setEndOfFeed(isEndOfFeed());
    model->setGetFeedInProgress(isGetFeedInProgress());
    auto* retval = model.get();

    mFilteredPostFeedModels.push_back(std::move(model));
    emit filteredPostFeedModelsChanged();

    return retval;
}

}
