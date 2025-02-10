// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "list_view_include.h"
#include "wrapped_skywalker.h"
#include <QCache>

namespace Skywalker {

class ListCache : public WrappedSkywalker
{
    Q_OBJECT

public:
    class Entry : public QObject
    {
    public:
        Entry() = default;
        Entry(const ListViewBasic& list);

        const ListViewBasic& getList() const { return mList; }

    private:
        ListViewBasic mList;
    };

    static ListCache& instance();

    void clear();
    void put(const ListViewBasic& list);
    void putList(const QString& uri, const std::function<void()>& addedCb = {});
    const ListViewBasic* get(const QString& uri) const;
    bool contains(const QString& uri) const;

private:
    explicit ListCache(QObject* parent = nullptr);

    QCache<QString, Entry> mCache; // key is list uri
    std::unordered_set<QString> mFetchingUris;
    std::unordered_set<QString> mFailedUris;

    static std::unique_ptr<ListCache> sInstance;
};

}
