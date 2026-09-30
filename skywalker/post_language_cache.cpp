// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "post_language_cache.h"
#include "post.h"

namespace Skywalker {

std::unique_ptr<PostLanguageCache> PostLanguageCache::sInstance;

PostLanguageCache::PostLanguageCache(QObject* parent) :
    WrappedSkywalker(parent),
    mLanguageUtils(this)
{
    connect(this, &WrappedSkywalker::skywalkerChanged, this, [this]{
        if (!mSkywalker)
            return;

        mLanguageUtils.setSkywalker(mSkywalker);
    });

    connect(&mLanguageUtils, &LanguageUtils::languageIdentified, this,
            [this](QString languageCode, int requestId){
                handleLanguageIdentified(languageCode, requestId);
            });
}

PostLanguageCache& PostLanguageCache::instance()
{
    if (!sInstance)
        sInstance = std::unique_ptr<PostLanguageCache>(new PostLanguageCache);

    return *sInstance;
}

void PostLanguageCache::put(const QString& postUri, const QString& languageCode)
{
    Q_ASSERT(!postUri.isEmpty());
    if (postUri.isEmpty())
        return;

    auto* info = new LanguageInfo;
    info->mFromLanguageCode = languageCode;
    mCache.insert(postUri, info);
    qDebug() << "Cache size:" << mCache.size();
    emit postAdded(postUri);
}

void PostLanguageCache::putPost(const Post& post)
{
    const QString& uri = post.getUri();
    qDebug() << "Put post:" << uri;

    if (contains(uri))
    {
        qDebug() << "Post already in cache:" << uri;
        return;
    }

    if (mFetchingUris.contains(uri))
    {
        qDebug() << "Identification already requested:" << uri;
        return;
    }

    auto* language = post.getFirstLanguage();
    QStringList excludeLanguages;

    if (language)
    {
        // Dutch and Afrikaans are often confused
        if (language->getShortCode() == "nl")
            excludeLanguages.push_back("af");
        else if (language->getShortCode() == "af")
            excludeLanguages.push_back("nl");
    }

    const int requestId = mLanguageUtils.identifyLanguage(post.getText(), excludeLanguages);

    if (requestId < 0)
    {
        qDebug() << "Cannot identify language:" << uri;
        return;
    }

    mFetchingUris.insert(uri);
    mRequestPostUriMap[requestId] = uri;
}

PostLanguageCache::LanguageInfo* PostLanguageCache::getLanguageInfo(const QString& postUri) const
{
    return contains(postUri) ? mCache[postUri] : nullptr;
}

bool PostLanguageCache::contains(const QString& postUri) const
{
    return mCache.contains(postUri);
}

void PostLanguageCache::handleLanguageIdentified(const QString& languageCode, int requestId)
{
    auto it = mRequestPostUriMap.find(requestId);

    if (it == mRequestPostUriMap.end())
    {
        qDebug() << "Unknown request id:" << requestId;
        return;
    }

    const QString& uri = it->second;
    mFetchingUris.erase(uri);

    if (!languageCode.isEmpty() && languageCode != Language::UNDEFINED_CODE)
    {
        if (LanguageUtils::existsShortCode(languageCode))
            put(uri, languageCode);
        else
            qWarning() << "Unknown language code:" << languageCode;
    }

    mRequestPostUriMap.erase(it);
}

}