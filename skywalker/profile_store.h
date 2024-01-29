// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <unordered_map>
#include <unordered_set>
#include <set>

namespace Skywalker {

class IProfileStore
{
public:
    virtual ~IProfileStore() = default;
    virtual bool contains(const QString& did) const = 0;
    virtual const BasicProfile* get(const QString& did) const = 0;
};

class ProfileStore : public IProfileStore
{
public:
    bool contains(const QString& did) const override;
    const BasicProfile* get(const QString& did) const override;
    virtual void add(const BasicProfile& profile);
    virtual void remove(const QString& did);
    virtual void clear();
    size_t size();

private:
    std::unordered_map<QString, BasicProfile> mDidProfileMap;
};

class ProfileListItemStore : public ProfileStore
{
public:
    virtual void add(const BasicProfile& profile) override;
    virtual void remove(const QString& did) override;
    virtual void clear() override;

    void add(const BasicProfile& profile, const QString& listItemUri);
    const QString* getListItemUri(const QString& did) const;
    const QString* getDidByListItemUri(const QString& listItemUri) const;

    const QString& getListUri() const { return mListUri; }
    void setListUri(const QString& uri) { mListUri = uri; }
    bool isListCreated() const { return mListCreated; }
    void setListCreated(bool created) { mListCreated = created; }

private:
    std::unordered_map<QString, QString> mDidListItemUriMap;
    std::unordered_map<QString, QString> mListItemUriDidMap;
    QString mListUri;
    bool mListCreated = false;
};

class IndexedProfileStore : public ProfileStore
{
public:
    virtual void add(const BasicProfile& profile) override;
    virtual void remove(const QString& did) override;
    virtual void clear() override;

    const std::unordered_set<const BasicProfile*> findProfiles(const QString& text, int limit = 10) const;
    const std::unordered_set<const BasicProfile*>& findWordMatch(const QString& word) const;
    const std::unordered_set<const BasicProfile*> findWordPrefixMatch(const QString& prefix, int limit = 10) const;

private:
    std::set<QString> getWords(const BasicProfile& profile) const;
    void addToIndex(const BasicProfile& profile);
    void removeFromIndex(const BasicProfile* profile);
    void removeNonWordMatches(std::unordered_set<const BasicProfile*>& matches, const QString& word) const;
    void removeNonPrefixMatches(std::unordered_set<const BasicProfile*>& matches, const QString& prefix) const;

    std::map<QString, std::unordered_set<const BasicProfile*>> mWordIndex;
    std::unordered_map<const BasicProfile*, std::set<QString>> mProfileWords;
};

}
