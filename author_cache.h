// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include "profile_store.h"
#include <QCache>
#include <unordered_set>

namespace Skywalker {

class AuthorCache
{
public:
    class Entry : public QObject
    {
    public:
        Entry() = default;
        Entry(const BasicProfile& profile);

        const BasicProfile& getAuthor() const { return mAuthor; }

    private:
        BasicProfile mAuthor; // non-volatile profile
    };

    static AuthorCache& instance();

    void clear();
    void put(const BasicProfile& author);
    const BasicProfile* get(const QString& did) const;
    bool contains(const QString& did) const;

    const BasicProfile& getUser() const { return mUser; }
    void setUser(const BasicProfile& user);
    void addProfileStore(const IProfileStore* store);

private:
    AuthorCache();

    const BasicProfile* getFromStores(const QString& did) const;

    QCache<QString, Entry> mCache; // key is did
    std::unordered_set<const IProfileStore*> mProfileStores;
    BasicProfile mUser;

    static std::unique_ptr<AuthorCache> sInstance;
};

}
