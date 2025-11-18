// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "list_store.h"
#include "graph_listener.h"
#include "skywalker.h"
#include "utils.h"

namespace Skywalker {

static constexpr int LIST_PAGE_SIZE = 100;

ListStore::ListStore(QObject* parent) :
    WrappedSkywalker(parent)
{
    auto& graphListener = GraphListener::instance();

    connect(&graphListener, &GraphListener::listDeleted, this,
            [this](const QString& uri){
                if (mLists.contains(uri))
                    removeList(uri);
            });

    connect(&graphListener, &GraphListener::userAdded, this,
            [this](const QString& uri, const BasicProfile& profile, const QString& itemUri){
                if (mLists.contains(uri))
                    addProfile(uri, profile, itemUri);
            });

    connect(&graphListener, &GraphListener::userRemoved, this,
            [this](const QString& uri, const QString& itemUri){
                if (mLists.contains(uri))
                    removeProfile(uri, itemUri);
            });
}

void ListStore::clear()
{
    mLists.clear();
}

void ListStore::loadList(const QString& uri, const SuccessCb& successCb, const ErrorCb& errorCb,
                            int maxPages, int pagesLoaded, const QString& cursor)
{
    qDebug() << "Load list:" << uri << "maxPages:" << maxPages << "cursor:" << cursor;

    if (maxPages <= 0)
    {
        qWarning() << "Max pages reached";
        const QString name = getListName(uri);

        mSkywalker->showStatusMessage(
            tr("List %1 has more than %2 users").arg(name).arg(pagesLoaded * LIST_PAGE_SIZE),
            QEnums::STATUS_LEVEL_ERROR, 30);
        successCb();
        return;
    }

    auto& profileStore = mLists[uri].mStore;

    bskyClient()->getList(uri, LIST_PAGE_SIZE, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), uri, successCb, errorCb, maxPages, pagesLoaded, &profileStore](auto output){
            if (!presence)
                return;

            qDebug() << "Got page of list:" << uri << output->mList->mName;
            mLists[uri].mList = ListViewBasic(output->mList);

            for (const auto& item : output->mItems)
            {
                const BasicProfile profile(item->mSubject);
                profileStore.add(profile, item->mUri);
            }

            if (output->mCursor)
                loadList(uri, successCb, errorCb, maxPages - 1, pagesLoaded + 1, *output->mCursor);
            else
                successCb();
        },
        [this, presence=getPresence(), uri, errorCb](const QString& error, const QString& msg){
            if (!presence)
                return;

            removeList(uri);
            qWarning() << "loadList failed:" << error << " - " << msg;
            errorCb(error, msg);
        });
}

void ListStore::addList(const QString& uri, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Add list:" << uri;

    if (mLists.contains(uri))
    {
        qWarning() << "List already added:" << uri;
        return;
    }

    loadList(uri,
             [successCb]{ successCb(); },
             [errorCb](const auto& error, const auto& msg){ errorCb(error, msg); });
}

void ListStore::removeList(const QString& uri)
{
    qDebug() << "Remove list:" << uri;

    if (mLists.erase(uri))
        emit listRemoved(uri);
}

void ListStore::addProfile(const QString& uri, const BasicProfile& profile, const QString& listItemUri)
{
    qDebug() << "Add profile, list:" << uri << "did:" << profile.getDid() << "item:" << listItemUri;
    mLists[uri].mStore.add(profile, listItemUri);
}

void ListStore::removeProfile(const QString& uri, const QString& listItemUri)
{
    qDebug() << "Add profile, list:" << uri << "item:" << listItemUri;
    mLists[uri].mStore.removeByListItemUri(listItemUri);
}

bool ListStore::hasList(const QString& uri) const
{
    return mLists.contains(uri);
}

QStringList ListStore::getListUris() const
{
    QStringList uris;

    for (const auto& [uri, _] : mLists)
        uris.push_back(uri);

    return uris;
}

bool ListStore::contains(const QString& did) const
{
    for (const auto& [_, entry] : mLists)
    {
        if (entry.mStore.contains(did))
            return true;
    }

    return false;
}

bool ListStore::contains(const QString& listUri, const QString& did) const
{
    const auto it = mLists.find(listUri);
    return it != mLists.end() ? it->second.mStore.contains(did) : false;
}

QStringList ListStore::getListUrisForDid(const QString& did) const
{
    QStringList uriList;

    for (const auto& [uri, entry] : mLists)
    {
        if (entry.mStore.contains(did))
            uriList.push_back(uri);
    }

    return uriList;
}

const BasicProfile* ListStore::get(const QString& did) const
{
    for (const auto& [_, entry] : mLists)
    {
        const auto* profile = entry.mStore.get(did);

        if (profile)
            return profile;
    }

    return nullptr;
}

QString ListStore::getListName(const QString& uri) const
{
    const auto it = mLists.find(uri);

    if (it == mLists.end() || it->second.mList.getName().isEmpty())
        return uri;

    return it->second.mList.getName();
}

const ListViewBasic& ListStore::getList(const QString& uri) const
{
    static const ListViewBasic NULL_LIST;

    const auto it = mLists.find(uri);
    return it != mLists.end() ? it->second.mList : NULL_LIST;
}

}
