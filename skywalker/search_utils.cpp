// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "search_utils.h"
#include "skywalker.h"
#include <QTextBoundaryFinder>

namespace Skywalker {

static std::vector<QString> combineSingleCharsToWords(const std::vector<QString>& words)
{
    static const int MIN_COMBINE_SIZE = 3;

    std::vector<QString> combinedWords;
    QString combined;

    for (const auto& w : words)
    {
        if (w.size() == 1)
        {
            combined += w;
            continue;
        }

        if (combined.size() >= MIN_COMBINE_SIZE)
            combinedWords.push_back(combined);

        combined.clear();
    }

    if (combined.size() >= MIN_COMBINE_SIZE)
        combinedWords.push_back(combined);

    return combinedWords;
}

QString SearchUtils::normalizeText(const QString& text)
{
    QLocale locale;
    const QString NFKD = text.normalized(QString::NormalizationForm_KD);
    QString normalized;

    for (const auto ch : NFKD)
    {
        switch (ch.category())
        {
        case QChar::Mark_NonSpacing:
        case QChar::Mark_SpacingCombining:
        case QChar::Mark_Enclosing:
            continue;
        default:
            break;
        }

        normalized.append(ch);
    }

    return locale.toLower(normalized);
}

std::vector<QString> SearchUtils::getNormalizedWords(const QString& text)
{
    if (text.isEmpty())
        return {};

    const QString normalized = SearchUtils::normalizeText(text);
    std::vector<QString> words = getWords(normalized);
    const std::vector<QString>& combinedWords = combineSingleCharsToWords(words);
    words.insert(words.end(), combinedWords.begin(), combinedWords.end());

    return words;
}

std::vector<QString> SearchUtils::getWords(const QString& text)
{
    if (text.isEmpty())
        return {};

    std::vector<QString> words;
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Word, text);
    int startWordPos = 0;

    while (!(boundaryFinder.boundaryReasons() & QTextBoundaryFinder::StartOfItem) && startWordPos != -1)
        startWordPos = boundaryFinder.toNextBoundary();

    while (startWordPos != -1)
    {
        const int endWordPos = boundaryFinder.toNextBoundary();

        Q_ASSERT(endWordPos != -1);
        if (endWordPos == -1)
            break;

        const QString word = text.sliced(startWordPos, endWordPos - startWordPos);
        words.push_back(word);

        startWordPos = boundaryFinder.toNextBoundary();
        while (!(boundaryFinder.boundaryReasons() & QTextBoundaryFinder::StartOfItem) && startWordPos != -1)
            startWordPos = boundaryFinder.toNextBoundary();
    }

    return words;
}

SearchUtils::SearchUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

SearchUtils::~SearchUtils()
{
    removeModels();
}

void SearchUtils::removeModels()
{
    if (mSearchPostFeedModelId >= 0)
    {
        mSkywalker->removeSearchPostFeedModel(mSearchPostFeedModelId);
        mSearchPostFeedModelId = -1;
    }

    if (mSearchUsersModelId >= 0) {
        mSkywalker->removeAuthorListModel(mSearchUsersModelId);
        mSearchUsersModelId = -1;
    }

    if (mSearchFeedsModelId >= 0) {
        mSkywalker->removeFeedListModel(mSearchFeedsModelId);
        mSearchFeedsModelId = -1;
    }
}

void SearchUtils::setAuthorTypeaheadList(const BasicProfileList& list)
{
    mAuthorTypeaheadList = list;
    emit authorTypeaheadListChanged();
}

void SearchUtils::setSearchPostsInProgress(bool inProgress)
{
    if (inProgress != mSearchPostsInProgress)
    {
        mSearchPostsInProgress = inProgress;
        emit searchPostsInProgressChanged();
    }
}

void SearchUtils::setSearchActorsInProgress(bool inProgress)
{
    if (inProgress != mSearchActorsInProgress)
    {
        mSearchActorsInProgress = inProgress;
        emit searchActorsInProgressChanged();
    }
}

void SearchUtils::setSearchFeedsInProgress(bool inProgress)
{
    if (inProgress != mSearchFeedsInProgress)
    {
        mSearchFeedsInProgress = inProgress;
        emit searchFeedsInProgressChanged();
    }
}

void SearchUtils::addAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasicList& profileViewBasicList)
{
    if (profileViewBasicList.empty())
        return;

    std::unordered_set<QString> alreadyFoundDids;

    for (const auto& basicProfile : mAuthorTypeaheadList)
        alreadyFoundDids.insert(basicProfile.getDid());

    for (const auto& profile : profileViewBasicList)
    {
        if (alreadyFoundDids.count(profile->mDid))
            continue;

        BasicProfile basicProfile(profile.get());
        mAuthorTypeaheadList.append(basicProfile.nonVolatileCopy());
    }

    emit authorTypeaheadListChanged();
}

void SearchUtils::searchAuthorsTypeahead(const QString& typed, int limit)
{
    localSearchAuthorsTypeahead(typed, limit);

    if (mAuthorTypeaheadList.size() >= limit)
        return;

    bskyClient()->searchActorsTypeahead(typed, limit - mAuthorTypeaheadList.size(),
        [this, presence=getPresence()](auto searchOutput){
            if (!presence)
                return;

            addAuthorTypeaheadList(searchOutput->mActors);
        },
        [presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Type ahead search failed:" << error << " - " << msg;
        });
}

void SearchUtils::localSearchAuthorsTypeahead(const QString& typed, int limit)
{
    const IndexedProfileStore& following = mSkywalker->getUserFollows();
    const std::unordered_set<const BasicProfile*> profiles = following.findProfiles(typed, limit);
    BasicProfileList profileList;

    for (const auto* profile : profiles)
        profileList.append(*profile);

    setAuthorTypeaheadList(profileList);
}

void SearchUtils::searchPosts(const QString& text, int maxPages, int minEntries, const QString& cursor)
{
    qDebug() << "Search posts:" << text << "cursor:" << cursor << "max pages:"
             << maxPages << "min entries:" << minEntries;

    if (text.isEmpty())
        return;

    if (mSearchPostsInProgress)
    {
        qDebug() << "Search posts still in progress";
        return;
    }

    setSearchPostsInProgress(true);
    bskyClient()->searchPosts(text, {}, mSkywalker->makeOptionalCursor(cursor),
        [this, presence=getPresence(), text, maxPages, minEntries, cursor](auto feed){
            if (!presence)
                return;

            setSearchPostsInProgress(false);
            auto& model = *getSearchPostFeedModel();

            int added = cursor.isEmpty() ?
                            model.setFeed(std::move(feed)) :
                            model.addFeed(std::move(feed));

            // Due to content filtering the feed may go empty
            int entriesToAdd = minEntries - added;

            if (entriesToAdd > 0)
                getNextPageSearchPosts(text, maxPages - 1, entriesToAdd);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            setSearchPostsInProgress(false);
            qDebug() << "searchPosts failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getNextPageSearchPosts(const QString& text, int maxPages, int minEntries)
{
    qDebug() << "Get next page search posts:" << "max pages:" << maxPages
             << "min entries:" << minEntries;

    if (mSearchPostsInProgress)
    {
        qDebug() << "Search posts still in progress";
        return;
    }

    if (maxPages <= 0)
    {
        // Protection against infinite loop.
        qWarning() << "Maximum pages reached!";
        return;
    }

    const auto& model = *getSearchPostFeedModel();
    const auto& cursor = model.getCursorNextPage();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    searchPosts(text, maxPages, minEntries, cursor);
}

void SearchUtils::searchActors(const QString& text, const QString& cursor)
{
    qDebug() << "Search actors:" << text << "cursor:" << cursor;

    if (text.isEmpty())
        return;

    if (mSearchActorsInProgress)
    {
        qDebug() << "Search actors still in progress";
        return;
    }

    setSearchActorsInProgress(true);
    bskyClient()->searchActors(text, {}, mSkywalker->makeOptionalCursor(cursor),
        [this, presence=getPresence(), text, cursor](auto output){
            if (!presence)
                return;

            setSearchActorsInProgress(false);
            auto* model = getSearchUsersModel();

            if (cursor.isEmpty())
                model->clear();

            model->addAuthors(std::move(output->mActors), output->mCursor.value_or(""));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            setSearchActorsInProgress(false);
            qDebug() << "searchActors failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getNextPageSearchActors(const QString& text)
{
    qDebug() << "Get next page search actors";

    if (mSearchActorsInProgress)
    {
        qDebug() << "Search actors still in progress";
        return;
    }

    auto* model = getSearchUsersModel();
    const auto& cursor = model->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    searchActors(text, cursor);
}

void SearchUtils::searchFeeds(const QString& text, const QString& cursor)
{
    qDebug() << "Search feeds:" << text << "cursor:" << cursor;

    if (mSearchFeedsInProgress)
    {
        qDebug() << "Search feeds still in progress";
        return;
    }

    setSearchFeedsInProgress(true);
    bskyClient()->getPopularFeedGenerators(text, 20, mSkywalker->makeOptionalCursor(cursor),
        [this, presence=getPresence(), cursor](auto output){
            if (!presence)
                return;

            setSearchFeedsInProgress(false);
            auto& model = *getSearchFeedsModel();

            if (cursor.isEmpty())
                model.clear();

            model.addFeeds(std::move(output->mFeeds), output->mCursor.value_or(""));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            setSearchFeedsInProgress(false);
            qDebug() << "searchFeeds failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

Q_INVOKABLE void SearchUtils::getNextPageSearchFeeds(const QString& text)
{
    qDebug() << "Get next page search feeds:" << text;

    if (mSearchFeedsInProgress)
    {
        qDebug() << "Search feeds still in progress";
        return;
    }

    auto& model = *getSearchFeedsModel();
    const auto& cursor = model.getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    searchFeeds(text, cursor);
}

void SearchUtils::legacySearchPosts(const QString& text)
{
    qDebug() << "Legacy search posts:" << text;

    if (mSearchPostsInProgress)
    {
        qDebug() << "Search posts still in progress";
        return;
    }

    setSearchPostsInProgress(true);
    bskyClient()->legacySearchPosts(text,
        [this, presence=getPresence()](auto feed){
            if (!presence)
                return;

            getPosts(feed->mUris);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            setSearchPostsInProgress(false);
            qDebug() << "searchPosts failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::legacySearchActors(const QString& text)
{
    qDebug() << "Legacy search actors:" << text;

    if (mSearchActorsInProgress)
    {
        qDebug() << "Search posts still in progress";
        return;
    }

    setSearchActorsInProgress(true);
    bskyClient()->legacySearchActors(text,
        [this, presence=getPresence()](auto feed){
            if (!presence)
                return;

            getProfiles(feed->mDids);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            setSearchActorsInProgress(false);
            qDebug() << "searchactors failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getPosts(const std::vector<QString>& uris)
{
    qDebug() << "Get posts:" << uris.size();

    if (uris.empty())
    {
        setSearchPostsInProgress(false);
        auto* model = getSearchPostFeedModel();
        model->clear();
        return;
    }

    // Truncate the list. MAX is enough for the legacy search.
    const int count = std::min(int(uris.size()), bskyClient()->MAX_URIS_GET_POSTS);
    const std::vector<QString> uriList(uris.begin(), uris.begin() + count);

    bskyClient()->getPosts(uriList,
        [this, presence=getPresence()](auto postViewList)
        {
            if (!presence)
                return;

            setSearchPostsInProgress(false);
            auto output = std::make_unique<ATProto::AppBskyFeed::SearchPostsOutput>();
            output->mPosts = std::move(postViewList);
            auto* model = getSearchPostFeedModel();
            model->setFeed(std::move(output));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg)
        {
            if (!presence)
                return;

            setSearchPostsInProgress(false);
            qWarning() << "Failed to get posts:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getProfiles(const std::vector<QString>& users)
{
    qDebug() << "Get profiles:" << users.size();

    if (users.empty())
    {
        setSearchActorsInProgress(false);
        auto* model = getSearchUsersModel();
        model->clear();
        return;
    }

    // Truncate the list. MAX is enough for the legacy search.
    const int count = std::min(int(users.size()), bskyClient()->MAX_IDS_GET_PROFILES);
    const std::vector<QString> userList(users.begin(), users.begin() + count);

    bskyClient()->getProfiles(userList,
        [this, presence=getPresence()](auto profilesViewDetailedList)
        {
            if (!presence)
                return;

            setSearchActorsInProgress(false);
            ATProto::AppBskyActor::ProfileViewList profileViewList;

            for (auto& profileViewDetailed : profilesViewDetailedList)
            {
                auto profileView = std::make_unique<ATProto::AppBskyActor::ProfileView>();
                profileView->mDid = std::move(profileViewDetailed->mDid);
                profileView->mHandle = std::move(profileViewDetailed->mHandle);
                profileView->mDisplayName = std::move(profileViewDetailed->mDisplayName);
                profileView->mAvatar = std::move(profileViewDetailed->mAvatar);
                profileView->mDescription = std::move(profileViewDetailed->mDescription);
                profileView->mIndexedAt = std::move(profileViewDetailed->mIndexedAt);
                profileView->mViewer = std::move(profileViewDetailed->mViewer);
                profileView->mLabels = std::move(profileViewDetailed->mLabels);

                profileViewList.push_back(std::move(profileView));
            }

            auto* model = getSearchUsersModel();
            model->clear();
            model->addAuthors(std::move(profileViewList), "");
        },
        [this, presence=getPresence()](const QString& error, const QString& msg)
        {
            if (!presence)
                return;

            setSearchActorsInProgress(false);
            qWarning() << "Failed to get posts:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

SearchPostFeedModel* SearchUtils::getSearchPostFeedModel()
{
    Q_ASSERT(mSkywalker);

    if (mSearchPostFeedModelId < 0)
        mSearchPostFeedModelId = mSkywalker->createSearchPostFeedModel();

    return mSkywalker->getSearchPostFeedModel(mSearchPostFeedModelId);
}

AuthorListModel* SearchUtils::getSearchUsersModel()
{
    Q_ASSERT(mSkywalker);

    if (mSearchUsersModelId < 0)
    {
        mSearchUsersModelId = mSkywalker->createAuthorListModel(
            AuthorListModel::Type::AUTHOR_LIST_SEARCH_RESULTS, "");
    }

    return mSkywalker->getAuthorListModel(mSearchUsersModelId);
}

FeedListModel* SearchUtils::getSearchFeedsModel()
{
    Q_ASSERT(mSkywalker);

    if (mSearchFeedsModelId < 0)
        mSearchFeedsModelId = mSkywalker->createFeedListModel();

    return mSkywalker->getFeedListModel(mSearchFeedsModelId);
}

void SearchUtils::clearAllSearchResults()
{
    mAuthorTypeaheadList.clear();

    if (mSearchPostFeedModelId >= 0)
    {
        Q_ASSERT(mSkywalker);
        auto* model = mSkywalker->getSearchPostFeedModel(mSearchPostFeedModelId);
        model->clear();
    }

    if (mSearchUsersModelId >= 0)
    {
        Q_ASSERT(mSkywalker);
        auto* model = mSkywalker->getAuthorListModel(mSearchUsersModelId);
        model->clear();
    }

    if (mSearchFeedsModelId >= 0)
    {
        Q_ASSERT(mSkywalker);
        auto* model = mSkywalker->getFeedListModel(mSearchFeedsModelId);
        model->clear();
    }
}

}
