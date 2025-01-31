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
        mSkywalker->statusMessage(tr("Hide list %1 has more than %2 users").arg(uri).arg(pagesLoaded * LIST_PAGE_SIZE),
                                  QEnums::STATUS_LEVEL_ERROR, 30);
        successCb();
        return;
    }

    auto& profileStore = mLists[uri];

    bskyClient()->getList(uri, LIST_PAGE_SIZE, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), uri, successCb, errorCb, maxPages, pagesLoaded, &profileStore](auto output){
            if (!presence)
                return;

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

            mLists.erase(uri);
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
    mLists.erase(uri);
}

void ListStore::addProfile(const QString& uri, const BasicProfile& profile, const QString& listItemUri)
{
    qDebug() << "Add profile, list:" << uri << "did:" << profile.getDid() << "item:" << listItemUri;
    mLists[uri].add(profile, listItemUri);
}

void ListStore::removeProfile(const QString& uri, const QString& listItemUri)
{
    qDebug() << "Add profile, list:" << uri << "item:" << listItemUri;
    mLists[uri].removeByListItemUri(listItemUri);
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
    for (const auto& [_, profileStore] : mLists)
    {
        if (profileStore.contains(did))
            return true;
    }

    return false;
}

const BasicProfile* ListStore::get(const QString& did) const
{
    for (const auto& [_, profileStore] : mLists)
    {
        const auto* profile = profileStore.get(did);

        if (profile)
            return profile;
    }

    return nullptr;
}

}
