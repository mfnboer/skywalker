// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "draft_posts.h"
#include <tuple>

namespace Skywalker {

// Migrate draft posts from PDS repo to local files.
// PDS repo records are public readable!
class DraftPostsMigration : public QObject
{
    Q_OBJECT

public:
    explicit DraftPostsMigration(Skywalker* skywalker, QObject* parent = nullptr);

    void migrateFromRepoToFile();

signals:
    void migrationOk();
    void migrationFailed();

private:
    void loadImagesFromRepo();
    void loadImagesFromRepo(const QStringList& images);
    void migrateToFile();
    void deleteRecords(const QStringList& recordUris, const std::function<void()>& okCb,
                       const std::function<void()>& failCb);
    std::tuple<QStringList, QStringList> getImages(const DraftPostData& data) const;

    Skywalker* mSKywalker;
    DraftPosts mRepoDrafts;
    DraftPosts mFileDrafts;
};

}
