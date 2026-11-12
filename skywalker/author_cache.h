// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include "profile_store.h"
#include "wrapped_skywalker.h"
#include <QCache>
#include <unordered_set>

namespace Skywalker {

class AuthorCache : public WrappedSkywalker
{
    Q_OBJECT

public:
    using AddedCb = std::function<void()>;

    class Entry : public QObject
    {
    public:
        Entry() = default;
        Entry(const BasicProfile& profile);

        const BasicProfile& getAuthor() const { return mAuthor; }

    private:
        BasicProfile mAuthor;
    };

    static AuthorCache& instance();

    void clear();
    void put(const BasicProfile& author);

    // Returns error if fetching profile failed permanently
    QString putProfile(const QString& did, const AddedCb& addedCb = {});

    // Returns error if fetching profile failed permanently
    QString getProfileError(const QString& did) const;

    const BasicProfile* get(const QString& did) const;
    bool contains(const QString& did) const;

    void setUser(const BasicProfile& user);
    void addProfileStore(const IProfileStore* store);

signals:
    void profileAdded(const QString& did);
    void profileFailed(const QString& did);

private:
    explicit AuthorCache(QObject* parent = nullptr);

    const BasicProfile* getFromStores(const QString& did) const;

    QCache<QString, Entry> mCache; // key is did
    std::unordered_set<const IProfileStore*> mProfileStores;
    BasicProfile mUser;
    std::unordered_map<QString, std::vector<AddedCb>> mFetchingDids;
    std::unordered_map<QString, QString> mFailedDids; // did -> error
    std::unordered_map<QString, QString> mPermanentlyFailedDids; // did -> error

    static std::unique_ptr<AuthorCache> sInstance;
};

}
