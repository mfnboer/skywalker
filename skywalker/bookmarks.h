// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class Bookmarks : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool migrationInProgress READ isMigrationInProgress NOTIFY migrationInProgressChanged FINAL)
    Q_PROPERTY(int toMigrateCount READ getToMigrateCount NOTIFY toMigrateCountChanged FINAL)
    Q_PROPERTY(int migratedCount READ getMigratedCount NOTIFY migratedCountChanged FINAL)
    QML_ELEMENT

public:
    explicit Bookmarks(QObject* parent = nullptr);

    Q_INVOKABLE void getBookmarks(int limit = 50, const QString& cursor = {});
    Q_INVOKABLE void getBookmarksNextPage();
    Q_INVOKABLE void addBookmark(const QString& postUri, const QString& postCid);
    Q_INVOKABLE void removeBookmark(const QString& postUri, const QString& postCid);

    bool migrateToBsky();

    bool isMigrationInProgress() const { return mMigrationInProgress; }
    int getToMigrateCount() const { return mToMigrateCount; }
    int getMigratedCount() const { return mMigratedCount; }

signals:
    void migrationInProgressChanged();
    void toMigrateCountChanged();
    void migratedCountChanged();

private:
    void createBookmarks(const QStringList& postUris, int startIndex);
    void createBookmarks(const ATProto::AppBskyFeed::PostView::List& postViewList, int index, const std::function<void()>& doneCb);
    void updateLocalBookmarks();

    void setMigrationInProgress(bool inProgress);
    void setToMigrateCount(int count);
    void setMigratedCount(int count);

    bool mMigrationInProgress = false;
    int mToMigrateCount = 0;
    int mMigratedCount = 0;
    QStringList mFailedToMigrateUris;
    std::unordered_set<QString> mSucceededToMigrateUris;
};

}
