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
    };

    static PostLanguageCache& instance();

    void put(const QString& postUri, const QString& languageCode);
    void addTranslation(const QString& postUri, const QString& text, const QString& toLanguageCode);
    void putPost(const Post& post);
    LanguageInfo* getLanguageInfo(const QString& postUri) const;
    bool contains(const QString& postUri) const;

signals:
    void postAdded(const QString& uri);

private:
    explicit PostLanguageCache(QObject* parent = nullptr);
    void handleLanguageIdentified(const QString& languageCode, int requestId);

    QCache<QString, LanguageInfo> mCache{100}; // post-uri -> language code
    std::unordered_set<QString> mFetchingUris;
    LanguageUtils mLanguageUtils;
    std::unordered_map<int, QString> mRequestPostUriMap; // request id -> post-uri

    static std::unique_ptr<PostLanguageCache> sInstance;
};

}