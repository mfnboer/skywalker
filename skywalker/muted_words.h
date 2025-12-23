// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "user_settings.h"
#include "normalized_word_index.h"
#include "unicode_fonts.h"
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <QString>
#include <unordered_map>
#include <vector>
#include <set>

namespace Skywalker {

class MutedWordEntry
{
    Q_GADGET
    Q_PROPERTY(QString typeName READ typeName CONSTANT FINAL)
    Q_PROPERTY(QString value READ getValue FINAL)
    Q_PROPERTY(QEnums::ActorTarget actorTarget READ getActorTarget FINAL)
    Q_PROPERTY(QDateTime expiresAt READ getExpiresAt FINAL)
    Q_PROPERTY(bool isDomain READ isDomain FINAL)
    QML_VALUE_TYPE(mutedwordentry)

public:
    using List = QList<MutedWordEntry>;

    static QString typeName() { return "MutedWordEntry"; }

    MutedWordEntry() = default;
    MutedWordEntry(const QString& value, QEnums::ActorTarget actorTarget, QDateTime expiresAt);
    explicit MutedWordEntry(const IMatchEntry* entry);

    const QString& getValue() const { return mValue; }
    QEnums::ActorTarget getActorTarget() const { return mActorTarget; }
    QDateTime getExpiresAt() const { return mExpiresAt; }
    bool isDomain() const { return mIsDomain; }
    bool isHashtag() const { return UnicodeFonts::isHashtag(mValue); }

    bool operator<(const MutedWordEntry& rhs) const { return mValue.localeAwareCompare(rhs.mValue) < 0; }

private:

    QString mValue;
    QEnums::ActorTarget mActorTarget;
    QDateTime mExpiresAt;
    bool mIsDomain = false;
};

class MutedWords : public QObject, public IMatchWords
{
    Q_OBJECT
    Q_PROPERTY(MutedWordEntry::List entries READ getEntries NOTIFY entriesChanged FINAL)
    Q_PROPERTY(int maxSize MEMBER MAX_ENTRIES CONSTANT FINAL)

public:
    static constexpr size_t MAX_ENTRIES = 100;

    explicit MutedWords(QObject* parent = nullptr);

    MutedWordEntry::List getEntries() const;
    void clear();

    Q_INVOKABLE void addEntry(const QString& word, QEnums::ActorTarget actorTarget = QEnums::ACTOR_TARGET_ALL, QDateTime expiresAt = {});
    void addEntry(const QString& word, const QJsonObject& bskyJson, const QStringList& unkwownTargets,
                  QEnums::ActorTarget actorTarget, QDateTime expiresAt);
    Q_INVOKABLE void removeEntry(const QString& word);
    Q_INVOKABLE bool containsEntry(const QString& word);
    void load(const ATProto::UserPreferences& userPrefs);
    bool legacyLoad(const UserSettings* userSettings);
    void save(ATProto::UserPreferences& userPrefs);
    bool isDirty() const { return mDirty; }

    std::pair<bool, const IMatchEntry*> match(const NormalizedWordIndex& post) const override;

signals:
    void entriesChanged();

private:
    struct Entry : public IMatchEntry
    {
        QString mRaw;
        std::vector<QString> mNormalizedWords;
        QEnums::ActorTarget mActorTarget = QEnums::ACTOR_TARGET_ALL;
        QDateTime mExpiresAt;

        // Bsky properties saved for forward compatibility, i.e. these will be saved unaltered.
        QJsonObject mBskyJson;
        QStringList mUnknownTargets;

        Entry(const QString& raw, const std::vector<QString>& normalizedWords,
              const QJsonObject& bskyJson, const QStringList& unknownTargets) :
            mRaw(raw),
            mNormalizedWords(normalizedWords),
            mBskyJson(bskyJson),
            mUnknownTargets(unknownTargets)
        {}

        bool operator<(const Entry& rhs) const { return mRaw.localeAwareCompare(rhs.mRaw) < 0; }

        size_t wordCount() const { return mNormalizedWords.size(); }
        bool isHashtag() const { return wordCount() == 1 && UnicodeFonts::isHashtag(mRaw); }
        bool isDomain() const;
    };

    using WordIndexType = std::unordered_map<QString, std::set<const Entry*>>;

    void addWordToIndex(const Entry* entry, WordIndexType& wordIndex);
    void removeWordFromIndex(const Entry* entry, WordIndexType& wordIndex);
    bool preAdd(const Entry& entry);
    bool mustSkip(const Entry& entry, const BasicProfile& author, QDateTime now) const;
    std::pair<bool, const IMatchEntry*> matchDomain(const NormalizedWordIndex& post, QDateTime now, const BasicProfile& author) const;
    std::pair<bool, const IMatchEntry*> matchHashtag(const NormalizedWordIndex& post, QDateTime now, const BasicProfile& author) const;
    std::pair<bool, const IMatchEntry*> matchWords(const NormalizedWordIndex& post, QDateTime now, const BasicProfile& author) const;

    std::set<Entry> mEntries;

    // Normalized word (from single word entries) -> index
    WordIndexType mSingleWordIndex;

    // Normalized first word (from multi-word entries) -> index
    WordIndexType mFirstWordIndex;

    WordIndexType mHashTagIndex;
    WordIndexType mDomainIndex;

    bool mDirty = false;

    friend class MutedWordEntry;
};

class MutedWordsNoMutes : public IMatchWords
{
public:
    std::pair<bool, const IMatchEntry*> match(const NormalizedWordIndex&) const override { return { false, nullptr }; }
};

}
