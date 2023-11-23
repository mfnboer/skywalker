// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "user_settings.h"
#include <QDebug>
#include <QObject>
#include <QString>
#include <unordered_set>

namespace Skywalker {

class Bookmarks : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int maxSize MEMBER MAX_BOOKMARKS CONSTANT FINAL)
    Q_PROPERTY(int size READ size NOTIFY sizeChanged FINAL)

public:
    static constexpr size_t MAX_BOOKMARKS = 200;

    explicit Bookmarks(QObject* parent = nullptr);

    size_t size() const { return mBookmarkedPostUris.size(); }
    bool isFull() const { return mBookmarkedPostUris.size() >= MAX_BOOKMARKS; }

    Q_INVOKABLE bool addBookmark(const QString& postUri);
    Q_INVOKABLE void removeBookmark(const QString& postUri);
    Q_INVOKABLE bool isBookmarked(const QString& postUri) const { return mPostUriIndex.count(postUri); }

    void clear();
    const std::vector<QString>& getBookmarks() const { return mBookmarkedPostUris; }

    std::vector<QString> getPage(int startIndex, int size) const;

    Q_INVOKABLE void load(const UserSettings* userSettings);
    void save(UserSettings* userSettings);

    Q_INVOKABLE bool noticeSeen(const UserSettings* userSettings) const;
    Q_INVOKABLE void setNoticeSeen(UserSettings* userSettings, bool seen) const;

signals:
    void sizeChanged();

private:
    void clearPrivate();
    bool addBookmarkPrivate(const QString& postUri);

    std::vector<QString> mBookmarkedPostUris;
    std::unordered_set<QString> mPostUriIndex;
    bool mDirty = false;
};

}
