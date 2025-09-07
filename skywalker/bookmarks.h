// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class Bookmarks : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Bookmarks(QObject* parent = nullptr);

    Q_INVOKABLE void getBookmarks(int limit = 50, const QString& cursor = {});
    Q_INVOKABLE void getBookmarksNextPage();
    Q_INVOKABLE void addBookmark(const QString& postUri, const QString& postCid);
    Q_INVOKABLE void removeBookmark(const QString& postUri, const QString& postCid);
};

}
