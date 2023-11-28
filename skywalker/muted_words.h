// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
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
public:
    static constexpr size_t MAX_ENTRIES = 100;

    MutedWords(QObject* parent = nullptr);

    QStringList getEntries() const;

    Q_INVOKABLE void addEntry(const QString& word);
    Q_INVOKABLE void removeEntry(const QString& word);

    void removeEntry(int index);
    bool match(const Post& post) const;

signals:
    void entriesChanged();

private:
    using WordIndexType = std::unordered_map<QString, std::set<int>>;

    void addWordToIndex(const QString& word, int index, WordIndexType& wordIndex);
    void removeWordFromIndex(const QString& word, int index, WordIndexType& wordIndex);
    void reindexAfterRemoval(int removedIndex, WordIndexType& wordIndex);

    struct Entry
    {
        QString mRaw;
        std::vector<QString> mNormalizedWords;
    };

    std::vector<Entry> mEntries;

    // Normalized word (from single word entries) -> index
    WordIndexType mSingleWordIndex;

    // Normalized first word (from multi-word entries) -> index
    WordIndexType mFirstWordIndex;
};

}
