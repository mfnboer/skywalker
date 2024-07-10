// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "search_utils.h"
#include "skywalker.h"
#include "utils.h"
#include <QTextBoundaryFinder>

namespace Skywalker {

static constexpr int MAX_LAST_SEARCHES = 25;
static constexpr char const* USER_ME = "me";

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
    return ATProto::RichTextMaster::normalizeText(text);
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
    for (const auto& [_, modelId] : mSearchPostFeedModelId)
        mSkywalker->removeSearchPostFeedModel(modelId);

    mSearchPostFeedModelId.clear();

    if (mSearchUsersModelId >= 0) {
        mSkywalker->removeAuthorListModel(mSearchUsersModelId);
        mSearchUsersModelId = -1;
    }

    if (mSearchSuggestedUsersModelId >= 0) {
        mSkywalker->removeAuthorListModel(mSearchSuggestedUsersModelId);
        mSearchSuggestedUsersModelId = -1;
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

void SearchUtils::setHashtagTypeaheadList(const QStringList& list)
{
    if (mHashtagTypeaheadList != list)
    {
        mHashtagTypeaheadList = list;
        emit hashtagTypeaheadListChanged();
    }
}

void SearchUtils::setSearchPostsInProgress(const QString& sortOrder, bool inProgress)
{
    if (inProgress != mSearchPostsInProgress[sortOrder])
    {
        mSearchPostsInProgress[sortOrder] = inProgress;

        if (sortOrder == ATProto::AppBskyFeed::SearchSortOrder::TOP)
            emit searchPostsTopInProgressChanged();
        else if (sortOrder == ATProto::AppBskyFeed::SearchSortOrder::LATEST)
            emit searchPostsLatestInProgressChanged();
        else
            Q_ASSERT(false);
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

void SearchUtils::setSearchSuggestedActorsInProgress(bool inProgress)
{
    if (inProgress != mSearchSuggestedActorsInProgress)
    {
        mSearchSuggestedActorsInProgress = inProgress;
        emit searchSuggestedActorsInProgressChanged();
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

void SearchUtils::addAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasicList& profileViewBasicList, const IProfileMatcher& matcher)
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

        BasicProfile basicProfile(profile);

        if (matcher.match(basicProfile))
            mAuthorTypeaheadList.append(basicProfile);
    }

    emit authorTypeaheadListChanged();
}

void SearchUtils::searchAuthorsTypeahead(const QString& typed, int limit, bool canChatOnly)
{
    const IProfileMatcher* matcher = canChatOnly ?
            &static_cast<IProfileMatcher&>(mCanChatProfileMatcher) :
            &static_cast<IProfileMatcher&>(mAnyProfileMatcher);

    localSearchAuthorsTypeahead(typed, limit, *matcher);

    if (mAuthorTypeaheadList.size() >= limit)
        return;

    bskyClient()->searchActorsTypeahead(typed, limit - mAuthorTypeaheadList.size(),
        [this, presence=getPresence(), matcher](auto searchOutput){
            if (!presence)
                return;

            addAuthorTypeaheadList(searchOutput->mActors, *matcher);
        },
        [presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Type ahead search failed:" << error << " - " << msg;
        });
}

void SearchUtils::searchHashtagsTypeahead(const QString& typed, int limit)
{
    const auto& hashtags = mSkywalker->getUserHashtags();
    auto results = hashtags.find(typed, limit);

    if (results.size() < limit)
    {
        const auto& seenHashtags = mSkywalker->getSeenHashtags();
        const auto& seenResults = seenHashtags.find(typed, limit - results.size(), results);
        results.append(seenResults);
    }

    setHashtagTypeaheadList(results);
}

void SearchUtils::localSearchAuthorsTypeahead(const QString& typed, int limit, const IProfileMatcher& matcher)
{
    const IndexedProfileStore& following = mSkywalker->getUserFollows();
    const std::unordered_set<const BasicProfile*> profiles = following.findProfiles(typed, limit, matcher);
    BasicProfileList profileList;

    for (const auto* profile : profiles)
        profileList.append(*profile);

    setAuthorTypeaheadList(profileList);
}

QString SearchUtils::preProcessSearchText(const QString& text) const
{
    static const QString FROM_ME = "from:me";
    static const auto FROM_ME_SIZE = FROM_ME.length();

    const auto index = text.indexOf(FROM_ME);

    if (index == -1)
        return text;

    if (index > 0 && !text.at(index - 1).isSpace())
        return text;

    if (index < text.length() - FROM_ME_SIZE && !text.at(index + FROM_ME_SIZE).isSpace())
        return text;

    const QString& handle = getSkywalker()->getUser().getHandle();
    const QString newText = text.sliced(0, index + 5) + handle + text.sliced(index + FROM_ME_SIZE);
    qDebug() << text << " --> " << newText;
    return newText;
}

void SearchUtils::searchPosts(const QString& text, const QString& sortOrder, const QString& author,
                              const QString& mentions, const QDateTime& since, bool setSince,
                              const QDateTime& until, bool setUntil, const QString& language,
                              int maxPages, int minEntries, const QString& cursor)
{
    qDebug() << "Search posts:" << text << "order:" << sortOrder << "author:" << author
             << "mentions:" << mentions << "since:" << since << "setSince:" << setSince
             << "until:" << until << "setUntil:" << setUntil << "language:" << language
             << "cursor:" << cursor << "max pages:"
             << maxPages << "min entries:" << minEntries;

    if (text.isEmpty())
        return;

    if (getSearchPostsInProgress(sortOrder))
    {
        qDebug() << "Search posts still in progress";
        return;
    }

    const auto searchText = preProcessSearchText(text);
    const auto authorId = (author == USER_ME) ? mSkywalker->getUserDid() : author;
    const auto mentionsId = (mentions == USER_ME) ? mSkywalker->getUserDid() : mentions;
    const std::optional<QDateTime> sinceParam = setSince ? std::optional<QDateTime>{since.toUTC()} : std::optional<QDateTime>{};
    const std::optional<QDateTime> untilParam = setUntil ? std::optional<QDateTime>{until.toUTC()} : std::optional<QDateTime>{};

    setSearchPostsInProgress(sortOrder, true);
    bskyClient()->searchPosts(searchText, {}, Utils::makeOptionalString(cursor),
        Utils::makeOptionalString(sortOrder), Utils::makeOptionalString(authorId),
        Utils::makeOptionalString(mentionsId), sinceParam, untilParam,
        Utils::makeOptionalString(language),
        [this, presence=getPresence(), searchText, sortOrder, author, mentions, since, setSince,
         until, setUntil, language, maxPages, minEntries, cursor](auto feed){
            if (!presence)
                return;

            setSearchPostsInProgress(sortOrder, false);
            auto& model = *getSearchPostFeedModel(sortOrder);

            int added = cursor.isEmpty() ?
                            model.setFeed(std::move(feed)) :
                            model.addFeed(std::move(feed));

            // Due to content filtering the feed may go empty
            int entriesToAdd = minEntries - added;

            if (entriesToAdd > 0)
                getNextPageSearchPosts(searchText, sortOrder, author, mentions, since, setSince,
                                       until, setUntil, language, maxPages - 1, entriesToAdd);
        },
        [this, presence=getPresence(), sortOrder](const QString& error, const QString& msg){
            if (!presence)
                return;

            setSearchPostsInProgress(sortOrder, false);
            qDebug() << "searchPosts failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getNextPageSearchPosts(const QString& text, const QString& sortOrder,
                                         const QString& author, const QString& mentions,
                                         const QDateTime& since, bool setSince,
                                         const QDateTime& until, bool setUntil,
                                         const QString& language,
                                         int maxPages, int minEntries)
{
    qDebug() << "Get next page search posts:" << text << "order:" << sortOrder << "max pages:" << maxPages
             << "min entries:" << minEntries;

    if (getSearchPostsInProgress(sortOrder))
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

    const auto& model = *getSearchPostFeedModel(sortOrder);
    const auto& cursor = model.getCursorNextPage();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    searchPosts(text, sortOrder, author, mentions, since, setSince, until, setUntil,
                language, maxPages, minEntries, cursor);
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

    const auto searchText = preProcessSearchText(text);

    setSearchActorsInProgress(true);
    bskyClient()->searchActors(searchText, {}, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), searchText, cursor](auto output){
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

void SearchUtils::getSuggestedActors(const QString& cursor)
{
    Q_ASSERT(mSkywalker);
    qDebug() << "Get suggested actors, cursor:" << cursor;

    if (mSearchSuggestedActorsInProgress)
    {
        qDebug() << "Search suggested actors still in progress";
        return;
    }

    const QString& did = mSkywalker->getUserDid();
    const QStringList langs = mSkywalker->getUserSettings()->getContentLanguages(did);

    setSearchSuggestedActorsInProgress(true);
    bskyClient()->getSuggestions({}, Utils::makeOptionalString(cursor), langs,
        [this, presence=getPresence(), cursor](auto output){
            if (!presence)
                return;

            setSearchSuggestedActorsInProgress(false);
            auto* model = getSearchSuggestedUsersModel();

            if (cursor.isEmpty())
                model->clear();

            model->addAuthors(std::move(output->mActors), output->mCursor.value_or(""));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            setSearchSuggestedActorsInProgress(false);
            qDebug() << "getSuggestedActors failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getNextPageSuggestedActors()
{
    qDebug() << "Get next page suggested actors";

    if (mSearchSuggestedActorsInProgress)
    {
        qDebug() << "Search suggested actors still in progress";
        return;
    }

    auto* model = getSearchSuggestedUsersModel();
    const auto& cursor = model->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    getSuggestedActors(cursor);
}

void SearchUtils::getSuggestedFollows(const QString& user)
{
    Q_ASSERT(mSkywalker);
    qDebug() << "Get suggested follows:" << user;

    if (mSearchSuggestedActorsInProgress)
    {
        qDebug() << "Search suggested actors still in progress";
        return;
    }

    const QString& did = mSkywalker->getUserDid();
    const QStringList langs = mSkywalker->getUserSettings()->getContentLanguages(did);

    setSearchSuggestedActorsInProgress(true);
    bskyClient()->getSuggestedFollows(user, langs,
        [this, presence=getPresence()](auto output){
            if (!presence)
                return;

            setSearchSuggestedActorsInProgress(false);
            auto* model = getSearchSuggestedUsersModel();
            model->clear();
            model->addAuthors(std::move(output->mSuggestions), "");
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            setSearchSuggestedActorsInProgress(false);
            qDebug() << "getSuggestedFollows failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
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
    bskyClient()->getPopularFeedGenerators(text, 20, Utils::makeOptionalString(cursor),
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

SearchPostFeedModel* SearchUtils::getSearchPostFeedModel(const QString& sortOrder)
{
    Q_ASSERT(mSkywalker);

    if (!mSearchPostFeedModelId.contains(sortOrder))
        mSearchPostFeedModelId[sortOrder] = mSkywalker->createSearchPostFeedModel();

    return mSkywalker->getSearchPostFeedModel(mSearchPostFeedModelId[sortOrder]);
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

AuthorListModel* SearchUtils::getSearchSuggestedUsersModel()
{
    Q_ASSERT(mSkywalker);

    if (mSearchSuggestedUsersModelId < 0)
    {
        mSearchSuggestedUsersModelId = mSkywalker->createAuthorListModel(
            AuthorListModel::Type::AUTHOR_LIST_SUGGESTIONS, "");
    }

    return mSkywalker->getAuthorListModel(mSearchSuggestedUsersModelId);
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

    for (const auto& [_, modelId] : mSearchPostFeedModelId)
    {
        Q_ASSERT(mSkywalker);
        auto* model = mSkywalker->getSearchPostFeedModel(modelId);
        model->clear();
    }

    if (mSearchUsersModelId >= 0)
    {
        Q_ASSERT(mSkywalker);
        auto* model = mSkywalker->getAuthorListModel(mSearchUsersModelId);
        model->clear();
    }

    if (mSearchSuggestedUsersModelId >= 0)
    {
        Q_ASSERT(mSkywalker);
        auto* model = mSkywalker->getAuthorListModel(mSearchSuggestedUsersModelId);
        model->clear();
    }

    if (mSearchFeedsModelId >= 0)
    {
        Q_ASSERT(mSkywalker);
        auto* model = mSkywalker->getFeedListModel(mSearchFeedsModelId);
        model->clear();
    }
}

QStringList SearchUtils::getLastSearches() const
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    return mSkywalker->getUserSettings()->getLastSearches(did);
}

void SearchUtils::addLastSearch(const QString& search)
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList lastSearches = settings->getLastSearches(did);

    for (auto it = lastSearches.cbegin(); it != lastSearches.cend(); ++it)
    {
        if (*it == search)
        {
            lastSearches.erase(it);
            break;
        }
    }

    while (lastSearches.size() >= MAX_LAST_SEARCHES)
        lastSearches.pop_back();

    lastSearches.push_front(search);
    settings->setLastSearches(did, lastSearches);
}

void SearchUtils::clearLastSearches() const
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    settings->setLastSearches(did, {});
}

}
