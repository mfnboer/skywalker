// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "normalized_word_index.h"
#include <QColor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <set>

namespace Skywalker {

class UserSettings;

class FocusHashtagEntry : public QObject, public IMatchEntry
{
    Q_OBJECT
    Q_PROPERTY(int maxSize MEMBER MAX_HASHTAGS CONSTANT FINAL)
    Q_PROPERTY(int id READ getId CONSTANT FINAL)
    Q_PROPERTY(QStringList hashtags READ getHashtags NOTIFY hashtagsChanged FINAL)
    Q_PROPERTY(QColor highlightColor READ getHightlightColor WRITE setHighlightColor NOTIFY highlightColorChanged FINAL)

public:
    static constexpr int MAX_HASHTAGS = 20;
    static FocusHashtagEntry* fromJson(const QJsonObject& json, QObject* parent = nullptr);

    explicit FocusHashtagEntry(QObject* parent = nullptr);

    QJsonObject toJson() const;

    const std::set<QString>& getHashtagSet() const { return mHashtags; }
    QStringList getHashtags() const;
    bool addHashtag(const QString& hashtag);
    bool removeHashtag(const QString& hashtag);

    int getId() const { return mId; }
    const QColor& getHightlightColor() const { return mHighlightColor; }
    void setHighlightColor(const QColor& color);

    Q_INVOKABLE int size() const { return mHashtags.size(); }
    bool empty() const { return mHashtags.empty(); }

signals:
    void hashtagsChanged();
    void highlightColorChanged();

private:
    int mId;
    std::set<QString> mHashtags;
    QColor mHighlightColor = Qt::yellow;

    static int sNextId;
};

using FocusHashtagEntryList = QList<FocusHashtagEntry*>;

class FocusHashtags : public QObject, public IMatchWords
{
    Q_OBJECT
    Q_PROPERTY(int maxSize MEMBER MAX_ENTRIES CONSTANT FINAL)
    Q_PROPERTY(FocusHashtagEntryList entries READ getEntries NOTIFY entriesChanged FINAL)

public:
    static constexpr int MAX_ENTRIES = 100;

    explicit FocusHashtags(QObject* parent = nullptr);

    QJsonDocument toJson() const;
    void setEntries(const QJsonDocument& json);

    void clear();
    const FocusHashtagEntryList& getEntries() const { return mEntries; }
    Q_INVOKABLE void addEntry(FocusHashtagEntry* entry);
    Q_INVOKABLE void addEntry(const QString& hashtag, QColor highlightColor = {});
    Q_INVOKABLE void removeEntry(int entryId);
    Q_INVOKABLE void addHashtagToEntry(FocusHashtagEntry* entry, const QString hashtag);
    Q_INVOKABLE void removeHashtagFromEntry(FocusHashtagEntry* entry, const QString hashtag);

    std::pair<bool, const IMatchEntry*> match(const NormalizedWordIndex& post) const override;

    // Returns invalid color when no match is found
    QColor highlightColor(const NormalizedWordIndex& post) const;

    FocusHashtagEntryList getMatchEntries(const NormalizedWordIndex& post) const;
    std::set<QString> getNormalizedMatchHashtags(const NormalizedWordIndex& post) const;

    Q_INVOKABLE void save(const QString& did, UserSettings* settings) const;
    Q_INVOKABLE void load(const QString& did, const UserSettings* settings);

signals:
    void entriesChanged();

private:
    FocusHashtagEntryList mEntries;
    std::unordered_map<QString, std::unordered_set<FocusHashtagEntry*>> mAllHashtags;
};

}
