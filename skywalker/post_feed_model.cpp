// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_feed_model.h"
#include "definitions.h"
#include "skywalker.h"
#include "user_settings.h"
#include <atproto/lib/time_monitor.h>
#include <algorithm>
#include <ranges>

namespace Skywalker {

using namespace std::chrono_literals;

PostFeedModel::PostFeedModel(const QString& feedName, const FeedVariant* feedVariant,
                             const QString& userDid, const IProfileStore& following,
                             const IProfileStore& mutedReposts,
                             const IProfileStore& feedHide,
                             const IContentFilter& contentFilter,
                             const IMatchWords& mutedWords,
                             const FocusHashtags& focusHashtags,
                             HashtagIndex& hashtags,
                             const ATProto::UserPreferences& userPrefs,
                             UserSettings& userSettings,
                             FollowsActivityStore& followsActivityStore,
                             ATProto::Client::SharedPtr bsky,
                             QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, feedHide, contentFilter, mutedWords, focusHashtags, hashtags, parent),
    mUserPreferences(userPrefs),
    mUserSettings(userSettings),
    mFollowsActivityStore(followsActivityStore),
    mFeedName(feedName)
{
    if (feedVariant)
        std::visit([this](auto&& variant){ setFeedVariant(variant); }, *feedVariant);

    createInteractionSender(bsky);

    connect(&mUserSettings, &UserSettings::contentLanguageFilterChanged, this,
            [this]{ emit languageFilterConfiguredChanged(); });

    connect(&mUserSettings, &UserSettings::feedHideRepliesChanged, this,
            [this](QString did, QString feedUri)
            {
                if (did == mUserDid && feedUri == getFeedUri())
                    mFeedHideReplies = mUserSettings.getFeedHideReplies(did, feedUri);
            });

    connect(&mUserSettings, &UserSettings::feedHideFollowingChanged, this,
            [this](QString did, QString feedUri)
            {
                if (did == mUserDid && feedUri == getFeedUri())
                    mFeedHideFollowing = mUserSettings.getFeedHideFollowing(did, feedUri);
            });
}

void PostFeedModel::createInteractionSender(ATProto::Client::SharedPtr bsky)
{
    if (feedAcceptsInteractions() && !getFeedDid().isEmpty())
    {
        Q_ASSERT(bsky);

        if (bsky)
        {
            qDebug() << "Create feed interaction sender:" << getFeedDid();
            mInteractionSender = std::make_unique<InteractionSender>(getFeedDid(), bsky, this);
        }
        else
        {
            qWarning() << "No ATProto client, feedDid:" << getFeedDid();
        }
    }
}

const QString PostFeedModel::getFeedDid() const
{
    if (!mGeneratorView.isNull())
        return mGeneratorView.getDid();

    return {};
}

QString PostFeedModel::getFeedUri() const
{
    if (!mListView.isNull())
        return mListView.getUri();

    if (!mGeneratorView.isNull())
        return mGeneratorView.getUri();

    return {};
}

QEnums::FeedType PostFeedModel::getFeedType() const
{
    return !mListView.isNull() ? QEnums::FEED_LIST : QEnums::FEED_GENERATOR;
}

bool PostFeedModel::feedAcceptsInteractions() const
{
    if (!mGeneratorView.isNull())
        return mGeneratorView.acceptsInteractions();

    return false;
}

QString PostFeedModel::getPreferencesFeedKey() const
{
    static const QString HOME_KEY = HOME_FEED;
    return mIsHomeFeed ? HOME_KEY : getFeedUri();
}

bool PostFeedModel::isLanguageFilterConfigured() const
{
    return !mUserSettings.getShowUnknownContentLanguage(mUserDid) ||
           !mUserSettings.getContentLanguages(mUserDid).empty();
}

void PostFeedModel::enableLanguageFilter(bool enabled)
{
    if (enabled != mLanguageFilterEnabled)
    {
        mLanguageFilterEnabled = enabled;
        emit languageFilterEnabledChanged();
    }
}

LanguageList PostFeedModel::getFilterdLanguages() const
{
    const QStringList langs = mUserSettings.getContentLanguages(mUserDid);
    return LanguageUtils::getLanguages(langs);
}

bool PostFeedModel::showPostWithMissingLanguage() const
{
    return mUserSettings.getShowUnknownContentLanguage(mUserDid);
}

void PostFeedModel::setFeed(const std::deque<Post>& filteredPosts,
                            const ContentFilterStats::PostHideInfoMap* postHideInfoMap,
                            ContentFilterStats::Details& hideDetails)
{
    clear();
    mPostHideInfoMap = postHideInfoMap;
    auto page = createPageFilteredPosts(filteredPosts, hideDetails);
    addPage(std::move(page));
}

void PostFeedModel::setFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    clear();
    addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed));
}

void PostFeedModel::setFeed(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr&& feed)
{
    clear();
    addFeed(std::forward<ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr>(feed));
}

int PostFeedModel::prependFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    qDebug() << "Prepend feed:" << feed->mFeed.size() << "current size:" << mFeed.size();

    if (feed->mFeed.empty())
        return 0;

    if (mFeed.empty())
    {
        setFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed));
        return 0;
    }

    const int gapId = insertFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed), 0);
    return gapId;
}

int PostFeedModel::gapFillFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed, int gapId)
{
    qDebug() << "Fill gap:" << gapId << "feed:" << feed->mFeed.size() << "current size:" << mFeed.size();

    if (!mGapIdIndexMap.count(gapId))
    {
        qWarning() << "Gap does not exist:" << gapId;
        return 0;
    }

    const int gapIndex = mGapIdIndexMap[gapId];
    mGapIdIndexMap.erase(gapId);

    if (gapIndex > (int)mFeed.size() - 1)
    {
        qWarning() << "Gap:" << gapId << "index:" << gapIndex << "beyond feed size" << mFeed.size();
        return 0;
    }

    Q_ASSERT(mFeed[gapIndex].getGapId() == gapId);

    // Remove gap place holder
    beginRemoveRows({}, gapIndex, gapIndex);
    mFeed.erase(mFeed.begin() + gapIndex);
    addToIndices(-1, gapIndex);
    endRemoveRows();

    qDebug() << "Removed place holder post:" << gapIndex;
    logIndices();

    return insertFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed), gapIndex, gapId);
}

void PostFeedModel::insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize, int fillGapId)
{
    if (fillGapId > 0)
        gapFillFilteredPostModels(page, pageSize, fillGapId);
    else if (feedInsertIt == mFeed.begin())
        prependPageToFilteredPostModels(page, pageSize);
    else if (feedInsertIt == mFeed.end())
        addPageToFilteredPostModels(page, pageSize);

    mFeed.insert(feedInsertIt, page.mFeed.begin(), page.mFeed.begin() + pageSize);

    for (const auto& post : page.mFeed)
    {
        const auto& cid = post.getCid();
        if (!cid.isEmpty())
            storeCid(cid);
    }

    cleanupStoredCids();
}

void PostFeedModel::addPageToFilteredPostModels(const Page& page, int pageSize)
{
    for (auto& model : mFilteredPostFeedModels)
        model->addPosts(page.mFeed, pageSize);
}

void PostFeedModel::prependPageToFilteredPostModels(const Page& page, int pageSize)
{
    for (auto& model : mFilteredPostFeedModels)
        model->prependPosts(page.mFeed, pageSize);
}

void PostFeedModel::gapFillFilteredPostModels(const Page& page, int pageSize, int gapId)
{
    for (auto& model : mFilteredPostFeedModels)
        model->gapFill(page.mFeed, pageSize, gapId);
}

void PostFeedModel::removeHeadFromFilteredPostModels(size_t headSize)
{
    for (auto& model : mFilteredPostFeedModels)
        model->removeHeadPosts(mFeed, headSize);
}

void PostFeedModel::removeTailFromFilteredPostModels(size_t tailSize)
{
    for (auto& model : mFilteredPostFeedModels)
    {
        model->removeTailPosts(mFeed, tailSize);
    }
}

void PostFeedModel::clearFilteredPostModels()
{
    for (auto& model : mFilteredPostFeedModels)
        model->clear();
}

void PostFeedModel::setEndOfFeedFilteredPostModels(bool endOfFeed)
{
    for (auto& model : mFilteredPostFeedModels)
        model->setEndOfFeed(endOfFeed);
}

int PostFeedModel::insertFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed, int insertIndex, int fillGapId)
{
    qDebug() << "Insert feed:" << feed->mFeed.size() << "index:" << insertIndex << "fillGap:" << fillGapId;
    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed));

    if (page->mFeed.empty())
    {
        qDebug() << "Page has no posts" << "checked:" << mContentFilterStats.checkedPosts() << "filtered:" << mContentFilterStats.total();

        if (fillGapId > 0)
            gapFillFilteredPostModels(*page, 0, fillGapId);

        return 0;
    }

    const auto [overlapStart, overlapDiscardedPost] = findOverlapStart(*page, insertIndex);

    if (!overlapStart)
    {
        int gapId = 0;

        if (!overlapDiscardedPost)
        {
            page->mFeed.push_back(Post::createGapPlaceHolder(page->mCursorNextPage));
            gapId = page->mFeed.back().getGapId();
            qDebug() << "Create new gap:" << gapId;
        }

        const size_t lastInsertIndex = insertIndex + page->mFeed.size() - 1;

        beginInsertRows({}, insertIndex, lastInsertIndex);
        insertPage(mFeed.begin() + insertIndex, *page, page->mFeed.size(), fillGapId);
        addToIndices(page->mFeed.size(), insertIndex);

        size_t indexOffset = 0;

        if (gapId != 0)
        {
            mGapIdIndexMap[gapId] = lastInsertIndex;
            indexOffset = 1; // offset for the place holder post.
        }

        if (!page->mCursorNextPage.isEmpty())
            mIndexCursorMap[lastInsertIndex - indexOffset] = page->mCursorNextPage;

        endInsertRows();

        mLastInsertedRowIndex = (int)lastInsertIndex;
        qDebug() << "Full feed inserted, new size:" << mFeed.size() << "checked:" << mContentFilterStats.checkedPosts() << "filtered:" << mContentFilterStats.total();
        logIndices();
        return gapId;
    }

    if (*overlapStart == 0)
    {
        qDebug() << "Full overlap, no new posts" << "checked:" << mContentFilterStats.checkedPosts() << "filtered:" << mContentFilterStats.total();

        if (fillGapId > 0)
        {
            const Page emptyPage;
            gapFillFilteredPostModels(emptyPage, 0, fillGapId);
        }

        return 0;
    }

    const auto overlapEnd = findOverlapEnd(*page, insertIndex);

    beginInsertRows({}, insertIndex, insertIndex + *overlapStart - 1);
    insertPage(mFeed.begin() + insertIndex, *page, *overlapStart, fillGapId);
    addToIndices(*overlapStart, insertIndex);

    if (!page->mCursorNextPage.isEmpty() && overlapEnd)
        mIndexCursorMap[*overlapStart + *overlapEnd] = page->mCursorNextPage;

    endInsertRows();

    mLastInsertedRowIndex = insertIndex + *overlapStart - 1;
    qDebug() << "Inserted" << *overlapStart << "posts, new size:" << mFeed.size()<< "checked:" << mContentFilterStats.checkedPosts() << "filtered:" << mContentFilterStats.total();
    logIndices();
    return 0;
}

void PostFeedModel::clear()
{
    clearFilteredPostModels();

    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        mIndexCursorMap.clear();
        mGapIdIndexMap.clear();
        endRemoveRows();
    }

    qDebug() << "All posts removed";
}

void PostFeedModel::reset()
{
    while (!mFilteredPostFeedModels.empty())
    {
        const auto index = mFilteredPostFeedModels.size() - 1;
        emit filteredPostFeedModelAboutToBeDeleted(index);
        mFilteredPostFeedModels.pop_back();
        emit filteredPostFeedModelsChanged();
        emit filteredPostFeedModelDeleted(index);
    }

    clear();
}

void PostFeedModel::addFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mFeed.size();
    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed));
    addPage(std::move(page));
}

void PostFeedModel::addFeed(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr&& feed)
{
    qDebug() << "Add quote posts:" << feed->mPosts.size();
    auto page = createPage(std::forward<ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr>(feed));
    addPage(std::move(page));
}

void PostFeedModel::addPage(Page::Ptr page)
{
    if (!page->mFeed.empty())
    {
        const size_t newRowCount = mFeed.size() + page->mFeed.size();

        beginInsertRows({}, mFeed.size(), newRowCount - 1);
        insertPage(mFeed.end(), *page, page->mFeed.size());
        endInsertRows();

        mLastInsertedRowIndex = newRowCount - 1;
        qDebug() << "New feed size:" << mFeed.size() << "checked:" << mContentFilterStats.checkedPosts() << "filtered:" << mContentFilterStats.total();
    }
    else
    {
        qDebug() << "All posts have been filtered from page";
    }

    if (!page->mCursorNextPage.isEmpty())
    {
        mIndexCursorMap[mFeed.size() - 1] = page->mCursorNextPage;
    }
    else
    {
        setEndOfFeed(true);
        setEndOfFeedFilteredPostModels(true);

        if (page->mFeed.empty() && !mFeed.empty())
        {
            // Set end of feed indication on existing row.
            mFeed.back().setEndOfFeed(true);
            const auto index = createIndex(mFeed.size() - 1, 0);
            emit dataChanged(index, index, { int(Role::EndOfFeed) });
        }
    }

    logIndices();
}

void PostFeedModel::removeTailPosts(int size)
{
    qDebug() << "Remove tail posts:" << size << "current size:" << mFeed.size();

    if (size <= 0 || size >= (int)mFeed.size())
        return;

    const auto removeIndexCursorIt = mIndexCursorMap.lower_bound(mFeed.size() - size - 1);

    if (removeIndexCursorIt == mIndexCursorMap.end())
    {
        qWarning() << "Cannot remove" << size << "posts";
        logIndices();
        return;
    }

    const size_t removeIndex = removeIndexCursorIt->first + 1;

    if (removeIndex >= mFeed.size())
    {
        qDebug() << "Cannot remove beyond end, removeIndex:" << removeIndex << "size:" << mFeed.size();
        logIndices();
        return;
    }

    const size_t removeCount = mFeed.size() - removeIndex;
    removeTailFromFilteredPostModels(removeCount);

    beginRemoveRows({}, removeIndex, mFeed.size() - 1);
    removePosts(removeIndex, removeCount);
    mIndexCursorMap.erase(std::next(removeIndexCursorIt), mIndexCursorMap.end());

    for (auto it = mGapIdIndexMap.begin(); it != mGapIdIndexMap.end(); )
    {
        if (it->second >= removeIndex)
            it = mGapIdIndexMap.erase(it);
        else
            ++it;
    }

    setEndOfFeed(false);
    setEndOfFeedFilteredPostModels(false);
    endRemoveRows();

    qDebug() << "Removed tail rows:" << size << "new size:" << mFeed.size();
    logIndices();
}

void PostFeedModel::removeHeadPosts(int size)
{
    qDebug() << "Remove head posts:" << size << "current size:" << mFeed.size();

    if (size <= 0 || size >= (int)mFeed.size())
        return;

    size_t removeEndIndex = size - 1;
    while (removeEndIndex < mFeed.size() - 1 && mFeed[removeEndIndex + 1].isGap())
        ++removeEndIndex;

    if (removeEndIndex >= mFeed.size() - 1)
    {
        qWarning() << "Cannot remove beyond end, removeIndex:" << removeEndIndex << "size:" << mFeed.size();
        logIndices();
        return;
    }

    const auto removeEndIndexCursorIt = mIndexCursorMap.upper_bound(removeEndIndex);
    const int removeSize = removeEndIndex + 1;
    removeHeadFromFilteredPostModels(removeSize);

    beginRemoveRows({}, 0, removeEndIndex);
    removePosts(0, removeSize);
    Q_ASSERT(!mFeed.front().isGap());
    mIndexCursorMap.erase(mIndexCursorMap.begin(), removeEndIndexCursorIt);

    for (auto it = mGapIdIndexMap.begin(); it != mGapIdIndexMap.end(); )
    {
        if (it->second <= removeEndIndex)
            it = mGapIdIndexMap.erase(it);
        else
            ++it;
    }

    addToIndices(-removeSize, removeSize);
    endRemoveRows();

    qDebug() << "Removed head rows, new size:" << mFeed.size();
    logIndices();
}

void PostFeedModel::removePosts(int startIndex, int size)
{
    Q_ASSERT(startIndex >=0 && startIndex + size <= (int)mFeed.size());

    for (int i = startIndex; i < startIndex + size; ++i)
        removeStoredCid(mFeed[i].getCid());

    mFeed.erase(mFeed.begin() + startIndex, mFeed.begin() + startIndex + size);
}

QString PostFeedModel::getLastCursor() const
{
    if (isEndOfFeed() || mIndexCursorMap.empty())
        return {};

    return mIndexCursorMap.rbegin()->second;
}

const Post* PostFeedModel::getGapPlaceHolder(int gapId) const
{
    const auto it = mGapIdIndexMap.find(gapId);

    if (it == mGapIdIndexMap.end())
    {
        qDebug() << "Gap does not exist:" << gapId;
        return nullptr;
    }

    const int gapIndex = it->second;

    if (gapIndex > (int)mFeed.size())
    {
        qWarning() << "Gap:" << gapId << "index:" << gapIndex << "beyond feed size" << mFeed.size();
        return nullptr;
    }

    const auto* gap = &mFeed[gapIndex];
    Q_ASSERT(gap->getGapId() == gapId);
    return gap;
}

void PostFeedModel::setGetFeedInProgress(bool inProgress)
{
    AbstractPostFeedModel::setGetFeedInProgress(inProgress);

    for (auto& filterModel : mFilteredPostFeedModels)
        filterModel->setGetFeedInProgress(inProgress);
}

void PostFeedModel::setFeedError(const QString& error)
{
    AbstractPostFeedModel::setFeedError(error);

    for (auto& filterModel : mFilteredPostFeedModels)
        filterModel->setFeedError(error);
}

void PostFeedModel::getFeed(IFeedPager* pager)
{
    if (mIsHomeFeed)
    {
        pager->updateTimeline(2, Skywalker::TIMELINE_PREPEND_PAGE_SIZE);
        return;
    }

    Q_ASSERT(mModelId > -1);

    if (!mGeneratorView.isNull())
        pager->getFeed(mModelId);
    else if (!mListView.isNull())
        pager->syncListFeed(mModelId);
    else
        qWarning() << "No view to get page";
}

void PostFeedModel::getFeedNextPage(IFeedPager* pager)
{
    if (mIsHomeFeed)
    {
        pager->getTimelineNextPage();
        return;
    }

    Q_ASSERT(mModelId > -1);

    if (!mGeneratorView.isNull())
        pager->getFeedNextPage(mModelId);
    else if (!mListView.isNull())
        pager->getListFeedNextPage(mModelId);
    else
        qWarning() << "No view to get next page";
}

FilteredPostFeedModel* PostFeedModel::addAuthorFilter(const BasicProfile& profile)
{
    auto filter = std::make_unique<AuthorPostFilter>(profile);
    return addFilteredPostFeedModel(std::move(filter));
}

Q_INVOKABLE FilteredPostFeedModel* PostFeedModel::addHashtagFilter(const QString& hashtag)
{
    auto filter = std::make_unique<HashtagPostFilter>(hashtag);
    return addFilteredPostFeedModel(std::move(filter));
}

FilteredPostFeedModel* PostFeedModel::addFocusHashtagFilter(FocusHashtagEntry* focusHashtag)
{
    auto filter = std::make_unique<FocusHashtagsPostFilter>(*focusHashtag);
    return addFilteredPostFeedModel(std::move(filter));
}

FilteredPostFeedModel* PostFeedModel::addVideoFilter()
{
    auto filter = std::make_unique<VideoPostFilter>();
    return addFilteredPostFeedModel(std::move(filter));
}

FilteredPostFeedModel* PostFeedModel::addMediaFilter()
{
    auto filter = std::make_unique<MediaPostFilter>();
    return addFilteredPostFeedModel(std::move(filter));
}

FilteredPostFeedModel* PostFeedModel::addFilteredPostFeedModel(IPostFilter::Ptr postFilter)
{
    Q_ASSERT(postFilter);
    qDebug() << "Add filtered post feed model:" << postFilter->getName();
    auto model = std::make_unique<FilteredPostFeedModel>(
            std::move(postFilter), this, mUserDid, mFollowing, mMutedReposts, mContentFilter,
            mMutedWords, mFocusHashtags, mHashtags, this);

    model->setModelId(mModelId);
    model->setPosts(mFeed, mFeed.size());
    model->setEndOfFeed(isEndOfFeed());
    model->setGetFeedInProgress(isGetFeedInProgress());
    auto* retval = model.get();

    emit filteredPostFeedModelAboutToBeAdded();
    mFilteredPostFeedModels.push_back(std::move(model));
    emit filteredPostFeedModelsChanged();
    emit filteredPostFeedModelAdded(retval);

    if (isHomeFeed())
        mUserSettings.setTimelineViews(mUserDid, filteredPostFeedModelsToJson());

    return retval;
}

void PostFeedModel::addFilteredPostFeedModel(FilteredPostFeedModel::Ptr model)
{
    Q_ASSERT(model);
    qDebug() << "Add model:" << model->getFeedName();
    auto* retval = model.get();
    emit filteredPostFeedModelAboutToBeAdded();
    mFilteredPostFeedModels.push_back(std::move(model));
    emit filteredPostFeedModelsChanged();
    emit filteredPostFeedModelAdded(retval);

    if (isHomeFeed())
        mUserSettings.setTimelineViews(mUserDid, filteredPostFeedModelsToJson());
}

int PostFeedModel::findFilteredPostFeedModel(FilteredPostFeedModel* postFeedModel) const
{
    for (auto it = mFilteredPostFeedModels.begin(); it != mFilteredPostFeedModels.end(); ++it)
    {
        if (it->get() == postFeedModel)
            return it - mFilteredPostFeedModels.begin();
    }

    return -1;
}

void PostFeedModel::deleteFilteredPostFeedModel(FilteredPostFeedModel* postFeedModel)
{
    removeFilteredPostFeedModel(postFeedModel);
}

FilteredPostFeedModel::Ptr PostFeedModel::removeFilteredPostFeedModel(FilteredPostFeedModel* postFeedModel)
{
    const int index = findFilteredPostFeedModel(postFeedModel);

    if (index < 0)
    {
        qWarning() << "Could not delete filtered post feed model:" << postFeedModel->getFeedName();
        return nullptr;
    }

    emit filteredPostFeedModelAboutToBeDeleted(index);

    // Keep unique_ptr alive till the filteredPostFeedModelsChanged has been emitted.
    // Deleting it before causes a crash when you delete a filter that is not the
    // the last, in the QML engine on Android only!
    auto deletedModel = std::move(mFilteredPostFeedModels[index]);
    mFilteredPostFeedModels.erase(mFilteredPostFeedModels.begin() + index);

    emit filteredPostFeedModelsChanged();
    emit filteredPostFeedModelDeleted(index);

    if (isHomeFeed())
        mUserSettings.setTimelineViews(mUserDid, filteredPostFeedModelsToJson());

    return deletedModel;
}

bool PostFeedModel::equalModels(QList<FilteredPostFeedModel*> models) const
{
    for (int i = 0; i < models.size(); ++i)
    {
        if (models[i] != mFilteredPostFeedModels[i].get())
            return false;
    }

    return true;
}

void PostFeedModel::reorderFilteredPostFeedModels(const QList<FilteredPostFeedModel*>& models)
{
    qDebug() << "Reorder filtered post feed models";
    Q_ASSERT(models.size() == (int)mFilteredPostFeedModels.size());

    if (models.size() != (int)mFilteredPostFeedModels.size())
    {
        qWarning() << "Sizes differ:" << models.size() << mFilteredPostFeedModels.size();
        return;
    }

    if (equalModels(models))
    {
        qDebug() << "No order change";
        return;
    }

    for (auto* model : models)
    {
        auto deletedModel = removeFilteredPostFeedModel(model);

        if (deletedModel)
            deletedModel.release();
    }

    for (auto* model : models)
    {
        auto modelPtr = FilteredPostFeedModel::Ptr(model);
        addFilteredPostFeedModel(std::move(modelPtr));
    }
}

QList<FilteredPostFeedModel*> PostFeedModel::getFilteredPostFeedModels() const
{
    auto models = mFilteredPostFeedModels | std::views::transform([](auto& m){ return m.get(); });
    return QList<FilteredPostFeedModel*>(models.begin(), models.end());
}

void PostFeedModel::addFilteredPostFeedModelsFromSettings()
{
    const auto json = mUserSettings.getTimelineViews(mUserDid);
    addFilteredPostFeedModelsFromJson(json);
}

QJsonObject PostFeedModel::filteredPostFeedModelsToJson()
{
    QJsonArray filterArray;

    for (const auto& filter : mFilteredPostFeedModels)
        filterArray.push_back(filter->getPostFilter().toJson());

    QJsonObject json;
    json.insert("postFilters", filterArray);
    return json;
}

int PostFeedModel::findFilteredPostFeedModelByFilter(IPostFilter* filter) const
{
    for (auto it = mFilteredPostFeedModels.begin(); it != mFilteredPostFeedModels.end(); ++it)
    {
        if (&(*it)->getPostFilter() == filter)
            return it - mFilteredPostFeedModels.begin();
    }

    return -1;
}

void PostFeedModel::addFilteredPostFeedModelsFromJson(const QJsonObject& json)
{
    if (json.isEmpty())
        return;

    const ATProto::XJsonObject xjson(json);

    try {
        const QJsonArray filterArray = xjson.getRequiredArray("postFilters");
        for (const auto& filterValue : filterArray)
        {
            const auto& filterJson = filterValue.toObject();
            IPostFilter::Ptr filter = IPostFilter::fromJson(filterJson, [this](IPostFilter* f){
                const int index = findFilteredPostFeedModelByFilter(f);

                if (index >= 0)
                    emit filteredPostFeedModelUpdated(index);
            });

            if (filter)
                addFilteredPostFeedModel(std::move(filter));
        }
    }
    catch (ATProto::InvalidJsonException& e) {
        qWarning() << e.msg();
    }
}

void PostFeedModel::refreshAllData()
{
    AbstractPostFeedModel::refreshAllData();
    refreshAllFilteredModels();
}

void PostFeedModel::refreshAllFilteredModels()
{
    for (auto& model : mFilteredPostFeedModels)
        model->refreshAllData();
}

void PostFeedModel::makeLocalFilteredModelChange(const std::function<void(LocalProfileChanges*)>& update)
{
    for (auto& model : mFilteredPostFeedModels)
        update(model.get());
}

void PostFeedModel::makeLocalFilteredModelChange(const std::function<void(LocalPostModelChanges*)>& update)
{
    for (auto& model : mFilteredPostFeedModels)
        update(model.get());
}

bool PostFeedModel::addFeedInteraction(const QString& feedDid,
                                       ATProto::AppBskyFeed::Interaction::EventType event,
                                       const QString& postUri, const QString& feedContext)
{
    if (!mInteractionSender || feedDid != getFeedDid())
        return false;

    mInteractionSender->addInteraction(event, postUri, feedContext);
    return true;
}

void PostFeedModel::removeFeedInteraction(const QString& feedDid,
                                          ATProto::AppBskyFeed::Interaction::EventType event,
                                          const QString& postUri)
{
    if (!mInteractionSender || feedDid != getFeedDid())
        return;

    mInteractionSender->removeInteraction(event, postUri);
}

void PostFeedModel::reportOnScreen(const QString& postUri)
{
    if (mInteractionSender)
        mInteractionSender->reportOnScreen(postUri);
}

void PostFeedModel::reportOffScreen(const QString& postUri, const QString& feedContext)
{
    if (mInteractionSender)
        mInteractionSender->reportOffScreen(postUri, feedContext);
}

void PostFeedModel::Page::addPost(const Post& post, bool isParent)
{
    mFeed.push_back(post);
    const auto& cid = post.getCid();

    if (!cid.isEmpty())
        mAddedCids.insert(post.getCid());

    if (isParent)
        mParentIndexMap[cid] = mFeed.size() - 1;
}

void PostFeedModel::Page::collectThreadgate(const Post& post)
{
    if (!post.isReply())
    {
        auto threadgate = post.getThreadgateView();

        if (threadgate)
            mRootUriToThreadgate[post.getUri()] = threadgate;
    }
}

bool PostFeedModel::Page::tryAddToExistingThread(const Post& post, const PostReplyRef& replyRef, ContentFilterStats& contentFilterStats)
{
    auto parentIt = mParentIndexMap.find(post.getCid());
    if (parentIt == mParentIndexMap.end())
        return false;

    // The post we add is already in the page, probably as a parent of another post

    const int parentIndex = parentIt->second;
    Q_ASSERT(parentIndex < (int)mFeed.size());

    const auto oldPost = mFeed[parentIndex];

    // Replace the existing post by this one, as this post has a reply ref.
    // A parent post does not have the information
    mFeed[parentIndex] = post;
    mFeed[parentIndex].setReplyRefTimestamp(oldPost.getTimelineTimestamp());
    mFeed[parentIndex].setParentInThread(oldPost.isParentInThread());

    if (replyRef.mParent.getCid() == replyRef.mRoot.getCid())
    {
        // The root is already in this page
        mFeed[parentIndex].setPostType(oldPost.getPostType());
        mFeed[parentIndex].setParentInThread(true);
        return true;
    }

    mFeed[parentIndex].setPostType(QEnums::POST_REPLY);
    mFeed[parentIndex].setParentInThread(true);

    // Add parent of this parent
    qDebug() << "Add parent:" << replyRef.mParent.getCid();
    contentFilterStats.reportChecked(replyRef.mParent);
    mFeed.insert(mFeed.begin() + parentIndex, replyRef.mParent);
    mFeed[parentIndex].setReplyRefTimestamp(oldPost.getTimelineTimestamp());
    mFeed[parentIndex].setPostType(oldPost.getPostType());

    if (mFeed[parentIndex].getReplyToCid() == replyRef.mRoot.getCid())
        mFeed[parentIndex].setParentInThread(true);

    mAddedCids.insert(replyRef.mParent.getCid());
    mParentIndexMap.erase(parentIt);

    for (auto& [cid, idx] : mParentIndexMap)
    {
        if (idx > parentIndex)
            ++idx;
    }

    mParentIndexMap[replyRef.mParent.getCid()] = parentIndex;
    return true;
}

bool PostFeedModel::getFeedHideReplies() const
{
    if (!mFeedHideReplies)
        mFeedHideReplies = mUserSettings.getFeedHideReplies(mUserDid, getPreferencesFeedKey());

    return *mFeedHideReplies;
}

bool PostFeedModel::getFeedHideFollowing() const
{
    if (!mFeedHideFollowing)
        mFeedHideFollowing = mUserSettings.getFeedHideFollowing(mUserDid, getPreferencesFeedKey());

    return *mFeedHideFollowing;
}

std::pair<QEnums::HideReasonType, ContentFilterStats::Details> PostFeedModel::mustHideContent(const Post& post) const
{
    const auto& feedViewPref = mUserPreferences.getFeedViewPref(getPreferencesFeedKey());

    if (feedViewPref.mHideReposts && post.isRepost())
        return { QEnums::HIDE_REASON_REPOST, nullptr };

    if (post.isQuotePost())
    {
        if (auto reason = mustHideQuotePost(post); reason != QEnums::HIDE_REASON_NONE)
            return { reason, nullptr };
    }

    if (auto reason = AbstractPostFeedModel::mustHideContent(post); reason.first != QEnums::HIDE_REASON_NONE)
        return reason;

    if (getFeedHideFollowing())
    {
        if (mFollowing.contains(post.getAuthorDid()))
        {
            qDebug() << "Hide post from followed user:" << post.getAuthorDid();
            return { QEnums::HIDE_REASON_HIDE_FOLLOWING_FROM_FEED, nullptr };
        }
    }

    if (post.isRepost())
    {
        const auto repostedBy = post.getRepostedBy();

        if (repostedBy->getDid() == post.getAuthorDid())
        {
            if (!mUserSettings.getShowSelfReposts(mUserDid))
                return { QEnums::HIDE_REASON_SELF_REPOST, nullptr };
        }
        else if (!mUserSettings.getShowFollowedReposts(mUserDid))
        {
            if (mFollowing.contains(post.getAuthorDid()))
                return { QEnums::HIDE_REASON_FOLLOWING_REPOST, nullptr };

            // Technically you do not follow yourself, but your own posts
            // show up in your timeline, so we hide such resposts.
            if (post.getAuthorDid() == mUserDid)
                return { QEnums::HIDE_REASON_FOLLOWING_REPOST, nullptr };
        }
    }

    // All posts should be video posts in a video feed.
    if (getContentMode() == QEnums::CONTENT_MODE_VIDEO && !post.hasVideo(true))
    {
        qWarning() << "Non-video post in video feed!";
        return { QEnums::HIDE_REASON_CONTENT_MODE, nullptr };
    }

    if (!passLanguageFilter(post))
    {
        auto& languages = post.getLanguages();
        const QString lang = languages.empty() ? tr("no language") : languages.first().getShortCode();
        return { QEnums::HIDE_REASON_LANGUAGE, lang };
    }

    return { QEnums::HIDE_REASON_NONE, nullptr };
}

bool PostFeedModel::passLanguageFilter(const Post& post) const
{
    if (!mLanguageFilterEnabled)
        return true;

    const LanguageList& postLangs = post.getLanguages();

    if (postLangs.empty())
    {
        if (mUserSettings.getShowUnknownContentLanguage(mUserDid))
            return true;

        qDebug() << "Unknown language:" << post.getText();
        return false;
    }

    const QStringList sortedContentLangs = mUserSettings.getContentLanguages(mUserDid);

    if (sortedContentLangs.empty())
        return true;

    for (const Language& lang : postLangs)
    {
        if (std::binary_search(sortedContentLangs.cbegin(), sortedContentLangs.cend(), lang.getShortCode()))
            return true;
    }

    qDebug() << "No matching language:" << post.getText();
    return false;
}

QEnums::HideReasonType PostFeedModel::mustHideReply(const Post& post, const std::optional<PostReplyRef>& replyRef) const
{
    const auto& feedViewPref = mUserPreferences.getFeedViewPref(getPreferencesFeedKey());

    if (feedViewPref.mHideReplies)
        return QEnums::HIDE_REASON_REPLY;

    if (getFeedHideReplies())
        return QEnums::HIDE_REASON_REPLY;

    // Always show the replies of the user.
    if (post.getAuthor().getDid() == mUserDid)
        return QEnums::HIDE_REASON_NONE;

    if (mUserSettings.getHideRepliesInThreadFromUnfollowed(mUserDid))
    {
        // In case of blocked posts there is no reply ref.
        // Surely someone that blocks you is not a friend of yours.
        if (!replyRef)
            return QEnums::HIDE_REASON_REPLY_THREAD_UNFOLLOWED;

        const auto parentAuthor = replyRef->mParent.getAuthor();
        const auto& parentDid = parentAuthor.getDid();

        // Do not show replies in threads starting with blocked and not-found root posts.
        // Unless the reply is directly to the user.
        if (replyRef->mRoot.isPlaceHolder() && parentDid != mUserDid)
            return QEnums::HIDE_REASON_REPLY_THREAD_UNFOLLOWED;

        const auto rootAuthor = replyRef->mRoot.getAuthor();
        const auto& rootDid = rootAuthor.getDid();

        // Always show replies to the user
        if (parentDid != mUserDid && !mFollowing.contains(rootDid))
            return QEnums::HIDE_REASON_REPLY_THREAD_UNFOLLOWED;
    }

    if (feedViewPref.mHideRepliesByUnfollowed)
    {
        // In case of blocked posts there is no reply ref.
        // Surely someone that blocks you is not a friend of yours.
        if (!replyRef)
            return QEnums::HIDE_REASON_REPLY_TO_UNFOLLOWED;

        // Do not show replies to blocked and not-found posts
        if (replyRef->mParent.isPlaceHolder())
            return QEnums::HIDE_REASON_REPLY_TO_UNFOLLOWED;

        const auto parentAuthor = replyRef->mParent.getAuthor();
        const auto& parentDid = parentAuthor.getDid();

        // Always show replies to the user
        if (parentDid == mUserDid)
            return QEnums::HIDE_REASON_NONE;

        const auto rootAuthor = replyRef->mRoot.getAuthor();
        const auto& rootDid = rootAuthor.getDid();

        // Always show replies in a thread from the user
        if (rootDid == mUserDid)
            return QEnums::HIDE_REASON_NONE;

        if (!mFollowing.contains(parentDid))
            return QEnums::HIDE_REASON_REPLY_TO_UNFOLLOWED;
    }

    return QEnums::HIDE_REASON_NONE;
}

QEnums::HideReasonType PostFeedModel::mustHideQuotePost(const Post& post) const
{
    Q_ASSERT(post.isQuotePost());
    const auto& feedViewPref = mUserPreferences.getFeedViewPref(getPreferencesFeedKey());

    if (feedViewPref.mHideQuotePosts)
        return QEnums::HIDE_REASON_QUOTE;

    if (!mUserSettings.getShowQuotesWithBlockedPost(mUserDid))
    {
        const auto& record = post.getRecordViewFromRecordOrRecordWithMedia();

        if (record && record->getBlocked())
            return QEnums::HIDE_REASON_QUOTE_BLOCKED_POST;
    }

    return QEnums::HIDE_REASON_NONE;
}

void PostFeedModel::reportActivity(const Post& post)
{
    const auto did = post.isRepost() ? post.getRepostedBy()->getDid() : post.getAuthorDid();

    if (did.isEmpty())
        return;

    const QDateTime timestamp = post.isRepost() ? post.getRepostTimestamp() : post.getIndexedAt();
    mFollowsActivityStore.reportActivity(did, timestamp);
}

void PostFeedModel::Page::setThreadgates()
{
    if (mRootUriToThreadgate.empty())
        return;

    for (auto& post : mFeed)
    {
        if (!post.isReply())
            continue;

        const auto& rootUri = post.getReplyRootUri();

        if (mRootUriToThreadgate.contains(rootUri))
            post.setThreadgateView(mRootUriToThreadgate[rootUri]);
    }
}

void PostFeedModel::Page::foldThreads()
{
    int threadStartIndex = -1;
    int threadEndIndex = -1;

    for (int i = 0; i < (int)mFeed.size(); ++i)
    {
        auto& post = mFeed[i];

        switch (post.getPostType())
        {
        case QEnums::POST_ROOT:
            threadStartIndex = i;
            break;
        case QEnums::POST_LAST_REPLY:
            threadEndIndex = i;
            foldPosts(threadStartIndex, threadEndIndex);
            threadStartIndex = -1;
            break;
        case QEnums::POST_REPLY:
        case QEnums::POST_STANDALONE:
        case QEnums::POST_THREAD:
            break;
        }
    }
}

void PostFeedModel::Page::foldPosts(int startIndex, int endIndex)
{
    if (startIndex < 0 || endIndex < 0 || startIndex > endIndex)
    {
        qWarning() << "Invalid thread:" << startIndex << endIndex;
        return;
    }

    if (endIndex >= (int)mFeed.size())
    {
        qWarning() << "Thread out of range:" << startIndex << endIndex << "size:" << mFeed.size();
        return;
    }

    // Do not fold short threads
    if (endIndex - startIndex < 4)
        return;

    // The first and the last 2 posts will stay visible. The rest gets folded.
    mFeed[startIndex + 1].setFoldedPostType(QEnums::FOLDED_POST_FIRST);

    for (int i = startIndex + 2; i < endIndex - 1; ++i)
    {
        mFeed[i].setFoldedPostType(QEnums::FOLDED_POST_SUBSEQUENT);
    }
}

PostFeedModel::Page::Ptr PostFeedModel::createPage(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    const bool assembleThreads = mUserSettings.getAssembleThreads(mUserDid);
    auto page = std::make_unique<Page>();

    for (size_t i = 0; i < feed->mFeed.size(); ++i)
    {
        const auto& feedEntry = feed->mFeed[i];

        if (feedEntry->mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        {
            Post post(feedEntry);
            page->collectThreadgate(post);
            reportActivity(post);

            // Due to reposting a post can show up multiple times in the feed.
            // Also overlapping pages (on prepend) can come in as we look for new posts.
            // On gap fill, some posts may already been shown, e.g. as parent on a reply.
            if (cidIsStored(post.getCid()))
            {
                if (!post.isRepost() &&
                    (page->mOldestDiscaredTimestamp.isNull() ||
                        post.getTimelineTimestamp() < page->mOldestDiscaredTimestamp))
                {
                    qDebug() << "Discard post:" << post.getTimelineTimestamp();
                    page->mOldestDiscaredTimestamp = post.getTimelineTimestamp();
                }

                continue;
            }

            mContentFilterStats.reportChecked(post);

            if (auto reason = mustHideContent(post); reason.first != QEnums::HIDE_REASON_NONE)
            {
                mContentFilterStats.report(post, reason.first, reason.second);
                continue;
            }

            const auto& replyRef = post.getViewPostReplyRef();

            if (replyRef)
            {
                reportActivity(replyRef->mRoot);
                reportActivity(replyRef->mParent);
            }

            // Reposted replies are displayed without thread context
            if (replyRef && !post.isRepost())
            {
                page->collectThreadgate(replyRef->mRoot);

                // If a reply fits in an existing thread then always show it as it provides
                // context to the user. The leaf of this thread is a reply that passed
                // through the filter settings.
                if (assembleThreads && page->tryAddToExistingThread(post, *replyRef, mContentFilterStats))
                {
                    preprocess(post);
                    continue;
                }

                if (auto reason = mustHideReply(post, replyRef); reason != QEnums::HIDE_REASON_NONE)
                {
                    mContentFilterStats.report(post, reason, nullptr);

                    // Preprocess replies when they are not shown. Those can help
                    // identify threads.
                    preprocess(post);
                    continue;
                }

                bool rootAdded = false;
                const auto& rootCid = replyRef->mRoot.getCid();
                const auto& parentCid = replyRef->mParent.getCid();

                if (assembleThreads && !rootCid.isEmpty() && rootCid != parentCid &&
                    !cidIsStored(rootCid) && !page->cidAdded(rootCid))
                {
                    mContentFilterStats.reportChecked(replyRef->mRoot);

                    if (auto reason = mustHideContent(replyRef->mRoot); reason.first == QEnums::HIDE_REASON_NONE)
                    {
                        preprocess(replyRef->mRoot);
                        page->addPost(replyRef->mRoot);
                        page->mFeed.back().setPostType(QEnums::POST_ROOT);
                        rootAdded = true;
                    }
                    else
                    {
                        mContentFilterStats.report(replyRef->mRoot, reason.first, reason.second);
                    }
                }

                // If the parent was seen already, but the root not, then show the parent
                // again for consistency of the thread.
                if (assembleThreads &&
                    ((!parentCid.isEmpty() && !cidIsStored(parentCid) && !page->cidAdded(parentCid)) || rootAdded))
                {
                    mContentFilterStats.reportChecked(replyRef->mParent);

                    if (auto reason = mustHideContent(replyRef->mParent); reason.first == QEnums::HIDE_REASON_NONE)
                    {
                        preprocess(replyRef->mParent);
                        page->addPost(replyRef->mParent, true);
                        auto& parentPost = page->mFeed.back();
                        parentPost.setPostType(rootAdded ? QEnums::POST_REPLY : QEnums::POST_ROOT);

                        // Determine author of the parent's parent
                        if (parentPost.getReplyToCid() == rootCid)
                        {
                            parentPost.setReplyToAuthor(replyRef->mRoot.getAuthor());
                            parentPost.setParentInThread(rootAdded);
                        }

                        post.setPostType(QEnums::POST_LAST_REPLY);
                        post.setParentInThread(true);
                    }
                    else
                    {
                        mContentFilterStats.report(replyRef->mParent, reason.first, reason.second);
                    }
                }
            }
            else if (post.isReply() && !post.isRepost())
            {
                // A post can still be a reply even if there is no reply reference.
                // The reference may be missing due to blocked posts.
                if (auto reason = mustHideReply(post, {}); reason != QEnums::HIDE_REASON_NONE)
                {
                    mContentFilterStats.report(post, reason, nullptr);
                    continue;
                }
            }
            else
            {
                // A post may have been added already as a parent/root of a reply
                if (page->cidAdded(post.getCid()))
                {
                    qDebug() << "Post already added:" << post.getCid();
                    continue;
                }
            }

            preprocess(post);
            page->addPost(post);
        }
        else
        {
            qWarning() << "Unsupported post record type:" << int(feedEntry->mPost->mRecordType);
            page->addPost(Post::createNotSupported(feedEntry->mPost->mRawRecordType));
        }
    }

    if (feed->mCursor && !feed->mCursor->isEmpty())
    {
        page->mCursorNextPage = *feed->mCursor;
    }
    else
    {
        if (!page->mFeed.empty())
            page->mFeed.back().setEndOfFeed(true);
    }

    page->setThreadgates();
    page->foldThreads();

    qDebug() << "Created page, size:" << page->mFeed.size() << "checked:" << mContentFilterStats.checkedPosts() << "filtered:" << mContentFilterStats.total();
    return page;
}

PostFeedModel::Page::Ptr PostFeedModel::createPage(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr&& feed)
{
    auto page = std::make_unique<Page>();

    for (size_t i = 0; i < feed->mPosts.size(); ++i)
    {
        const auto& feedEntry = feed->mPosts[i];

        if (feedEntry->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        {
            Post post(feedEntry);
            preprocess(post);
            page->addPost(post);
        }
        else
        {
            qWarning() << "Unsupported post record type:" << int(feedEntry->mRecordType);
            page->addPost(Post::createNotSupported(feedEntry->mRawRecordType));
        }
    }

    if (feed->mCursor && !feed->mCursor->isEmpty())
    {
        page->mCursorNextPage = *feed->mCursor;
    }
    else
    {
        if (!page->mFeed.empty())
            page->mFeed.back().setEndOfFeed(true);
    }

    return page;
}

PostFeedModel::Page::Ptr PostFeedModel::createPageFilteredPosts(
    const std::deque<Post>& posts, const ContentFilterStats::Details& hideDetails)
{
    auto page = std::make_unique<Page>();

    Q_ASSERT(mHideReason);

    if (!mHideReason)
    {
        qWarning() << "No hide reason set";
        return page;
    }

    qDebug() << "Create page, posts:" << posts.size() << "reason:" << *mHideReason;

    for (const auto& post : posts)
    {
        if (!mustHideFilteredPost(post, hideDetails))
        {
            preprocess(post);
            page->addPost(post);
        }
    }

    if (!page->mFeed.empty())
        page->mFeed.back().setEndOfFeed(true);

    qDebug() << "Created page, size:" << page->mFeed.size();
    return page;
}

bool PostFeedModel::mustHideFilteredPost(
    const Post& post, const ContentFilterStats::Details& hideDetails) const
{
    if (*mHideReason == QEnums::HIDE_REASON_ANY)
        return false;

    if (!mPostHideInfoMap)
    {
        qWarning() << "Post hide info missing";
        return true;
    }

    const auto it = mPostHideInfoMap->find(post.getCid());

    if (it == mPostHideInfoMap->end())
    {
        qWarning() << "No hide info for post:" << post.getCid();
        return true;
    }

    const ContentFilterStats::PostHideInfo& postHideInfo = it->second;

    if (*mHideReason != postHideInfo.mHideReason)
        return true;

    if (std::holds_alternative<std::nullptr_t>(hideDetails))
        return false;

    if (std::holds_alternative<BasicProfile>(hideDetails) && std::holds_alternative<BasicProfile>(postHideInfo.mDetails))
        return std::get<BasicProfile>(hideDetails).getDid() != std::get<BasicProfile>(postHideInfo.mDetails).getDid();

    if (std::holds_alternative<MutedWordEntry>(hideDetails) && std::holds_alternative<MutedWordEntry>(postHideInfo.mDetails))
        return std::get<MutedWordEntry>(hideDetails).getValue() != std::get<MutedWordEntry>(postHideInfo.mDetails).getValue();

    if (std::holds_alternative<ContentLabel>(hideDetails) && std::holds_alternative<ContentLabel>(postHideInfo.mDetails))
    {
        const auto& label1 = std::get<ContentLabel>(hideDetails);
        const auto& label2 = std::get<ContentLabel>(postHideInfo.mDetails);

        if (label1.getDid() != label2.getDid())
            return true;

        if (label1.getLabelId().isEmpty())
            return false;

        return label1.getLabelId() != label2.getLabelId();
    }

    if (std::holds_alternative<QString>(hideDetails) && std::holds_alternative<QString>(postHideInfo.mDetails))
        return std::get<QString>(hideDetails) != std::get<QString>(postHideInfo.mDetails);

    return true;
}

std::tuple<std::optional<size_t>, bool> PostFeedModel::findOverlapStart(const Page& page, size_t feedIndex) const
{
    Q_ASSERT(mFeed.size() > feedIndex);
    QString cidFirstStoredPost;
    QDateTime timestampFirstStoredPost;
    bool overlapDiscardedPost = false;

    for (size_t i = feedIndex; i < mFeed.size(); ++i)
    {
        const auto& post = mFeed[i];

        if (post.isPlaceHolder())
            continue;

        cidFirstStoredPost = post.getCid();
        timestampFirstStoredPost = post.getTimelineTimestamp();
        break;
    }

    if (cidFirstStoredPost.isEmpty())
    {
        qWarning() << "There are no real posts in the feed!";
        return {};
    }

    if (!page.mOldestDiscaredTimestamp.isNull() && !timestampFirstStoredPost.isNull() &&
        page.mOldestDiscaredTimestamp < timestampFirstStoredPost)
    {
        overlapDiscardedPost = true;
    }

    for (size_t i = 0; i < page.mFeed.size(); ++i)
    {
        const auto& post = page.mFeed[i];

        // Check on timestamp because of repost.
        // A repost will have the cid of the original post.
        // We also put the parent/root of a reply in the timeline with the
        // timestamp of the reply.
        if (cidFirstStoredPost == post.getCid() && timestampFirstStoredPost == post.getTimelineTimestamp())
        {
            qDebug() << "Matching overlap index found:" << i;
            return {i, overlapDiscardedPost};
        }

        if (timestampFirstStoredPost > post.getTimelineTimestamp())
        {
            qDebug() << "Overlap start on timestamp found:" << i << timestampFirstStoredPost << post.getTimelineTimestamp();
            return {i, overlapDiscardedPost};
        }
    }

    // NOTE: the gap may be empty when the last post in the page is the predecessor of
    // the first post the stored feed. There is no way of knowing.
    qDebug() << "No overlap found, there is a gap";
    return {std::nullopt, overlapDiscardedPost};
}

std::optional<size_t> PostFeedModel::findOverlapEnd(const Page& page, size_t feedIndex) const
{
    Q_ASSERT(!page.mFeed.empty());
    const auto& cidLastPagePost = page.mFeed.back().getCid();
    const auto& timestampLastPagePost = page.mFeed.back().getTimelineTimestamp();

    for (size_t i = feedIndex; i < mFeed.size(); ++ i)
    {
        const auto& post = mFeed[i];

        if (post.isPlaceHolder())
            continue;

        if (cidLastPagePost == post.getCid() && timestampLastPagePost == post.getTimelineTimestamp())
        {
            qDebug() << "Last matching overlap index found:" << i;
            return i;
        }

        if (timestampLastPagePost >= post.getTimelineTimestamp())
        {
            qDebug() << "Overlap end on timestamp found:" << i << timestampLastPagePost << post.getTimelineTimestamp();
            return i;
        }
    }

    qWarning() << "No overlap found, page exceeds end of stored feed";
    return {};
}

void PostFeedModel::addToIndices(int offset, size_t startAtIndex)
{
    std::map<size_t, QString> newCursorMap;
    for (const auto& [index, cursor] : mIndexCursorMap)
    {
        if (index >= startAtIndex)
            newCursorMap[index + offset] = cursor;
        else
            newCursorMap[index] = cursor;
    }

    mIndexCursorMap = std::move(newCursorMap);

    for (auto& [gapId, index] : mGapIdIndexMap)
    {
        if (index >= startAtIndex)
            index += offset;
    }
}

void PostFeedModel::logIndices() const
{
    qDebug() << "INDEX CURSOR MAP:";
    for (const auto& [index, cursor] : mIndexCursorMap)
        qDebug() << "Index:" << index << "Cursor:" << cursor;

    qDebug() << "GAP INDEX MAP:";
    for (const auto& [gapId, index] : mGapIdIndexMap)
        qDebug() << "Gap:" << gapId << "Index:" << index;
}

}
