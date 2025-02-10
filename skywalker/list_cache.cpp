// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "list_cache.h"

namespace Skywalker {

std::unique_ptr<ListCache> ListCache::sInstance;

ListCache& ListCache::instance()
{
    if (!sInstance)
        sInstance = std::unique_ptr<ListCache>(new ListCache);

    return *sInstance;
}

ListCache::Entry::Entry(const ListViewBasic& list) :
    mList(list)
{
}

ListCache::ListCache(QObject* parent) :
    WrappedSkywalker(parent),
    mCache(100)
{
}

void ListCache::clear()
{
    mCache.clear();
}

void ListCache::put(const ListViewBasic& list)
{
    const QString& uri = list.getUri();
    Q_ASSERT(!uri.isEmpty());

    if (uri.isEmpty())
        return;

    auto* entry = new Entry(list);
    mCache.insert(uri, entry);
}

void ListCache::putList(const QString& uri, const std::function<void()>& addedCb)
{
    if (contains(uri))
    {
        qDebug() << "List already in cache:" << uri;
        return;
    }

    if (mFetchingUris.contains(uri))
        return;

    if (!bskyClient())
        return;

    mFetchingUris.insert(uri);

    bskyClient()->getList(uri, 1, {},
        [this, addedCb](auto output){
            const auto& list = output->mList;
            mFetchingUris.erase(list->mUri);
            mFailedUris.erase(list->mUri);
            put(ListViewBasic(list));

            if (addedCb)
                addedCb();
        },
        [this, uri](const QString& error, const QString& msg){
            qDebug() << "puList failed:" << uri << error << " - " << msg;

            if (!mFailedUris.contains(uri))
            {
                mFetchingUris.erase(uri);
                mFailedUris.insert(uri);
            }
            else
            {
                qWarning() << "Failed to get URI for the second time:" << uri << error << " - " << msg;
                // Do not remove from mFetchingUris, so we will not try to get it again
            }
        });
}

const ListViewBasic* ListCache::get(const QString& uri) const
{
    auto* entry = mCache[uri];
    return entry ? &entry->getList() : nullptr;
}

bool ListCache::contains(const QString& uri) const
{
    return get(uri) != nullptr;
}

}
