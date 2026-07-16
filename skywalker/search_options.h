// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/client.h>
#include <QDateTime>
#include <QObject>

namespace Skywalker {

class SearchOptions
{
    Q_GADGET
    Q_PROPERTY(QString sortOrder READ getSortOrder WRITE setSortOrder FINAL)
    Q_PROPERTY(bool exactPhrase READ getExactPhrase WRITE setExactPhrase FINAL)
    Q_PROPERTY(bool following READ getFollowing WRITE setFollowing FINAL)
    Q_PROPERTY(QStringList authors READ getAuthors WRITE setAuthors FINAL)
    Q_PROPERTY(QStringList mentions READ getMentions WRITE setMentions FINAL)
    Q_PROPERTY(QDateTime since READ getSince WRITE setSince FINAL)
    Q_PROPERTY(bool isSetSince READ getSetSince WRITE setSetSince FINAL)
    Q_PROPERTY(QDateTime until READ getUntil WRITE setUntil FINAL)
    Q_PROPERTY(bool isSetUntil READ getSetUntil WRITE setSetUntil FINAL)
    Q_PROPERTY(QString language READ getLanguage WRITE setLanguage FINAL)
    Q_PROPERTY(PostFilter postFilter READ getPostFilter WRITE setPostFilter FINAL)
    Q_PROPERTY(MediaPostFilter mediaPostFilter READ getMediaPostFilter WRITE setMediaPostFilter FINAL)
    Q_PROPERTY(QStringList excludeWords READ getExcludeWords WRITE setExcludeWords FINAL)
    QML_ELEMENT

public:
    enum PostFilter
    {
        POST_FILTER_ALL = 0,
        POST_FILTER_NO_REPLIES,
        POST_FILTER_ONLY_REPLIES,

        POST_FILTER_LAST = POST_FILTER_ONLY_REPLIES
    };
    Q_ENUM(PostFilter)

    enum MediaPostFilter
    {
        MEDIA_POST_FILTER_ALL = 0,
        MEDIA_POST_FILTER_MEDIA,
        MEDIA_POST_FILTER_VIDEO,

        MEDIA_POST_FILTER_LAST = MEDIA_POST_FILTER_VIDEO
    };
    Q_ENUM(MediaPostFilter)

    const QString& getSortOrder() const { return mSortOrder; }
    void setSortOrder(const QString& sortOrder) { mSortOrder = sortOrder; }

    bool getExactPhrase() const { return mExactPrase; }
    void setExactPhrase(bool exactPhrase) { mExactPrase = exactPhrase; }

    bool getFollowing() const { return mFollowing; }
    void setFollowing(bool following) { mFollowing = following; }

    const QStringList& getAuthors() const { return mAuthors; }
    void setAuthors(const QStringList& authors) { mAuthors = authors; }

    const QStringList& getMentions() const { return mMentions; }
    void setMentions(const QStringList& mentions) { mMentions = mentions; }

    const QDateTime& getSince() const { return mSince; }
    void setSince(const QDateTime& since) { mSince = since; }

    bool getSetSince() const { return mSetSince; }
    void setSetSince(bool setSince) { mSetSince = setSince; }

    const QDateTime& getUntil() const { return mUntil; }
    void setUntil(const QDateTime& until) { mUntil = until; }

    bool getSetUntil() const { return mSetUntil; }
    void setSetUntil(bool setUntil) { mSetUntil = setUntil; }

    const QString& getLanguage() const { return mLanguage; }
    void setLanguage(const QString& language) { mLanguage = language; }

    PostFilter getPostFilter() const { return mPostFilter; }
    void setPostFilter(PostFilter postFilter) { mPostFilter = postFilter; }

    MediaPostFilter getMediaPostFilter() const { return mMediaPostFilter; }
    void setMediaPostFilter(MediaPostFilter mediaPostFilter) { mMediaPostFilter = mediaPostFilter; }

    const QStringList& getExcludeWords() const { return mExcludeWords; }
    void setExcludeWords(const QStringList& excludeWords) { mExcludeWords = excludeWords; }

    ATProto::Client::SearchParams createSearchParams(const QString& userDid) const;
    Q_INVOKABLE QString getDescription() const;

    bool equals(const SearchOptions& other) const;
    bool isDefault() const;

    static QStringList validateHandles(const QStringList& handles);
    static QStringList cleanHandleList(const QStringList& authors, const QString& userDid);

private:
    QString mSortOrder = ATProto::AppBskyFeed::SearchSortOrder::RECENT;
    bool mExactPrase = false;
    bool mFollowing = false;
    QStringList mAuthors;
    QStringList mMentions;
    QDateTime mSince;
    bool mSetSince = false;
    QDateTime mUntil;
    bool mSetUntil = false;
    QString mLanguage;
    PostFilter mPostFilter = POST_FILTER_ALL;
    MediaPostFilter mMediaPostFilter = MEDIA_POST_FILTER_ALL;
    QStringList mExcludeWords;
};

}
