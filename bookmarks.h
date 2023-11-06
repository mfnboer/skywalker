// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
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
    static constexpr size_t MAX_BOOKMARKS = 3;

    explicit Bookmarks(QObject* parent = nullptr);

    size_t size() const { return mBookmarkedPostUris.size(); }
    bool isFull() const { return mBookmarkedPostUris.size() == MAX_BOOKMARKS; }

    Q_INVOKABLE bool addBookmark(const QString& postUri);
    Q_INVOKABLE void removeBookmark(const QString& postUri);
    Q_INVOKABLE bool isBookmarked(const QString& postUri) const { return mPostUriIndex.count(postUri); }

    void clear();
    const std::vector<QString>& getBookmarks() const { return mBookmarkedPostUris; }

    std::vector<QString> getPage(int startIndex, int size) const;

signals:
    void sizeChanged();

private:
    std::vector<QString> mBookmarkedPostUris;
    std::unordered_set<QString> mPostUriIndex;
};

}
