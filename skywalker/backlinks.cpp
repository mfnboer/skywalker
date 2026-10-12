// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "backlinks.h"
#include "skywalker.h"
#include "utils.h"

namespace Skywalker {

static constexpr int BACKLINKS_PAGE_SIZE = 25;

Backlinks::Backlinks(Constellation& constellation, QObject* parent) :
    WrappedSkywalker(parent),
    mConstellation(constellation)
{
}

void Backlinks::getBlockedByAuthorList(const QString& atId, const QString& cursor, int modelId)
{
    getAuthorList("app.bsky.graph.block:subject", atId, cursor, modelId);
}

void Backlinks::getVerifiedByAuthorList(const QString& atId, const QString& cursor, int modelId)
{
    getAuthorList("app.bsky.graph.verification:subject", atId, cursor, modelId);
}

void Backlinks::getMemberOfListList(const QString& atId, QEnums::ListPurpose purpose, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    qDebug() << "Get member of list list:" << atId << "purpose:" << purpose << "maxPages:" << maxPages << "minEntries:" << minEntries << "cursor" << cursor << "modelId:" << modelId;

    Q_ASSERT(mSkywalker);
    if (!mSkywalker)
        return;

    auto* model = mSkywalker->getListListModel(modelId);

    if (model)
        model->setGetFeedInProgress(true);

    mConstellation.getBackLinks(atId, "app.bsky.graph.listitem:subject", {}, BACKLINKS_PAGE_SIZE, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), purpose, maxPages, minEntries, modelId](Constellation::Backlinks::SharedPtr backlinks){
            if (!presence)
                return;

            std::vector<QString> uris;

            for (const auto& record : backlinks->mRecords)
            {
                const ATProto::ATUri atUri(record->mDid, record->mCollection, record->mRkey);

                if (atUri.isValid())
                    uris.push_back(atUri.toString());
                else
                    qWarning() << "Invalid at-uri:" << record->mDid << record->mCollection << record->mRkey;
            }

            getListsFromListItems(uris, purpose, maxPages, minEntries, backlinks->mCursor.value_or(""), modelId);
        },
        [this, presence=getPresence(), modelId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "getMemberOfListList failed:" << error << "-" << msg;
            auto* model = mSkywalker->getListListModel(modelId);

            if (model)
                model->setGetFeedInProgress(false);

            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Backlinks::getAuthorList(const QString& source, const QString& atId, const QString& cursor, int modelId)
{
    Q_ASSERT(mSkywalker);
    if (!mSkywalker)
        return;

    auto* model = mSkywalker->getAuthorListModel(modelId);

    if (model)
        model->setGetFeedInProgress(true);

    mConstellation.getBackLinks(atId, source, {}, BACKLINKS_PAGE_SIZE, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), modelId](Constellation::Backlinks::SharedPtr backlinks){
            if (!presence)
                return;

            std::vector<QString> dids;

            for (const auto& record : backlinks->mRecords)
                dids.push_back(record->mDid);

            getProfiles(dids, backlinks->mCursor.value_or(""), modelId);
        },
        [this, presence=getPresence(), source, modelId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "getAuthorList failed:" << source << error << "-" << msg;
            auto* model = mSkywalker->getAuthorListModel(modelId);

            if (model)
                model->setGetFeedInProgress(false);

            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Backlinks::getProfiles(const std::vector<QString>& dids, const QString& cursor, int modelId)
{
    Q_ASSERT(mSkywalker);
    if (!mSkywalker)
        return;

    if (dids.empty())
    {
        qDebug() << "Not profiles to get, modelId:" << modelId;
        auto* model = mSkywalker->getAuthorListModel(modelId);

        if (model)
        {
            model->setGetFeedInProgress(false);
            model->setCursor(cursor);
        }

        return;
    }

    bskyClient()->getProfiles(dids,
        [this, presence=getPresence(), cursor, modelId](auto profileDetailedList){
            if (!presence)
                return;

            auto* model = mSkywalker->getAuthorListModel(modelId);

            if (model)
            {
                model->setGetFeedInProgress(false);
                model->addAuthors(std::move(profileDetailedList), cursor);
            }
        },
        [this, presence=getPresence(), modelId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "getProfiles failed:" << error << "-" << msg;
            auto* model = mSkywalker->getAuthorListModel(modelId);

            if (model)
                model->setGetFeedInProgress(false);

            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Backlinks::getListsFromListItems(const std::vector<QString>& listItemUris, QEnums::ListPurpose purpose, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    qDebug() << "Get lists from list items:" << listItemUris.size() << "purpose:" << purpose << "maxPages:" << maxPages << "minEntries:" << minEntries << "cursor:" << cursor;

    Q_ASSERT(mSkywalker);
    if (!mSkywalker)
        return;

    repoMaster()->getRecords<ATProto::AppBskyGraph::ListItem>(listItemUris,
        [this, presence=getPresence(), purpose, maxPages, minEntries, cursor, modelId](ATProto::AppBskyGraph::ListItem::List listItems){
            if (!presence)
                return;

            auto* model = mSkywalker->getListListModel(modelId);

            if (!model)
                return;

            std::unordered_set<QString> seenUris;
            std::vector<QString> uris;
            uris.reserve(listItems.size());

            for (const auto& listItem : listItems)
            {
                if (!seenUris.contains(listItem->mList) && !model->containsList(listItem->mList))
                {
                    uris.push_back(listItem->mList);
                    seenUris.insert(listItem->mList);
                }
            }

            getLists(uris, purpose, maxPages, minEntries, cursor, modelId);
        });
}

void Backlinks::getLists(const std::vector<QString>& listUris, QEnums::ListPurpose purpose, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    qDebug() << "Get lists:" << listUris.size() << "purpose:" << purpose << "maxPages:" << maxPages << "minEntries:" << minEntries << "cursor:" << cursor;

    Q_ASSERT(mSkywalker);
    if (!mSkywalker)
        return;

    if (listUris.empty())
    {
        qDebug() << "Not lists to get, modelId:" << modelId;
        auto* model = mSkywalker->getListListModel(modelId);

        if (model)
        {
            model->setGetFeedInProgress(false);
            model->setCursor(cursor);
            mSkywalker->getListListNextPage(modelId, {}, maxPages - 1, minEntries);
        }

        return;
    }

    auto todo = std::make_shared<int>(listUris.size());
    auto lists = std::make_shared<ATProto::AppBskyGraph::ListView::List>();

    for (const auto& uri : listUris)
    {
        bskyClient()->getList(uri, 1, {},
            [this, presence=getPresence(), todo, lists, purpose, maxPages, minEntries, cursor, modelId](ATProto::AppBskyGraph::GetListOutput::SharedPtr output){
                if (!presence)
                    return;

                if ((QEnums::ListPurpose)output->mList->mPurpose == purpose)
                    lists->push_back(output->mList);

                if (--*todo > 0)
                    return;

                auto* model = mSkywalker->getListListModel(modelId);

                if (model)
                {
                    model->setGetFeedInProgress(false);
                    const int added = model->addLists(*lists, cursor);
                    const int toAdd = minEntries - added;

                    if (toAdd > 0)
                        mSkywalker->getListListNextPage(modelId, {}, maxPages - 1, toAdd);
                }
            },
            [this, presence=getPresence(), uri, todo, lists, maxPages, minEntries, modelId, cursor](const QString& error, const QString& msg){
                if (!presence)
                    return;

                qWarning() << "getLists failed:" << uri << error << "-" << msg;

                if (--*todo > 0)
                    return;

                auto* model = mSkywalker->getListListModel(modelId);

                if (model)
                {
                    model->setGetFeedInProgress(false);
                    const int added = model->addLists(*lists, cursor);
                    const int toAdd = minEntries - added;

                    if (toAdd > 0)
                        mSkywalker->getListListNextPage(modelId, {}, maxPages - 1, toAdd);
                }
            });
    }
}

ATProto::RepoMaster* Backlinks::repoMaster()
{
    if (!mRepoMaster)
        mRepoMaster = std::make_unique<ATProto::RepoMaster>(*bskyClient());

    return mRepoMaster.get();
}

}