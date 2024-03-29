// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"
#include "lexicon/bookmark.h"
#include <QDebug>
#include <QObject>
#include <QString>
#include <unordered_map>
#include <unordered_set>

namespace Skywalker {

class Bookmarks : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(int maxSize MEMBER MAX_BOOKMARKS CONSTANT FINAL)
    Q_PROPERTY(int size READ size NOTIFY sizeChanged FINAL)

public:
    static constexpr qsizetype MAX_BOOKMARKS = 200;

    explicit Bookmarks(QObject* parent = nullptr);

    size_t size() const { return mBookmarkedPostUris.size(); }
    bool isFull() const { return mBookmarkedPostUris.size() >= MAX_BOOKMARKS; }

    Q_INVOKABLE bool addBookmark(const QString& postUri);
    Q_INVOKABLE void removeBookmark(const QString& postUri);
    Q_INVOKABLE bool isBookmarked(const QString& postUri) const { return mPostUriIndex.count(postUri); }

    void clear();
    std::vector<QString> getPage(int startIndex, int size) const;

    void load();
    void save();

signals:
    void sizeChanged();
    void bookmarksLoaded();

private:
    void loadFromBsky(std::function<void()> doneCb);
    void loadFromSettings();
    bool addBookmarkPrivate(const QString& postUri);

    // Functions to store bookmarks in the PDS.
    // As PDS records are public, we store bookmarks in user settings for now.
    void writeRecord(const Bookmark::Bookmark& bookmark);
    void deleteRecord(const QString& postUri);
    void deleteRecords();
    void listRecords(const std::function<void()>& doneCb, std::optional<QString> cursor = {}, int maxPages = 10);
    void createRecords(const QStringList& postUris, const std::function<void()>& doneCb);

    QStringList mBookmarkedPostUris;
    std::unordered_set<QString> mPostUriIndex;
    std::unordered_map<QString, QString> mPostUriRecordUriMap;
    bool mDirty = false;
};

}
