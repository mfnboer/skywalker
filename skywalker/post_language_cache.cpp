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

    connect(&mLanguageUtils, &LanguageUtils::translation, this,
            [this](QString text, int requestId){
                handleTranslation(text, requestId);
            });

    connect(&mLanguageUtils, &LanguageUtils::translationError, this,
            [this](QString text, int requestId){
                handleTranslationError(text, requestId);
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

void PostLanguageCache::addTranslation(const QString& postUri, const QString& text, const QString& toLanguageCode)
{
    Q_ASSERT(!postUri.isEmpty());
    if (postUri.isEmpty())
        return;

    auto* info = getLanguageInfo(postUri);

    if (!info)
    {
        qWarning() << "No entry for:" << postUri;
        return;
    }

    info->mToLanguageCode = toLanguageCode;
    info->mTranslation = text;
    emit translationAdded(postUri);
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
        return;

    const int requestId = mLanguageUtils.identifyLanguage(post.getText());

    if (requestId < 0)
    {
        qDebug() << "Cannot identify language:" << uri;
        return;
    }

    mFetchingUris.insert(uri);
    mRequestPostUriMap[requestId] = uri;
}

void PostLanguageCache::translatePost(const Post& post, const QString& toLanguageCode)
{
    const QString& uri = post.getUri();
    qDebug() << "Translate post:" << uri << "to:" << toLanguageCode;

    if (mTranslatingUris.contains(uri))
        return;

    auto* info = getLanguageInfo(uri);

    if (!info)
    {
        qDebug() << "Identify post language first:" << uri;
        mUriTransatePostMap[uri] = {post, toLanguageCode};
        putPost(post);
        return;
    }

    if (info->mFromLanguageCode == toLanguageCode)
        return;

    const int requestId = mLanguageUtils.translate(post.getText(), info->mFromLanguageCode, toLanguageCode);

    if (requestId < 0)
    {
        qDebug() << "Cannot translate:" << uri << "from:" << info->mFromLanguageCode << "to:" << toLanguageCode;
        return;
    }

    info->mToLanguageCode = toLanguageCode;
    mTranslatingUris.insert(uri);
    mRequestPostUriMap[requestId] = uri;
}

PostLanguageCache::LanguageInfo* PostLanguageCache::getLanguageInfo(const QString& postUri) const
{
    return mCache[postUri];
}

bool PostLanguageCache::contains(const QString& postUri) const
{
    return mCache.contains(postUri);
}

QString PostLanguageCache::getDefaultPostLanguage() const
{
    return mLanguageUtils.getDefaultPostLanguage();
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

    auto itTranslate = mUriTransatePostMap.find(uri);

    if (itTranslate != mUriTransatePostMap.end())
    {
        const std::pair<Post, QString>& postLangCode = itTranslate->second;
        translatePost(postLangCode.first, postLangCode.second);
        mUriTransatePostMap.erase(itTranslate);
    }

    mRequestPostUriMap.erase(it);
}

void PostLanguageCache::handleTranslation(const QString& text, int requestId)
{
    auto it = mRequestPostUriMap.find(requestId);

    if (it == mRequestPostUriMap.end())
    {
        qDebug() << "Unknown request id:" << requestId;
        return;
    }

    const QString& uri = it->second;
    mTranslatingUris.erase(uri);
    auto* info = getLanguageInfo(uri);

    if (info)
    {
        info->mTranslation = text;
        emit translationAdded(uri);
    }
    else
    {
        qWarning() << "No entry for:" << uri << "request:" << requestId;
    }

    mRequestPostUriMap.erase(it);
}

void PostLanguageCache::handleTranslationError(const QString& error, int requestId)
{
    qDebug() << "Translation error:" << error << "request:" << requestId;
    auto it = mRequestPostUriMap.find(requestId);

    if (it == mRequestPostUriMap.end())
    {
        qDebug() << "Unknown request id:" << requestId;
        return;
    }

    const QString& uri = it->second;
    mTranslatingUris.erase(uri);
    auto* info = getLanguageInfo(uri);

    if (info)
        info->mToLanguageCode = {};

    mRequestPostUriMap.erase(it);
}

}
