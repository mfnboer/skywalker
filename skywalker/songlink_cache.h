// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "songlink_links.h"
#include <QCache>
#include <QUrl>

namespace Skywalker {

class SonglinkCache
{
public:
    class Entry : public QObject
    {
    public:
        Entry() = default;
        Entry(const SonglinkLinks& links);

        const SonglinkLinks& getLinks() const { return mLinks; }

    private:
        SonglinkLinks mLinks;
    };

    static SonglinkCache& instance();

    void put(const QUrl& url, const SonglinkLinks& links);
    const SonglinkLinks* get(const QUrl& url) const;
    bool contains(const QUrl& url) const;

private:
    QCache<QUrl, Entry> mCache{50};

    static std::unique_ptr<SonglinkCache> sInstance;
};

}
