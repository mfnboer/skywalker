// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QCache>
#include <QObject>
#include <QString>
#include <map>
#include <set>
#include <unordered_set>

namespace Skywalker {

class HashtagIndex
{
public:
    explicit HashtagIndex(int maxEntries);

    void clear();
    void insert(const QString& hashtag);
    QStringList find(const QString& hashtag, int limit) const;

private:
    class Entry : public QObject
    {
    public:
        Entry() = default;
        Entry(HashtagIndex* index, const QString& hashtag);
        ~Entry();

        const QString& getNormalized() const { return mNormalized; }

    private:
        HashtagIndex* mHashtagIndex = nullptr;
        QString mHashtag;
        QString mNormalized;
    };

    void addToIndex(const QString& hashtag, const QString& normalized);
    void removeFromIndex(const QString& hashtag, const QString& normalized);
    void findFullMatch(const QString& normalized, int limit, QStringList& result,
                       std::unordered_set<QString>& alreadyFound) const;
    void findPrefixMatch(const QString& normalized, const QString& nonNormalized, int limit, QStringList& result,
                         std::unordered_set<QString>& alreadyFound) const;

    // normalized hashtag -> hashtags
    std::map<QString, std::set<QString>> mIndex;

    // hashtag -> Entry
    QCache<QString, Entry> mCache;
};

}
