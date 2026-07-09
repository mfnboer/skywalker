// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QCache>
#include <QDateTime>

namespace Skywalker {

using namespace std::chrono_literals;

template<typename Value>
struct ExpiryCacheEntry
{
    ~ExpiryCacheEntry() {
        if (mValue)
            delete mValue;
    }

    Value* mValue = nullptr;
    QDateTime mTimestamp;
};

template<typename Key, typename Value>
class ExpiryCache : public QCache<Key, ExpiryCacheEntry<Value>>
{
public:
    using Entry = ExpiryCacheEntry<Value>;
    using Parent = QCache<Key, Entry>;

    explicit ExpiryCache(std::chrono::minutes expiryInterval, qsizetype maxCost = 100) :
        Parent(maxCost),
        mExpiryInterval(expiryInterval)
    {}

    std::chrono::minutes getExpiryInterval() const
    {
        return mExpiryInterval;
    }

    bool contains(const Key& key) const
    {
        return object(key) != nullptr;
    }

    bool insert(const Key& key, Value* object, qsizetype cost = 1)
    {
        auto* entry = new Entry{object, QDateTime::currentDateTimeUtc()};
        return Parent::insert(key, entry, cost);
    }

    bool insertEntry(const Key& key, Entry* entry, qsizetype cost = 1)
    {
        return Parent::insert(key, entry, cost);
    }

    Value* object(const Key& key) const
    {
        Entry* entry = Parent::object(key);

        if (!entry)
            return nullptr;

        auto dt = QDateTime::currentDateTimeUtc() - entry->mTimestamp;

        if (dt >= mExpiryInterval)
        {
            qDebug() << "Expired:" << key << "age:" << (dt / 1min) << "minutes";
            return nullptr;
        }

        return entry->mValue;
    }

    Entry* getEntry(const Key& key) const
    {
        return Parent::object(key);
    }

    Value* take(const Key& key)
    {
        Entry* entry = Parent::take(key);

        if (!entry)
            return nullptr;

        Value* v = entry->mValue;
        entry->mValue = nullptr;
        delete entry;

        return v;
    }

    Value* operator[](const Key& key) const
    {
        return object(key);
    }

private:
    std::chrono::minutes mExpiryInterval;
};

}
