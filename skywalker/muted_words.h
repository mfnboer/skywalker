// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "user_settings.h"
#include "normalized_word_index.h"
#include <QObject>
#include <QString>
#include <unordered_map>
#include <vector>
#include <set>

namespace Skywalker {

class MutedWords : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList entries READ getEntries NOTIFY entriesChanged FINAL)
    Q_PROPERTY(int maxSize MEMBER MAX_ENTRIES CONSTANT FINAL)

public:
    static constexpr size_t MAX_ENTRIES = 100;

    MutedWords(QObject* parent = nullptr);

    QStringList getEntries() const;
    void clear();

    Q_INVOKABLE void addEntry(const QString& word);
    Q_INVOKABLE void removeEntry(const QString& word);
    Q_INVOKABLE void load(const UserSettings* userSettings);
    Q_INVOKABLE void save(UserSettings* userSettings);
    Q_INVOKABLE bool noticeSeen(const UserSettings* userSettings) const;
    Q_INVOKABLE void setNoticeSeen(UserSettings* userSettings, bool seen) const;

    bool match(const NormalizedWordIndex& post) const;

signals:
    void entriesChanged();

private:
    struct Entry
    {
        QString mRaw;
        std::vector<QString> mNormalizedWords;

        Entry(const QString& raw, const std::vector<QString>& normalizedWords) :
            mRaw(raw), mNormalizedWords(normalizedWords) {}

        bool operator<(const Entry& rhs) const { return mRaw.localeAwareCompare(rhs.mRaw) < 0; }
    };

    using WordIndexType = std::unordered_map<QString, std::set<const Entry*>>;

    void addWordToIndex(const Entry* entry, WordIndexType& wordIndex);
    void removeWordFromIndex(const Entry* entry, WordIndexType& wordIndex);

    std::set<Entry> mEntries;

    // Normalized word (from single word entries) -> index
    WordIndexType mSingleWordIndex;

    // Normalized first word (from multi-word entries) -> index
    WordIndexType mFirstWordIndex;

    bool mDirty = false;
};

}
