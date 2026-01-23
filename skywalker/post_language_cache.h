// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "language_utils.h"
#include "wrapped_skywalker.h"
#include <QCache>

namespace Skywalker {

class Post;

// Cache of uri's from root posts with a thread indication.
class PostLanguageCache : public WrappedSkywalker
{
    Q_OBJECT

public:
    struct LanguageInfo
    {
        QString mFromLanguageCode;
        QString mToLanguageCode;
        QString mTranslation;
    };

    static PostLanguageCache& instance();

    void put(const QString& postUri, const QString& languageCode);
    void addTranslation(const QString& postUri, const QString& text, const QString& toLanguageCode);
    void putPost(const Post& post);
    void translatePost(const Post& post, const QString& toLanguageCode);
    LanguageInfo* getLanguageInfo(const QString& postUri) const;
    bool contains(const QString& postUri) const;
    QString getDefaultPostLanguage() const;

signals:
    void postAdded(const QString& uri);
    void translationAdded(const QString& uri);

private:
    explicit PostLanguageCache(QObject* parent = nullptr);
    void handleLanguageIdentified(const QString& languageCode, int requestId);
    void handleTranslation(const QString& text, int requestId);
    void handleTranslationError(const QString& error, int requestId);

    QCache<QString, LanguageInfo> mCache{100}; // post-uri -> language code
    std::unordered_set<QString> mFetchingUris;
    std::unordered_set<QString> mTranslatingUris;
    LanguageUtils mLanguageUtils;
    std::unordered_map<int, QString> mRequestPostUriMap; // request id -> post-uri
    std::unordered_map<QString, std::pair<Post, QString>> mUriTransatePostMap; // post-uri -> (post, language)

    static std::unique_ptr<PostLanguageCache> sInstance;
};

}
