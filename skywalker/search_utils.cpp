// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "search_utils.h"
#include "author_cache.h"
#include "skywalker.h"
#include "utils.h"
#include <QCollator>
#include <QTextBoundaryFinder>

namespace Skywalker {

static constexpr int MAX_LAST_SEARCHES = 25;
static constexpr int MAX_LAST_PROFILE_SEARCHES = 10;
static constexpr int MAX_TRENDING_TOPICS = 10;
static constexpr int MAX_SUGGESTIONS = 10;
static constexpr char const* USER_ME = "me";

std::vector<QString> SearchUtils::combineSingleCharsToWords(const std::vector<QString>& words)
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

int SearchUtils::normalizedCompare(const QString& lhs, const QString& rhs)
{
    const QString cleanedLhs = UnicodeFonts::removeEmojis(lhs).trimmed();
    const QString cleanedRhs = UnicodeFonts::removeEmojis(rhs).trimmed();

    const int result = QCollator::defaultCompare(normalizeText(cleanedLhs), normalizeText(cleanedRhs));

    if (result != 0)
        return result;

    return QCollator::defaultCompare(lhs, rhs);
}

std::vector<QString> SearchUtils::getNormalizedWords(const QString& text)
{
    if (text.isEmpty())
        return {};

    const QString normalized = SearchUtils::normalizeText(text);
    std::vector<QString> words = getWords(normalized);
    const std::vector<QString> combinedWords = combineSingleCharsToWords(words);
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

    if (mSuggestedFeedsModelId >= 0) {
        mSkywalker->removeFeedListModel(mSuggestedFeedsModelId);
        mSuggestedFeedsModelId = -1;
    }

    if (mSuggestedStarterPacksModelId >= 0) {
        mSkywalker->removeStarterPackListModel(mSuggestedStarterPacksModelId);
        mSuggestedStarterPacksModelId = -1;
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

void SearchUtils::setLastSearchedProfiles(const BasicProfileList& list)
{
    mLastSearchedProfiles = list;
    emit lastSearchedProfilesChanged();
}

void SearchUtils::addAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasic::List& profileViewBasicList, const IProfileMatcher& matcher)
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

    const auto user = getSkywalker()->getUser();
    const QString& handle = user.getHandle();
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

    auto& model = *getSearchPostFeedModel(sortOrder);

    if (model.isGetFeedInProgress())
    {
        qDebug() << "Search posts still in progress";
        return;
    }

    const auto searchText = preProcessSearchText(text);
    const auto authorId = (author == USER_ME) ? mSkywalker->getUserDid() : author;
    const auto mentionsId = (mentions == USER_ME) ? mSkywalker->getUserDid() : mentions;
    const std::optional<QDateTime> sinceParam = setSince ? std::optional<QDateTime>{since.toUTC()} : std::optional<QDateTime>{};
    const std::optional<QDateTime> untilParam = setUntil ? std::optional<QDateTime>{until.toUTC()} : std::optional<QDateTime>{};

    model.setGetFeedInProgress(true);

    bskyClient()->searchPosts(searchText, {}, Utils::makeOptionalString(cursor),
        Utils::makeOptionalString(sortOrder), Utils::makeOptionalString(authorId),
        Utils::makeOptionalString(mentionsId), sinceParam, untilParam,
        Utils::makeOptionalString(language),
        [this, presence=getPresence(), searchText, sortOrder, author, mentions, since, setSince,
         until, setUntil, language, maxPages, minEntries, cursor](auto feed){
            if (!presence)
                return;

            auto& model = *getSearchPostFeedModel(sortOrder);
            model.setGetFeedInProgress(false);

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

            auto& model = *getSearchPostFeedModel(sortOrder);
            model.setGetFeedInProgress(false);
            model.setFeedError(msg);
            qDebug() << "searchPosts failed:" << error << " - " << msg;
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

    const auto& model = *getSearchPostFeedModel(sortOrder);

    if (model.isGetFeedInProgress())
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

    auto* model = getSearchUsersModel();

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Search actors still in progress";
        return;
    }

    const auto searchText = preProcessSearchText(text);

    model->setGetFeedInProgress(true);
    bskyClient()->searchActors(searchText, {}, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), searchText, cursor](auto output){
            if (!presence)
                return;

            auto* model = getSearchUsersModel();
            model->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                model->clear();

            model->addAuthors(std::move(output->mActors), output->mCursor.value_or(""));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto* model = getSearchUsersModel();
            model->setGetFeedInProgress(false);

            qDebug() << "searchActors failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getNextPageSearchActors(const QString& text)
{
    qDebug() << "Get next page search actors";
    auto* model = getSearchUsersModel();

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Search actors still in progress";
        return;
    }

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
    auto* model = getSearchSuggestedUsersModel();

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Search suggested actors still in progress";
        return;
    }

    const QString& did = mSkywalker->getUserDid();
    const QStringList langs = mSkywalker->getUserSettings()->getContentLanguages(did);

    model->setGetFeedInProgress(true);
    bskyClient()->getSuggestions(MAX_SUGGESTIONS, Utils::makeOptionalString(cursor), langs,
        [this, presence=getPresence(), cursor](auto output){
            if (!presence)
                return;

            auto* model = getSearchSuggestedUsersModel();
            model->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                model->clear();

            model->addAuthors(std::move(output->mActors), output->mCursor.value_or(""));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto* model = getSearchSuggestedUsersModel();
            model->setGetFeedInProgress(false);

            qDebug() << "getSuggestedActors failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getNextPageSuggestedActors()
{
    qDebug() << "Get next page suggested actors";
    auto* model = getSearchSuggestedUsersModel();

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Search suggested actors still in progress";
        return;
    }

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
    auto* model = getSearchSuggestedUsersModel();

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Search suggested follows still in progress";
        return;
    }

    const QString& did = mSkywalker->getUserDid();
    const QStringList langs = mSkywalker->getUserSettings()->getContentLanguages(did);

    model->setGetFeedInProgress(true);
    bskyClient()->getSuggestedFollows(user, langs,
        [this, presence=getPresence()](auto output){
            if (!presence)
                return;

            auto* model = getSearchSuggestedUsersModel();
            model->setGetFeedInProgress(false);
            model->clear();

            if (output->mSuggestions.size() > MAX_SUGGESTIONS)
                output->mSuggestions.resize(MAX_SUGGESTIONS);

            model->addAuthors(std::move(output->mSuggestions), "");
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto* model = getSearchSuggestedUsersModel();
            model->setGetFeedInProgress(false);

            qDebug() << "getSuggestedFollows failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::searchFeeds(const QString& text, const QString& cursor)
{
    qDebug() << "Search feeds:" << text << "cursor:" << cursor;
    auto& model = *getSearchFeedsModel();

    if (model.isGetFeedInProgress())
    {
        qDebug() << "Search feeds still in progress";
        return;
    }

    model.setGetFeedInProgress(true);
    bskyClient()->getPopularFeedGenerators(text, 20, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), cursor](auto output){
            if (!presence)
                return;

            auto& model = *getSearchFeedsModel();
            model.setGetFeedInProgress(false);

            if (cursor.isEmpty())
                model.clear();

            model.addFeeds(std::move(output->mFeeds), output->mCursor.value_or(""));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto& model = *getSearchFeedsModel();
            model.setGetFeedInProgress(false);

            qDebug() << "searchFeeds failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

Q_INVOKABLE void SearchUtils::getNextPageSearchFeeds(const QString& text)
{
    qDebug() << "Get next page search feeds:" << text;
    auto& model = *getSearchFeedsModel();

    if (model.isGetFeedInProgress())
    {
        qDebug() << "Search feeds still in progress";
        return;
    }

    const auto& cursor = model.getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    searchFeeds(text, cursor);
}

void SearchUtils::getSuggestedFeeds()
{
    qDebug() << "Get suggested feeds";
    auto& model = *getSuggestedFeedsModel();

    if (model.isGetFeedInProgress())
    {
        qDebug() << "Search feeds still in progress";
        return;
    }

    model.setGetFeedInProgress(true);
    bskyClient()->getSuggestedFeeds(MAX_SUGGESTIONS,
        [this, presence=getPresence()](auto output){
            if (!presence)
                return;

            auto& model = *getSuggestedFeedsModel();
            model.setGetFeedInProgress(false);
            model.clear();
            model.addFeeds(std::move(output->mFeeds), "");
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto& model = *getSuggestedFeedsModel();
            model.setGetFeedInProgress(false);

            qDebug() << "getSuggestedFeeds failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void SearchUtils::getSuggestedStarterPacks()
{
    qDebug() << "Get suggested starter packs";
    auto& model = *getSuggestedStarterPacksModel();

    if (model.isGetFeedInProgress())
    {
        qDebug() << "Search feeds still in progress";
        return;
    }

    model.setGetFeedInProgress(true);
    bskyClient()->getSuggestedStarterPacks(MAX_SUGGESTIONS,
        [this, presence=getPresence()](auto output){
            if (!presence)
                return;

            auto& model = *getSuggestedStarterPacksModel();
            model.setGetFeedInProgress(false);
            model.clear();
            model.addStarterPacks(std::move(output->mStarterPacks), "");
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto& model = *getSuggestedStarterPacksModel();
            model.setGetFeedInProgress(false);

            qDebug() << "getSuggestedStarterPacks failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

SearchPostFeedModel* SearchUtils::getSearchPostFeedModel(const QString& sortOrder, const QString& feedName)
{
    Q_ASSERT(mSkywalker);

    if (!mSearchPostFeedModelId.contains(sortOrder))
        mSearchPostFeedModelId[sortOrder] = mSkywalker->createSearchPostFeedModel(feedName);

    auto* model = mSkywalker->getSearchPostFeedModel(mSearchPostFeedModelId[sortOrder]);

    if (mOVerrideAdultVisibility != QEnums::CONTENT_VISIBILITY_SHOW)
        model->setOverrideAdultVisibility(mOVerrideAdultVisibility);
    else
        model->clearOverrideAdultVisibility();

    return model;
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

FeedListModel* SearchUtils::getSuggestedFeedsModel()
{
    Q_ASSERT(mSkywalker);

    if (mSuggestedFeedsModelId < 0)
        mSuggestedFeedsModelId = mSkywalker->createFeedListModel();

    return mSkywalker->getFeedListModel(mSuggestedFeedsModelId);
}

StarterPackListModel* SearchUtils::getSuggestedStarterPacksModel()
{
    Q_ASSERT(mSkywalker);

    if (mSuggestedStarterPacksModelId < 0)
        mSuggestedStarterPacksModelId = mSkywalker->createStarterPackListModel();

    return mSkywalker->getStarterPackListModel(mSuggestedStarterPacksModelId);
}

void SearchUtils::clearAllSearchResults()
{
    Q_ASSERT(mSkywalker);
    mAuthorTypeaheadList.clear();

    for (const auto& [_, modelId] : mSearchPostFeedModelId)
    {
        auto* model = mSkywalker->getSearchPostFeedModel(modelId);
        model->clear();
    }

    if (mSearchUsersModelId >= 0)
    {
        auto* model = mSkywalker->getAuthorListModel(mSearchUsersModelId);
        model->clear();
    }

    if (mSearchSuggestedUsersModelId >= 0)
    {
        auto* model = mSkywalker->getAuthorListModel(mSearchSuggestedUsersModelId);
        model->clear();
    }

    if (mSearchFeedsModelId >= 0)
    {
        auto* model = mSkywalker->getFeedListModel(mSearchFeedsModelId);
        model->clear();
    }

    if (mSuggestedFeedsModelId >= 0)
    {
        auto* model = mSkywalker->getFeedListModel(mSuggestedFeedsModelId);
        model->clear();
    }

    if (mSuggestedStarterPacksModelId >= 0)
    {
        auto* model = mSkywalker->getStarterPackListModel(mSuggestedStarterPacksModelId);
        model->clear();
    }
}

QStringList SearchUtils::getLastSearches() const
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    return mSkywalker->getUserSettings()->getLastSearches(did);
}

static void updateLastSearches(QStringList& lastSearches, const QString& search, int maxSearches)
{
    for (auto it = lastSearches.cbegin(); it != lastSearches.cend(); ++it)
    {
        if (*it == search)
        {
            lastSearches.erase(it);
            break;
        }
    }

    while (lastSearches.size() >= maxSearches)
        lastSearches.pop_back();

    lastSearches.push_front(search);
}

void SearchUtils::addLastSearch(const QString& search)
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList lastSearches = settings->getLastSearches(did);
    updateLastSearches(lastSearches, search, MAX_LAST_SEARCHES);
    settings->setLastSearches(did, lastSearches);
}

void SearchUtils::removeLastSearch(const QString& search)
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList lastSearches = settings->getLastSearches(did);
    lastSearches.removeOne(search);
    settings->setLastSearches(did, lastSearches);
}

QStringList SearchUtils::getLastProfileSearches() const
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    return mSkywalker->getUserSettings()->getLastProfileSearches(did);
}

void SearchUtils::addLastSearchedProfile(const BasicProfile& profile)
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList lastSearches = settings->getLastProfileSearches(did);
    updateLastSearches(lastSearches, profile.getDid(), MAX_LAST_PROFILE_SEARCHES);
    settings->setLastProfileSearches(did, lastSearches);
    AuthorCache::instance().put(profile);
    initLastSearchedProfiles();
}

void SearchUtils::removeLastSearchedProfile(const QString& profileDid)
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList lastSearches = settings->getLastProfileSearches(did);
    lastSearches.removeOne(profileDid);
    settings->setLastProfileSearches(did, lastSearches);
    initLastSearchedProfiles();
}

void SearchUtils::clearLastSearches()
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    settings->setLastSearches(did, {});
    settings->setLastProfileSearches(did, {});
    setLastSearchedProfiles({});
}

void SearchUtils::initLastSearchedProfiles(bool resolveDids)
{
    qDebug() << "Init last searched profiles";

    const QStringList lastDids = getLastProfileSearches();

    if (lastDids.empty())
    {
        qDebug() << "No profiles";
        setLastSearchedProfiles({});
        return;
    }

    std::vector<QString> unresolvedDids;
    BasicProfileList profiles;

    for (const auto& did : lastDids)
    {
        auto* profile = AuthorCache::instance().get(did);

        if (profile)
        {
            qDebug() << "Add profile:" << profile->getHandle();
            profiles.push_back(*profile);
        }
        else
        {
            qDebug() << "Unresolved did:" << did;
            unresolvedDids.push_back(did);
        }
    }

    setLastSearchedProfiles(profiles);

    if (unresolvedDids.empty() || !resolveDids)
        return;

    bskyClient()->getProfiles(unresolvedDids,
        [this, presence=getPresence()](auto profilesViewDetailedList){
            if (!presence)
                return;

            for (const auto& profileViewDetailed : profilesViewDetailedList)
            {
                BasicProfile profile(profileViewDetailed);
                AuthorCache::instance().put(profile);
            }

            // There could still be unresolved DIDs when an account gets de-activated.
            initLastSearchedProfiles(false);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "initLastSearchedProfiles failed:" << error << " - " << msg;
        });
}

TrendingTopicListModel& SearchUtils::createTrendingTopicsListModel()
{
    if (!mTrendingTopicsListModel)
    {
        Q_ASSERT(mSkywalker);
        mTrendingTopicsListModel = std::make_unique<TrendingTopicListModel>(*mSkywalker->getMutedWords(), this);
        emit trendingTopicsListModelChanged();
    }

    return *mTrendingTopicsListModel;
}

void SearchUtils::getTrendingTopics()
{
    qDebug() << "Get trending topics";
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    const int limit = std::max(MAX_TRENDING_TOPICS, ATProto::Client::MAX_TRENDS);

    bskyClient()->getTrends(limit,
        [this, presence=getPresence()](auto output){
            if (!presence)
                return;

            auto& model = createTrendingTopicsListModel();
            model.clear();
            model.addTopics(output->mTrends, MAX_TRENDING_TOPICS);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "getTrendingTopics failed:" << error << " - " << msg;
        });
}

void SearchUtils::setOverrideAdultVisibility(QEnums::ContentVisibility visibility)
{
    if (visibility != mOVerrideAdultVisibility)
    {
        mOVerrideAdultVisibility = visibility;
        emit overrideAdultVisibilityChanged();
    }
}

SearchFeed SearchUtils::createSearchFeed(
    const QString& searchQuery, const QString& authorHandle, const QString& mentionsHandle,
    QDateTime since, QDateTime until, const QString& language) const
{
    const auto feed = SearchFeed(searchQuery, authorHandle, mentionsHandle, since, until, language);
    qDebug() << "Search feed:" << feed.getSearchQuery() << "author:" << feed.getAuthorHandle() <<
        "mention:" << feed.getMentionHandle() << "since:" << feed.getSince() << "until:" << feed.getUntil() <<
        "lang:" << feed.getLanguage();
    return feed;
}

}
